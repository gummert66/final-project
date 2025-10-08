#ifndef PAYMENT_H
#define PAYMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX 100

typedef struct {
    char paymentID[10];
    char payerName[50];
    char serviceType[30];
    float amount;
    char paymentDate[15];
} Payment;

extern Payment payments[];
extern int count;
extern const char *serviceTypes[];
extern int serviceTypeCount;

void loadCSV(const char *filename);
void saveCSV(const char *filename);
void addPayment(void);
void searchPayment(void);
void updatePayment(void);
void deletePayment(void);
void displayMenu(void);
int daysInMonth(int year, int month);
void toLower(char *s);
int containsIgnoreCase(const char *text, const char *pattern);
int findServiceMatches(const char *input, int matchedIndexes[]);
int generateNextPaymentID(char outID[10]);
int comparePayment(const void *a, const void *b);

// Test helpers exposed to menu (UI)
int runUnitTests(void);
int runE2ETest(void);

#ifdef __cplusplus
}
#endif

#endif /* PAYMENT_H */
