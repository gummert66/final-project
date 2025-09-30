#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void loadCSV(const char *filename);
void saveCSV(const char *filename);
void addPayment();
void searchPayment();
void updatePayment();
void deletePayment();
void displayPayments();
void displayMenu();

int main() {
    int choice;
    loadCSV("paymentinfo.csv");  

    do {
        displayMenu();
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar();

        switch(choice) {
            case 1: addPayment(); break;
            case 2: searchPayment(); break;
            case 3: updatePayment(); break;
            case 4: deletePayment(); break;
            case 5: displayPayments(); break;
            case 0: printf("Exiting program...\n"); break;
            default: printf("Invalid menu!\n");
        }
    } while(choice != 0);

    return 0;
}

void displayMenu() {
    printf("\n===== Payment Management System =====\n");
    printf("1. Add Payment\n");
    printf("2. Search Payment\n");
    printf("3. Update Payment\n");
    printf("4. Delete Payment\n");
    printf("5. Display All Payments\n");
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

void addPayment() {
    if (count >= MAX) {
        printf("Cannot add more records (full)\n");
        return;
    }
    printf("Enter Payment ID: ");
    scanf("%9s", payments[count].paymentID);
    getchar();

    printf("Enter Payer Name: ");
    fgets(payments[count].payerName, 50, stdin);
    payments[count].payerName[strcspn(payments[count].payerName, "\n")] = '\0';

    printf("Enter Service Type: ");
    scanf("%29s", payments[count].serviceType);

    printf("Enter Amount: ");
    scanf("%f", &payments[count].amount);

    printf("Enter Payment Date (YYYY-MM-DD): ");
    scanf("%14s", payments[count].paymentDate);

    count++;
    saveCSV("paymentinfo.csv");  
    printf("Payment added!\n");
}

void searchPayment() {
    char id[10];
    printf("Enter Payment ID to search: ");
    scanf("%9s", id);

    for (int i = 0; i < count; i++) {
        if (strcmp(payments[i].paymentID, id) == 0) {
            printf("Found: %s | %s | %s | %.2f | %s\n",
                   payments[i].paymentID,
                   payments[i].payerName,
                   payments[i].serviceType,
                   payments[i].amount,
                   payments[i].paymentDate);
            return;
        }
    }
    printf("Payment not found!\n");
}

void updatePayment() {
    char id[10];
    printf("Enter Payment ID to update: ");
    scanf("%9s", id);

    for (int i = 0; i < count; i++) {
        if (strcmp(payments[i].paymentID, id) == 0) {
            printf("Current Data: %s | %s | %s | %.2f | %s\n",
                   payments[i].paymentID,
                   payments[i].payerName,
                   payments[i].serviceType,
                   payments[i].amount,
                   payments[i].paymentDate);

            printf("\n--- Enter New Data ---\n");
            getchar();
            printf("New Payer Name: ");
            fgets(payments[i].payerName, 50, stdin);
            payments[i].payerName[strcspn(payments[i].payerName, "\n")] = '\0';

            printf("New Service Type: ");
            scanf("%29s", payments[i].serviceType);

            printf("New Amount: ");
            scanf("%f", &payments[i].amount);

            printf("New Payment Date (YYYY-MM-DD): ");
            scanf("%14s", payments[i].paymentDate);

            saveCSV("paymentinfo.csv");
            printf("Payment updated!\n");
            return;
        }
    }
    printf("Payment not found!\n");
}

void deletePayment() {
    char id[10];
    printf("Enter Payment ID to delete: ");
    scanf("%9s", id);

    for (int i = 0; i < count; i++) {
        if (strcmp(payments[i].paymentID, id) == 0) {
            for (int j = i; j < count - 1; j++) {
                payments[j] = payments[j + 1];
            }
            count--;
            saveCSV("paymentinfo.csv");  
            printf("Payment deleted!\n");
            return;
        }
    }
    printf("Payment not found!\n");
}

void displayPayments() {
    printf("\n--- All Payments (%d records) ---\n", count);
    for (int i = 0; i < count; i++) {
        printf("%s | %s | %s | %.2f | %s\n",
               payments[i].paymentID,
               payments[i].payerName,
               payments[i].serviceType,
               payments[i].amount,
               payments[i].paymentDate);
    }
}
