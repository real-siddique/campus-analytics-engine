#include "fee_tracker.h"
#include "filehandler.h"
#include "student_ops.h"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstdio>

using namespace std;

#define FEES_FILE   "fees.txt"
#define FEES_HEADER "fee_id,roll_no,semester,total_fee,amount_paid,due_date,payment_date,payment_method,status"

// Validate DD-MM-YYYY format
static bool validateDate(const string& d) {
    if (d.length() != 10) return false;
    if (d[2] != '-' || d[5] != '-') return false;
    for (int i = 0; i < 10; i++) {
        if (i == 2 || i == 5) continue;
        if (!isdigit(d[i])) return false;
    }
    int day   = atoi(d.substr(0, 2).c_str());
    int month = atoi(d.substr(3, 2).c_str());
    int year  = atoi(d.substr(6, 4).c_str());
    if (month < 1 || month > 12) return false;
    if (day < 1 || day > 31) return false;
    if (year < 2000 || year > 2100) return false;
    return true;
}

// Convert DD-MM-YYYY to total days since a fixed epoch (manual, no ctime)
static int dateToDays(const string& d) {
    int day   = atoi(d.substr(0, 2).c_str());
    int month = atoi(d.substr(3, 2).c_str());
    int year  = atoi(d.substr(6, 4).c_str());

    // Days in each month (non-leap year)
    int monthDays[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // Count days from year 1 to (year-1)
    int totalDays = 0;
    for (int y = 1; y < year; y++) {
        bool leap = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
        totalDays += leap ? 366 : 365;
    }

    // Add days for months in current year
    bool leapYear = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    if (leapYear) monthDays[2] = 29;
    for (int m = 1; m < month; m++) totalDays += monthDays[m];
    totalDays += day;

    return totalDays;
}

int daysBetween(const string& d1, const string& d2) {
    return dateToDays(d2) - dateToDays(d1);
}

bool recordPayment(const string& roll, const string& semester,
                   double amount, const string& payDate, const string& method) {
    if (!validateDate(payDate)) {
        cout << "Error: Invalid date format. Use DD-MM-YYYY.\n";
        return false;
    }

    vector<vector<string>> rows = readTXT(FEES_FILE);
    bool found = false;

    for (int i = 0; i < (int)rows.size(); i++) {
        if (rows[i].size() > 2 && rows[i][1] == roll && rows[i][2] == semester) {
            double totalFee  = atof(rows[i][3].c_str());
            double alreadyPaid = atof(rows[i][4].c_str());
            double newPaid = alreadyPaid + amount;
            if (newPaid > totalFee) newPaid = totalFee;

            char buf[16];
            sprintf(buf, "%.2f", newPaid);
            rows[i][4] = string(buf);
            rows[i][6] = payDate;
            rows[i][7] = method;
            rows[i][8] = (newPaid >= totalFee) ? "paid" : "partial";

            found = true;
            break;
        }
    }

    if (!found) {
        cout << "Error: Fee record for " << roll << " semester " << semester << " not found.\n";
        return false;
    }

    writeTXT(FEES_FILE, FEES_HEADER, rows);
    cout << "Payment recorded.\n";
    return true;
}

double computeLateFine(const string& roll, const string& semester) {
    vector<vector<string>> rows = readTXT(FEES_FILE);
    for (int i = 0; i < (int)rows.size(); i++) {
        if (rows[i].size() > 8 && rows[i][1] == roll && rows[i][2] == semester) {
            string dueDate  = rows[i][5];
            string paidDate = rows[i][6];
            if (!validateDate(paidDate) || !validateDate(dueDate)) return 0.0;

            int diff = daysBetween(dueDate, paidDate);
            if (diff <= 0) return 0.0; // paid on time or early

            int completeWeeks = diff / 7;
            double totalFee = atof(rows[i][3].c_str());
            return totalFee * 0.02 * completeWeeks;
        }
    }
    return 0.0;
}

void generateReceipt(const string& roll, const string& semester) {
    vector<vector<string>> rows = readTXT(FEES_FILE);
    for (int i = 0; i < (int)rows.size(); i++) {
        if (rows[i].size() > 8 && rows[i][1] == roll && rows[i][2] == semester) {
            double totalFee   = atof(rows[i][3].c_str());
            double amtPaid    = atof(rows[i][4].c_str());
            double lateFine   = computeLateFine(roll, semester);
            double balance    = (totalFee + lateFine) - amtPaid;

            cout << "\n";
            cout << string(50, '=') << "\n";
            cout << setw(30) << right << "FEE RECEIPT\n";
            cout << string(50, '=') << "\n";
            cout << left << setw(20) << "Roll No:"     << roll << "\n";
            cout << left << setw(20) << "Semester:"    << semester << "\n";
            cout << left << setw(20) << "Due Date:"    << rows[i][5] << "\n";
            cout << left << setw(20) << "Payment Date:"<< rows[i][6] << "\n";
            cout << left << setw(20) << "Method:"      << rows[i][7] << "\n";
            cout << string(50, '-') << "\n";
            cout << left << setw(30) << "Tuition Fee:"
                 << right << setw(10) << fixed << setprecision(2) << totalFee << "\n";
            if (lateFine > 0) {
                cout << left << setw(30) << "Late Fine:"
                     << right << setw(10) << lateFine << "\n";
            }
            cout << left << setw(30) << "Total Due:"
                 << right << setw(10) << (totalFee + lateFine) << "\n";
            cout << left << setw(30) << "Amount Paid:"
                 << right << setw(10) << amtPaid << "\n";
            cout << string(50, '-') << "\n";
            cout << left << setw(30) << "Balance:"
                 << right << setw(10) << balance << "\n";
            cout << string(50, '=') << "\n";
            if (balance <= 0) cout << "Status: FULLY PAID\n";
            else cout << "Status: PENDING\n";
            cout << string(50, '=') << "\n";
            return;
        }
    }
    cout << "Fee record not found for " << roll << " semester " << semester << ".\n";
}

vector<vector<string>> getDefaulters() {
    vector<vector<string>> rows = readTXT(FEES_FILE);
    vector<vector<string>> defaulters;
    // Today's reference: we use a fixed "today" for comparison
    // Students whose balance > 0 are defaulters
    for (int i = 0; i < (int)rows.size(); i++) {
        if (rows[i].size() > 8) {
            double totalFee = atof(rows[i][3].c_str());
            double paid     = atof(rows[i][4].c_str());
            string status   = rows[i][8];
            if (paid < totalFee && status != "paid") {
                double balance = totalFee - paid;
                char buf[16];
                sprintf(buf, "%.2f", balance);
                vector<string> entry = rows[i];
                entry.push_back(string(buf)); // add balance as last field
                defaulters.push_back(entry);
            }
        }
    }

    // Bubble sort by balance (descending) — balance is last field added
    for (int i = 0; i < (int)defaulters.size(); i++) {
        for (int j = 0; j < (int)defaulters.size() - i - 1; j++) {
            double b1 = atof(defaulters[j].back().c_str());
            double b2 = atof(defaulters[j+1].back().c_str());
            if (b1 < b2) {
                vector<string> tmp = defaulters[j];
                defaulters[j] = defaulters[j+1];
                defaulters[j+1] = tmp;
            }
        }
    }
    return defaulters;
}

void feeMenu() {
    int choice;
    do {
        cout << "\n===== FEE MANAGEMENT =====\n";
        cout << "1. Record Payment\n";
        cout << "2. Compute Late Fine\n";
        cout << "3. Generate Receipt\n";
        cout << "4. View Defaulters\n";
        cout << "0. Back\n";
        cout << "Choice: ";
        cin >> choice;
        cin.ignore();

        if (choice == 1) {
            string roll, sem, date, method;
            double amount;
            cout << "Roll: "; getline(cin, roll);
            cout << "Semester: "; getline(cin, sem);
            cout << "Amount: "; cin >> amount; cin.ignore();
            cout << "Payment date (DD-MM-YYYY): "; getline(cin, date);
            cout << "Method (Cash/Online/Bank): "; getline(cin, method);
            recordPayment(roll, sem, amount, date, method);

        } else if (choice == 2) {
            string roll, sem;
            cout << "Roll: "; getline(cin, roll);
            cout << "Semester: "; getline(cin, sem);
            double fine = computeLateFine(roll, sem);
            cout << "Late fine for " << roll << ": Rs. " << fixed << setprecision(2) << fine << "\n";

        } else if (choice == 3) {
            string roll, sem;
            cout << "Roll: "; getline(cin, roll);
            cout << "Semester: "; getline(cin, sem);
            generateReceipt(roll, sem);

        } else if (choice == 4) {
            vector<vector<string>> d = getDefaulters();
            if (d.empty()) { cout << "No fee defaulters.\n"; }
            else {
                cout << "\nFee Defaulters:\n";
                cout << string(55, '-') << "\n";
                cout << left << setw(15) << "Roll" << setw(6) << "Sem"
                     << setw(12) << "Total" << setw(12) << "Paid"
                     << "Balance\n";
                cout << string(55, '-') << "\n";
                for (int i = 0; i < (int)d.size(); i++) {
                    if (d[i].size() > 9)
                        cout << left << setw(15) << d[i][1]
                             << setw(6)  << d[i][2]
                             << setw(12) << d[i][3]
                             << setw(12) << d[i][4]
                             << d[i].back() << "\n";
                }
            }
        }
    } while (choice != 0);
}
