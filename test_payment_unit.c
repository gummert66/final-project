#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "payment.h"

static int g_total = 0;
static int g_passed = 0;
static int g_failed = 0;

static void start_test(const char *name) {
    printf("Running: %s\n", name);
}

static void expect_true(int cond, const char *what) {
    g_total++;
    if (cond) {
        g_passed++;
        printf("[PASS] %s\n", what);
    } else {
        g_failed++;
        printf("[FAIL] %s\n", what);
    }
}

static void reset_state(void) {
    count = 0;
    memset(payments, 0, sizeof(Payment) * MAX);
}

static int file_exists(const char *path) {
    FILE *f = fopen(path, "r");
    if (f) { fclose(f); return 1; }
    return 0;
}

static int backup_csv(void) {
    if (!file_exists("paymentinfo.csv")) return 0;
    remove("paymentinfo_backup.csv");
    return rename("paymentinfo.csv", "paymentinfo_backup.csv") == 0;
}

static void restore_csv(void) {
    if (file_exists("paymentinfo_backup.csv")) {
        remove("paymentinfo.csv");
        rename("paymentinfo_backup.csv", "paymentinfo.csv");
    }
}

static void write_input_file(const char *path, const char *content) {
    FILE *f = fopen(path, "w");
    assert(f != NULL);
    fputs(content, f);
    fclose(f);
}

static void redirect_stdin(const char *path) {
    FILE *f = freopen(path, "r", stdin);
    assert(f != NULL);
}

static void restore_stdin_null(void) {
    freopen("NUL", "r", stdin);
}

static void test_daysInMonth(void) {
    start_test("daysInMonth");
    expect_true(daysInMonth(2023, 1) == 31, "Jan has 31 days (2023)");
    expect_true(daysInMonth(2023, 2) == 28, "Feb has 28 days (2023)");
    expect_true(daysInMonth(2024, 2) == 29, "Feb has 29 days (leap 2024)");
    expect_true(daysInMonth(2023, 4) == 30, "Apr has 30 days (2023)");
}

static void test_toLower(void) {
    start_test("toLower");
    char s[16];
    strcpy(s, "AbC123!");
    // Act
    toLower(s);
    // Assert
    expect_true(strcmp(s, "abc123!") == 0, "toLower converts letters only");
}

static void test_containsIgnoreCase(void) {
    start_test("containsIgnoreCase");
    expect_true(containsIgnoreCase("Hello World", "world") == 1, "find 'world' in 'Hello World'");
    expect_true(containsIgnoreCase("Payment", "pay") == 1, "prefix search case-insensitive");
    expect_true(containsIgnoreCase("Sample", "z") == 0, "non-existing substring returns 0");
}

static void test_findServiceMatches(void) {
    start_test("findServiceMatches");
    int idx[8];
    int m = findServiceMatches("internet", idx);
    expect_true(m >= 1, "at least one service matches 'internet'");
    expect_true(serviceTypeCount >= 1, "service types loaded (count >= 1)");
}

static void test_comparePayment(void) {
    start_test("comparePayment");
    Payment a, b;
    strcpy(a.paymentID, "P002");
    strcpy(b.paymentID, "P010");
    // Act & Assert
    expect_true(comparePayment(&a, &b) < 0, "P002 < P010");
    expect_true(comparePayment(&b, &a) > 0, "P010 > P002");
    strcpy(b.paymentID, "P002");
    expect_true(comparePayment(&a, &b) == 0, "P002 == P002");
}

static void test_generateNextPaymentID(void) {
    start_test("generateNextPaymentID");
    reset_state();
    char id[10];
    expect_true(generateNextPaymentID(id) == 1, "first ID generation returns 1");
    expect_true(strcmp(id, "P001") == 0, "first ID should be P001");

    // occupy P001 and P003
    strcpy(payments[0].paymentID, "P001");
    strcpy(payments[1].paymentID, "P003");
    count = 2;
    expect_true(generateNextPaymentID(id) == 1, "generation skips existing IDs");
    expect_true(strcmp(id, "P002") == 0, "next ID should be P002");
}

static void test_save_and_loadCSV(void) {
    start_test("saveCSV/loadCSV");
    reset_state();
    strcpy(payments[0].paymentID, "P001");
    strcpy(payments[0].payerName, "Alice Smith");
    strcpy(payments[0].serviceType, "Internet");
    payments[0].amount = 123.45f;
    strcpy(payments[0].paymentDate, "2024-01-31");

    strcpy(payments[1].paymentID, "P002");
    strcpy(payments[1].payerName, "Bob Lee");
    strcpy(payments[1].serviceType, "ATM");
    payments[1].amount = 50.00f;
    strcpy(payments[1].paymentDate, "2024-02-01");

    count = 2;
    saveCSV("unit_tmp.csv");

    // reset and load
    reset_state();
    loadCSV("unit_tmp.csv");
    expect_true(count == 2, "loaded 2 records from CSV");
    expect_true(strcmp(payments[0].paymentID, "P001") == 0 || strcmp(payments[0].paymentID, "P002") == 0, "IDs preserved after load");
    remove("unit_tmp.csv");
}

static void test_displayMenu_noop(void) {
    start_test("displayMenu (no-op)");
    int before = count;
    displayMenu();
    expect_true(count == before, "displayMenu does not change count in test env");
}

static void test_addPayment_flow(void) {
    start_test("addPayment flow");
    reset_state();
    int backed = backup_csv();
    (void)backed;
    write_input_file("unit_in_add.txt",
                    "John Doe\n"
                    "Internet\n"
                    "100\n"
                    "2024-01-31\n");
    redirect_stdin("unit_in_add.txt");
    addPayment();
    restore_stdin_null();
    remove("unit_in_add.txt");

    expect_true(count == 1, "count incremented to 1 after add");
    expect_true(strcmp(payments[0].payerName, "John Doe") == 0, "payer name stored");
    expect_true(strcmp(payments[0].serviceType, "Internet") == 0, "service type stored");
    expect_true(payments[0].amount == 100.0f, "amount stored");
    expect_true(strcmp(payments[0].paymentDate, "2024-01-31") == 0, "date stored");

    restore_csv();
}

static void test_searchPayment_by_id_no_mutation(void) {
    start_test("searchPayment by ID (no mutation)");
    reset_state();
    strcpy(payments[0].paymentID, "P001");
    strcpy(payments[0].payerName, "Jane Roe");
    strcpy(payments[0].serviceType, "ATM");
    payments[0].amount = 10.0f;
    strcpy(payments[0].paymentDate, "2024-02-02");
    count = 1;

    write_input_file("unit_in_search.txt",
                    "1\n"      // search by ID
                    "P001\n");
    redirect_stdin("unit_in_search.txt");
    searchPayment();
    restore_stdin_null();
    remove("unit_in_search.txt");

    expect_true(count == 1, "search does not change count");
    expect_true(strcmp(payments[0].payerName, "Jane Roe") == 0, "record unchanged after search");
}

static void test_updatePayment_amount(void) {
    start_test("updatePayment amount");
    reset_state();
    int backed = backup_csv();
    (void)backed;
    strcpy(payments[0].paymentID, "P050");
    strcpy(payments[0].payerName, "Chris P.");
    strcpy(payments[0].serviceType, "Website");
    payments[0].amount = 77.0f;
    strcpy(payments[0].paymentDate, "2024-03-01");
    count = 1;

    write_input_file("unit_in_update.txt",
                    "P050\n"   // ID to update
                    "3\n"      // choose Amount
                    "200\n"    // new amount
                    "0\n");   // finish
    redirect_stdin("unit_in_update.txt");
    updatePayment();
    restore_stdin_null();
    remove("unit_in_update.txt");

    expect_true(count == 1, "record count unchanged after update");
    expect_true(payments[0].amount == 200.0f, "amount updated to 200");

    restore_csv();
}

static void test_deletePayment_by_id(void) {
    start_test("deletePayment by ID");
    reset_state();
    int backed = backup_csv();
    (void)backed;
    strcpy(payments[0].paymentID, "P001");
    strcpy(payments[1].paymentID, "P002");
    count = 2;

    write_input_file("unit_in_delete.txt", "P001\n");
    redirect_stdin("unit_in_delete.txt");
    deletePayment();
    restore_stdin_null();
    remove("unit_in_delete.txt");

    expect_true(count == 1, "one record remains after delete");
    expect_true(strcmp(payments[0].paymentID, "P002") == 0, "remaining record is P002");

    restore_csv();
}

int main(void) {
    test_daysInMonth();
    test_toLower();
    test_containsIgnoreCase();
    test_findServiceMatches();
    test_comparePayment();
    test_generateNextPaymentID();
    test_save_and_loadCSV();
    test_displayMenu_noop();
    test_addPayment_flow();
    test_searchPayment_by_id_no_mutation();
    test_updatePayment_amount();
    test_deletePayment_by_id();

    // Newly added negative/edge tests
    {
        // CSV overflow guard: ensure load stops at MAX
        start_test("loadCSV caps at MAX");
        reset_state();
        FILE *f = fopen("unit_over.csv", "w");
        assert(f != NULL);
        for (int i = 1; i <= MAX + 5; i++) {
            fprintf(f, "P%03d,Name%d,ATM,10.00,2024-01-01\n", i, i);
        }
        fclose(f);
        loadCSV("unit_over.csv");
        expect_true(count == MAX, "loaded records are capped to MAX");
        remove("unit_over.csv");
    }

    {
        // Invalid ID rows are skipped
        start_test("loadCSV skips invalid ID row");
        reset_state();
        FILE *f = fopen("unit_badid.csv", "w");
        assert(f != NULL);
        fputs("X12,Bad,ATM,5.00,2024-01-01\n", f);
        fputs("P001,Good,ATM,7.00,2024-01-02\n", f);
        fclose(f);
        loadCSV("unit_badid.csv");
        expect_true(count == 1, "only valid ID loaded");
        expect_true(strcmp(payments[0].paymentID, "P001") == 0, "valid record present");
        remove("unit_badid.csv");
    }

    {
        // addPayment: invalid amount then valid
        start_test("addPayment rejects non-numeric amount and retries");
        reset_state();
        int backed = backup_csv();
        (void)backed;
        write_input_file("unit_in_add_bad_amount.txt",
                         "Person A\n"
                         "Internet\n"
                         "abc\n"    // invalid amount
                         "100\n"    // valid amount
                         "2024-01-31\n");
        redirect_stdin("unit_in_add_bad_amount.txt");
        addPayment();
        restore_stdin_null();
        remove("unit_in_add_bad_amount.txt");
        expect_true(count == 1, "record added after valid amount");
        expect_true(payments[0].amount == 100.0f, "amount equals 100 after retry");
        restore_csv();
    }

    printf("Unit Summary: %d passed, %d failed, %d total.\n", g_passed, g_failed, g_total);
    return (g_failed == 0) ? 0 : 1;
}
