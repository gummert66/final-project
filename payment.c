#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "payment.h"

// Internal helpers (no header exposure)
static int isValidPaymentID(const char *id) {
    if (!id || strlen(id) != 4) return 0;
    if (id[0] != 'P' && id[0] != 'p') return 0;
    for (int i = 1; i < 4; i++) if (!isdigit((unsigned char)id[i])) return 0;
    return 1;
}

static int parsePaymentNumber(const char *id, int *out) {
    if (!isValidPaymentID(id)) return 0;
    int n = atoi(id + 1);
    if (n <= 0) return 0;
    if (out) *out = n;
    return 1;
}

static void chomp(char *s) {
    if (!s) return;
    s[strcspn(s, "\r\n")] = '\0';
}

static int read_line(char *buf, size_t sz) {
    if (!fgets(buf, (int)sz, stdin)) return 0;
    chomp(buf);
    return 1;
}

static int read_int_range(const char *prompt, int minv, int maxv, int *out) {
    char line[64];
    while (1) {
        if (prompt && *prompt) printf("%s", prompt);
        if (!read_line(line, sizeof(line))) return 0;
        char *end = NULL;
        long v = strtol(line, &end, 10);
        if (end && *end == '\0' && v >= minv && v <= maxv) { if (out) *out = (int)v; return 1; }
        printf("Invalid input! Please enter a number between %d and %d.\n", minv, maxv);
    }
}

static int read_float_range(const char *prompt, float minv, float maxv, float *out) {
    char line[64];
    while (1) {
        if (prompt && *prompt) printf("%s", prompt);
        if (!read_line(line, sizeof(line))) return 0;
        char *end = NULL;
        float v = strtof(line, &end);
        if (end && *end == '\0' && v >= minv && v <= maxv) { if (out) *out = v; return 1; }
        printf("Invalid amount! Please enter between %.0f and %.0f.\n", (double)minv, (double)maxv);
    }
}

static int read_date_ymd(const char *prompt, int *y, int *m, int *d) {
    char line[64];
    while (1) {
        if (prompt && *prompt) printf("%s", prompt);
        if (!read_line(line, sizeof(line))) return 0;
        int yy, mm, dd;
        if (sscanf(line, "%d-%d-%d", &yy, &mm, &dd) == 3) {
            if (yy >= 2020 && mm >= 1 && mm <= 12 && dd >= 1 && dd <= daysInMonth(yy, mm)) {
                if (y) *y = yy; if (m) *m = mm; if (d) *d = dd; return 1;
            }
        }
        printf("Invalid date! Please re-enter as YYYY-MM-DD.\n");
    }
}

static void csv_safe_copy(const char *in, char *out, size_t outsz) {
    if (!in || !out || outsz == 0) return;
    size_t len = strlen(in);
    if (len > 0 && (in[0] == '=' || in[0] == '+' || in[0] == '-' || in[0] == '@')) {
        if (outsz > 1) {
            out[0] = '\'';
            strncpy(out + 1, in, outsz - 2);
            out[outsz - 1] = '\0';
        }
    } else {
        strncpy(out, in, outsz - 1);
        out[outsz - 1] = '\0';
    }
}

#ifdef _WIN32
#ifndef strcasecmp
#define strcasecmp _stricmp
#endif
#endif

Payment payments[MAX];
int count = 0;

const char *serviceTypes[] = {
    "Internet",
    "Cable TV",
    "Mobile banking",
    "Website",
    "ATM",
    "QR Code"
};
int serviceTypeCount = 6;

int comparePayment(const void *a, const void *b) {
    const Payment *pa = (const Payment *)a;
    const Payment *pb = (const Payment *)b;
    int nA = 0, nB = 0;
    int va = parsePaymentNumber(pa->paymentID, &nA);
    int vb = parsePaymentNumber(pb->paymentID, &nB);
    if (va && vb) return nA - nB;
    if (va && !vb) return -1;
    if (!va && vb) return 1;
    return strcasecmp(pa->paymentID, pb->paymentID);
}

#ifndef UNIT_TEST
int main() {
    int choice;
    loadCSV("paymentinfo.csv");
    do {
        displayMenu();
        read_int_range("Enter your choice: ", 0, 6, &choice);

        switch (choice) {
            case 1: addPayment(); break;
            case 2: searchPayment(); break;
            case 3: updatePayment(); break;
            case 4: deletePayment(); break;
            case 5: {
                int rc = runUnitTests();
                printf("Unit tests %s (rc=%d)\n", rc==0?"PASSED":"FAILED", rc);
                break;
            }
            case 6: {
                int rc = runE2ETest();
                printf("E2E tests %s (rc=%d)\n", rc==0?"PASSED":"FAILED", rc);
                break;
            }
            case 0: printf("Exiting program...\n"); break;
            default: printf("Invalid menu!\n");
        }
    } while (choice != 0);
    return 0;
}
#endif /* UNIT_TEST */

void displayMenu() {
    printf("\n===== Payment Management System =====\n");
    printf("1. Add Payment\n");
    printf("2. Search Payment\n");
    printf("3. Update Payment\n");
    printf("4. Delete Payment\n");
    printf("5. Run Unit Tests\n");
    printf("6. Run E2E Tests\n");
    printf("0. Exit\n");
    printf("=====================================\n");
}

void loadCSV(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("File %s not found. It will be created when you save.\n", filename);
        return;
    }
    char line[200];
    while (fgets(line, sizeof(line), fp)) {
        if (count >= MAX) { printf("Warning: maximum records reached (%d). Extra rows ignored.\n", MAX); break; }
        Payment tmp;
        if (sscanf(line, "%9[^,],%49[^,],%29[^,],%f,%14s",
                   tmp.paymentID,
                   tmp.payerName,
                   tmp.serviceType,
                   &tmp.amount,
                   tmp.paymentDate) == 5) {
            if (!isValidPaymentID(tmp.paymentID)) {
                printf("Skipping invalid ID record: %s", line);
                continue;
            }
            payments[count++] = tmp;
        }
    }
    fclose(fp);
}

void saveCSV(const char *filename) {
    qsort(payments, count, sizeof(Payment), comparePayment);

    char tmpname[260];
    snprintf(tmpname, sizeof(tmpname), "%s.tmp", filename);
    FILE *fp = fopen(tmpname, "w");
    if (!fp) {
        printf("Cannot write temp file %s\n", tmpname);
        return;
    }
    for (int i = 0; i < count; i++) {
        char safeName[60];
        char safeService[40];
        csv_safe_copy(payments[i].payerName, safeName, sizeof(safeName));
        csv_safe_copy(payments[i].serviceType, safeService, sizeof(safeService));
        fprintf(fp, "%s,%s,%s,%.2f,%s\n",
                payments[i].paymentID,
                safeName,
                safeService,
                payments[i].amount,
                payments[i].paymentDate);
    }
    fflush(fp);
    fclose(fp);
    remove(filename);
    if (rename(tmpname, filename) != 0) {
        printf("Failed to atomically replace %s with %s\n", filename, tmpname);
    }
}

int daysInMonth(int year, int month) {
    int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (month == 2) {
        if ((year % 400 == 0) || (year % 4 == 0 && year % 100 != 0)) return 29;
    }
    return days[month - 1];
}

void toLower(char *s) {
    for (int i = 0; s[i]; i++) s[i] = tolower((unsigned char)s[i]);
}

int containsIgnoreCase(const char *text, const char *pattern) {
    char lowerText[100], lowerPattern[50];
    strncpy(lowerText, text, sizeof(lowerText));
    strncpy(lowerPattern, pattern, sizeof(lowerPattern));
    lowerText[sizeof(lowerText)-1] = '\0';
    lowerPattern[sizeof(lowerPattern)-1] = '\0';
    toLower(lowerText);
    toLower(lowerPattern);
    return strstr(lowerText, lowerPattern) != NULL;
}

int findServiceMatches(const char *input, int matchedIndexes[]) {
    int matchCount = 0;
    for (int i = 0; i < serviceTypeCount; i++) {
        if (containsIgnoreCase(serviceTypes[i], input)) {
            matchedIndexes[matchCount++] = i;
        }
    }
    return matchCount;
}

int generateNextPaymentID(char outID[10]) {
    int used[MAX + 1] = {0};
    for (int i = 0; i < count; i++) {
        int n = 0;
        if (parsePaymentNumber(payments[i].paymentID, &n)) {
            if (n >= 1 && n <= MAX) used[n] = 1;
        }
    }
    for (int k = 1; k <= MAX; k++) {
        if (!used[k]) {
            sprintf(outID, "P%03d", k);
            return 1;
        }
    }
    return 0;
}

void addPayment() {
    if (count >= MAX) {
        printf("Cannot add more records (full)\n");
        return;
    }

    char newID[10];
    if (!generateNextPaymentID(newID)) {
        printf("Cannot add more records: all IDs used.\n");
        return;
    }
    strcpy(payments[count].paymentID, newID);
    printf("Assigned Payment ID: %s\n", payments[count].paymentID);

    do {
        printf("Enter Payer Name (First [Middle] Last): ");
        fgets(payments[count].payerName, 50, stdin);
        payments[count].payerName[strcspn(payments[count].payerName, "\n")] = '\0';
        if (strlen(payments[count].payerName) == 0)
            printf("Payer name cannot be empty!\n");
    } while (strlen(payments[count].payerName) == 0);

    int matched[10], matchCount;
    char serviceInput[30];
    do {
        printf("Enter Service Type (Internet, Cable TV, Mobile banking, Website, ATM, QR Code): ");
        fgets(serviceInput, sizeof(serviceInput), stdin);
        serviceInput[strcspn(serviceInput, "\n")] = '\0';
        matchCount = findServiceMatches(serviceInput, matched);

        if (matchCount == 0) {
            printf("No matching service type found. Please try again.\n");
        } else if (matchCount == 1) {
            strcpy(payments[count].serviceType, serviceTypes[matched[0]]);
            printf("Selected: %s\n", payments[count].serviceType);
            break;
        } else {
            printf("Multiple matches found:\n");
            for (int i = 0; i < matchCount; i++)
                printf("%d) %s\n", i + 1, serviceTypes[matched[i]]);
            int sel;
            if (!read_int_range("Select number: ", 1, matchCount, &sel)) { printf("Input error.\n"); return; }
            if (sel > 0 && sel <= matchCount) {
                strcpy(payments[count].serviceType, serviceTypes[matched[sel - 1]]);
                printf("Selected: %s\n", payments[count].serviceType);
                break;
            } else {
                printf("Invalid selection.\n");
            }
        }
    } while (1);

    read_float_range("Enter Amount (1 - 10000): ", 1.0f, 10000.0f, &payments[count].amount);

    int year, month, day;
    read_date_ymd("Enter Payment Date (YYYY-MM-DD): ", &year, &month, &day);
    sprintf(payments[count].paymentDate, "%04d-%02d-%02d", year, month, day);

    count++;
    saveCSV("paymentinfo.csv");
    printf("Payment added!\n");
}

void searchPayment() {
    int choice;
    read_int_range("Search by:\n1. Payment ID\n2. Payer Name\nChoose: ", 1, 2, &choice);

    if (choice == 1) {
        char id[10];
        printf("Enter Payment ID: ");
        if (!read_line(id, sizeof(id))) { printf("Input error.\n"); return; }
        for (int i=0; id[i]; i++) id[i] = toupper((unsigned char)id[i]);

        for (int i=0; i<count; i++) {
            char tmp[10];
            strcpy(tmp, payments[i].paymentID);
            for (int j=0; tmp[j]; j++) tmp[j] = toupper((unsigned char)tmp[j]);
            if (strcmp(tmp, id)==0) {
                printf("\nFound:\n%s | %s | %s | %.2f | %s\n",
                       payments[i].paymentID, payments[i].payerName,
                       payments[i].serviceType, payments[i].amount,
                       payments[i].paymentDate);
                return;
            }
        }
        printf("Payment not found!\n");
    }
    else if (choice == 2) {
        char name[50];
        printf("Enter Payer Name (keyword): ");
        fgets(name, 50, stdin);
        name[strcspn(name, "\n")] = '\0';

        int foundIndexes[MAX], foundCount=0;
        for (int i=0; i<count; i++) {
            if (containsIgnoreCase(payments[i].payerName, name)) {
                printf("%d) %s | %s\n", foundCount+1, payments[i].paymentID, payments[i].payerName);
                foundIndexes[foundCount++] = i;
            }
        }

        if (!foundCount) { printf("No records found.\n"); return; }

        int sel;
        read_int_range("\nEnter number to view detail (0 cancel): ", 0, foundCount, &sel);
        if (sel>0 && sel<=foundCount) {
            int idx=foundIndexes[sel-1];
            printf("\nSelected:\n%s | %s | %s | %.2f | %s\n",
                   payments[idx].paymentID, payments[idx].payerName,
                   payments[idx].serviceType, payments[idx].amount,
                   payments[idx].paymentDate);
        }
    } else printf("Invalid choice!\n");
}

void updatePayment() {
    char id[10];
    printf("Enter Payment ID to update: ");
    if (!read_line(id, sizeof(id))) { printf("Input error.\n"); return; }
    for (int k=0; id[k]; k++) id[k] = toupper((unsigned char)id[k]);

    for (int i=0; i<count; i++) {
        char tmp[10]; strcpy(tmp, payments[i].paymentID);
        for (int j=0; tmp[j]; j++) tmp[j] = toupper((unsigned char)tmp[j]);

        if (strcmp(tmp,id)==0){
            printf("\nCurrent Data:\n%s | %s | %s | %.2f | %s\n",
                payments[i].paymentID, payments[i].payerName,
                payments[i].serviceType, payments[i].amount,
                payments[i].paymentDate);

            int opt;
            do{
                read_int_range("\n--- Update Menu ---\n1.Name\n2.Service\n3.Amount\n4.Date\n0.Finish\nChoose: ", 0, 4, &opt);

                if(opt==1){
                    printf("Current: %s\nNew Name: ", payments[i].payerName);
                    fgets(payments[i].payerName,50,stdin);
                    payments[i].payerName[strcspn(payments[i].payerName,"\n")]='\0';
                }
                else if(opt==2){
                    char input[30]; int matched[10],mc;
                    do{
                        printf("Current: %s\nNew Service keyword: ",payments[i].serviceType);
                        fgets(input,sizeof(input),stdin);
                        input[strcspn(input,"\n")]='\0';
                        mc=findServiceMatches(input,matched);
                        if(mc==0) printf("No match!\n");
                        else if(mc==1){ strcpy(payments[i].serviceType,serviceTypes[matched[0]]); break;}
                        else { for(int x=0;x<mc;x++) printf("%d)%s\n",x+1,serviceTypes[matched[x]]);
                               int sel; read_int_range("Select: ", 1, mc, &sel);
                               if(sel>0&&sel<=mc){strcpy(payments[i].serviceType,serviceTypes[matched[sel-1]]);break;}
                        }
                    }while(1);
                }
                else if(opt==3){
                    char prompt[80];
                    snprintf(prompt, sizeof(prompt), "Current: %.2f\nNew Amount (1-10000): ", payments[i].amount);
                    read_float_range(prompt, 1.0f, 10000.0f, &payments[i].amount);
                }
                else if(opt==4){
                    int y,m,d;
                    read_date_ymd("Current date override (YYYY-MM-DD): ", &y, &m, &d);
                    sprintf(payments[i].paymentDate,"%04d-%02d-%02d",y,m,d);
                }
            }while(opt!=0);

            saveCSV("paymentinfo.csv");
            printf("Changes saved!\n");
            return;
        }
    }
    printf("Payment not found!\n");
}

void deletePayment() {
    char id[10]; printf("Enter Payment ID to delete: ");
    if (!read_line(id, sizeof(id))) { printf("Input error.\n"); return; }
    for(int i=0;i<count;i++){
        if(strcasecmp(payments[i].paymentID,id)==0){
            for(int j=i;j<count-1;j++) payments[j]=payments[j+1];
            count--; saveCSV("paymentinfo.csv");
            printf("Deleted.\n"); return;
        }
    }
    printf("Not found.\n");
}

int runUnitTests(void) {
#ifdef _WIN32
    int rc = system("gcc -DUNIT_TEST -o test_payment_unit.exe test_payment_unit.c payment.c");
    if (rc != 0) {
        printf("Failed to build unit tests (ensure gcc is installed).\n");
        return rc ? rc : 1;
    }
    rc = system(".\\test_payment_unit.exe");
    return rc;
#else
    printf("Unit test runner not supported on this platform in-app. Use Makefile/CI.\n");
    return 1;
#endif
}

int runE2ETest(void) {
#ifdef _WIN32
    int rc = system("powershell -ExecutionPolicy Bypass -File .\\test_payment_e2e.ps1");
    return rc;
#else
    printf("E2E runner not supported on this platform in-app.\n");
    return 1;
#endif
}

