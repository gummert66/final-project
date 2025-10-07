#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX 100

typedef struct {
    char paymentID[10];
    char payerName[50];
    char serviceType[30];
    float amount;
    char paymentDate[15];
} Payment;

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

void loadCSV(const char *filename);
void saveCSV(const char *filename);
void addPayment();
void searchPayment();
void updatePayment();
void deletePayment();
void displayMenu();
int daysInMonth(int year, int month);
void toLower(char *s);
int containsIgnoreCase(const char *text, const char *pattern);
int findServiceMatches(const char *input, int matchedIndexes[]);
int generateNextPaymentID(char outID[10]);

int comparePayment(const void *a, const void *b) {
    const Payment *pa = (const Payment *)a;
    const Payment *pb = (const Payment *)b;
    int idA = atoi(pa->paymentID + 1);  
    int idB = atoi(pb->paymentID + 1);
    return idA - idB; 
}

int main() {
    int choice;
    loadCSV("paymentinfo.csv");
    do {
        displayMenu();
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1: addPayment(); break;
            case 2: searchPayment(); break;
            case 3: updatePayment(); break;
            case 4: deletePayment(); break;
            case 0: printf("Exiting program...\n"); break;
            default: printf("Invalid menu!\n");
        }
    } while (choice != 0);
    return 0;
}

void displayMenu() {
    printf("\n===== Payment Management System =====\n");
    printf("1. Add Payment\n");
    printf("2. Search Payment\n");
    printf("3. Update Payment\n");
    printf("4. Delete Payment\n");
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
        if (sscanf(line, "%9[^,],%49[^,],%29[^,],%f,%14s",
                   payments[count].paymentID,
                   payments[count].payerName,
                   payments[count].serviceType,
                   &payments[count].amount,
                   payments[count].paymentDate) == 5) {
            count++;
        }
    }
    fclose(fp);
}

void saveCSV(const char *filename) {
    qsort(payments, count, sizeof(Payment), comparePayment);

    FILE *fp = fopen(filename, "w");
    if (!fp) {
        printf("Cannot write file %s\n", filename);
        return;
    }
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s,%s,%s,%.2f,%s\n",
                payments[i].paymentID,
                payments[i].payerName,
                payments[i].serviceType,
                payments[i].amount,
                payments[i].paymentDate);
    }
    fclose(fp);
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
        const char *id = payments[i].paymentID;
        if (id[0] == 'P' || id[0] == 'p') {
            int n = atoi(id + 1);
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
            printf("Select number: ");
            scanf("%d", &sel);
            getchar();
            if (sel > 0 && sel <= matchCount) {
                strcpy(payments[count].serviceType, serviceTypes[matched[sel - 1]]);
                printf("Selected: %s\n", payments[count].serviceType);
                break;
            } else {
                printf("Invalid selection.\n");
            }
        }
    } while (1);

    do {
        printf("Enter Amount (1 - 10000): ");
        scanf("%f", &payments[count].amount);
        getchar();
        if (payments[count].amount < 1 || payments[count].amount > 10000)
            printf("Invalid amount! Please enter between 1 and 10000.\n");
    } while (payments[count].amount < 1 || payments[count].amount > 10000);

    int year, month, day;
    while (1) {
        printf("Enter Payment Date (YYYY-MM-DD): ");
        int read = scanf("%d-%d-%d", &year, &month, &day);
        getchar();
        if (read != 3 || year < 2020 || month < 1 || month > 12 || day < 1 || day > daysInMonth(year, month)) {
            printf("Invalid date! Please re-enter.\n");
        } else break;
    }
    sprintf(payments[count].paymentDate, "%04d-%02d-%02d", year, month, day);

    count++;
    saveCSV("paymentinfo.csv");
    printf("Payment added!\n");
}

void searchPayment() {
    int choice;
    printf("Search by:\n1. Payment ID\n2. Payer Name\nChoose: ");
    scanf("%d", &choice);
    getchar();

    if (choice == 1) {
        char id[10];
        printf("Enter Payment ID: ");
        scanf("%9s", id);
        getchar();
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
        printf("\nEnter number to view detail (0 cancel): ");
        scanf("%d", &sel); getchar();
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
    scanf("%9s", id); getchar();
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
                printf("\n--- Update Menu ---\n1.Name\n2.Service\n3.Amount\n4.Date\n0.Finish\nChoose: ");
                scanf("%d",&opt); getchar();

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
                               int sel; printf("Select: "); scanf("%d",&sel); getchar();
                               if(sel>0&&sel<=mc){strcpy(payments[i].serviceType,serviceTypes[matched[sel-1]]);break;}
                        }
                    }while(1);
                }
                else if(opt==3){
                    do{
                        printf("Current: %.2f\nNew Amount (1-10000): ",payments[i].amount);
                        scanf("%f",&payments[i].amount); getchar();
                    }while(payments[i].amount<1||payments[i].amount>10000);
                }
                else if(opt==4){
                    int y,m,d;
                    while(1){
                        printf("Current: %s\nNew Date YYYY-MM-DD: ",payments[i].paymentDate);
                        int read = scanf("%d-%d-%d",&y,&m,&d); getchar();
                        if(read!=3 || y<2020||m<1||m>12||d<1||d>daysInMonth(y,m)){
                            printf("Invalid date! Please re-enter.\n");
                        }else break;
                    }
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
    scanf("%9s",id);
    for(int i=0;i<count;i++){
        if(strcasecmp(payments[i].paymentID,id)==0){
            for(int j=i;j<count-1;j++) payments[j]=payments[j+1];
            count--; saveCSV("paymentinfo.csv");
            printf("Deleted.\n"); return;
        }
    }
    printf("Not found.\n");
}
