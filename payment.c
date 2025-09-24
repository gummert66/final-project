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

    do {
        displayMenu();
        printf("เลือกเมนู : ");
        scanf("%d", &choice);
        getchar();

        switch(choice){
            case 1 : addPayment(); break;
            case 2 : searchPayment(); break;
            case 3 : updatePayment(); break;
            case 4 : deletePayment(); break;
            case 5 : displayPayment(); break;
            case 0 : printf("ออกจากโปรแกรม ... \n");
            default : printf("เมนูไม่ถูกต้อง..! \n");
        }
    } while(choice != 0);

    return 0;
}

//Func def

void displayMenu() {
    printf("\n===== [ระบบการชำระเงิน] =====\n");
    printf("1. เพิ่มข้อมูลการชำระ \n");
    printf("2. ค้นหาข้อมูลการชำระ \n");
    printf("3. แก้ไข้ข้อมูลการชำระ \n");
    printf("4. ลบข้อมูลารชำระ \n");
    printf("5. แสดงข้อมูลทั้งหมด \n");
    printf("0. ออก \n");
    printf("=================================");

}
