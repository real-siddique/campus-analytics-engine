#ifndef FEE_TRACKER_H
#define FEE_TRACKER_H

#include <string>
#include <vector>
using namespace std;

// Record a payment for a student (validates DD-MM-YYYY format)
bool recordPayment(const string& roll, const string& semester,
                   double amount, const string& payDate, const string& method);

// Compute late fine: 2% per complete week after due_date
double computeLateFine(const string& roll, const string& semester);

// Manual date difference (no ctime): parses DD-MM-YYYY
// Returns number of days between d1 and d2 (can be negative)
int daysBetween(const string& d1, const string& d2);

// Print formatted receipt for a student's fee record
void generateReceipt(const string& roll, const string& semester);

// Get all students with outstanding balance past due date
// Returns rows sorted by outstanding amount (bubble sort, descending)
vector<vector<string>> getDefaulters();

// Interactive menu for fee operations
void feeMenu();

#endif
