#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX 100

typedef struct{
    char PaymentID[10];
    char PayerName[30];
    char ServiceType[30];
    float Amount;
    char Paymentdate[20];
} Payment;

Payment payment[MAX];
int count = 0;

//Function Prototype
void loadCSV();
void saveCSV();
void addPayment();
void searchPayment();
void updatePayment();
void deletePayment();
void displayPayment();
void displayMenu();

int main() {
    int choice;
    loadCSV();
}
