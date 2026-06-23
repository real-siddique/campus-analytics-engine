#include "fee_tracker.h"
#include "filehandler.h"
#include "student_ops.h"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstdio>

using namespace std;

// fees.txt: 0=fee_id, 1=roll, 2=semester, 3=total_fee, 4=amount_paid, 5=due_date, 6=payment_date, 7=method, 8=status
#define FEES_FILE   "fees.txt"
#define FEES_HEADER "fee_id,roll_no,semester,total_fee,amount_paid,due_date,payment_date,payment_method,status"

static bool validateDate(const string& d) {
    if (d.length() != 10) return false;
    if (d[2] != '-' || d[5] != '-') return false;
    for (int i = 0; i < 10; i++) {
        if (i == 2 || i == 5) continue;
        if (!isdigit(d[i])) return false;
    }
    int day = atoi(d.substr(0,2).c_str());
    int mon = atoi(d.substr(3,2).c_str());
    int yr  = atoi(d.substr(6,4).c_str());
    return (day>=1 && day<=31 && mon>=1 && mon<=12 && yr>=2000 && yr<=2100);
}

static int dateToDays(const string& d) {
    int day = atoi(d.substr(0,2).c_str());
    int mon = atoi(d.substr(3,2).c_str());
    int yr  = atoi(d.substr(6,4).c_str());
    int mdays[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    int total = 0;
    for (int y = 1; y < yr; y++) {
        bool leap = (y%4==0 && y%100!=0)||(y%400==0);
        total += leap ? 366 : 365;
    }
    bool leap = (yr%4==0 && yr%100!=0)||(yr%400==0);
    if (leap) mdays[2] = 29;
    for (int m = 1; m < mon; m++) total += mdays[m];
    total += day;
    return total;
}

int daysBetween(const string& d1, const string& d2) {
    return dateToDays(d2) - dateToDays(d1);
}

bool recordPayment(const string& roll, const string& semester, double amount, const string& payDate, const string& method) {
    if (!validateDate(payDate)) { cout << "Error: Invalid date format. Use DD-MM-YYYY.\n"; return false; }
    vector<vector<string>> rows = readTXT(FEES_FILE);
    bool found = false;
    for (int i = 0; i < (int)rows.size(); i++) {
        if (rows[i].size() > 8 && rows[i][1] == roll && rows[i][2] == semester) {
            double total = atof(rows[i][3].c_str());
            double paid  = atof(rows[i][4].c_str()) + amount;
            if (paid > total) paid = total;
            char buf[16]; sprintf(buf, "%.2f", paid);
            rows[i][4] = string(buf);
            rows[i][6] = payDate;
            rows[i][7] = method;
            rows[i][8] = (paid >= total) ? "paid" : "partial";
            found = true; break;
        }
    }
    if (!found) { cout << "Fee record not found.\n"; return false; }
    writeTXT(FEES_FILE, FEES_HEADER, rows);
    cout << "Payment recorded.\n";
    return true;
}

double computeLateFine(const string& roll, const string& semester) {
    vector<vector<string>> rows = readTXT(FEES_FILE);
    for (int i = 0; i < (int)rows.size(); i++) {
        if (rows[i].size() > 8 && rows[i][1] == roll && rows[i][2] == semester) {
            string due  = rows[i][5];
            string paid = rows[i][6];
            if (!validateDate(paid) || !validateDate(due)) return 0.0;
            int diff = daysBetween(due, paid);
            if (diff <= 0) return 0.0;
            int weeks = diff / 7;
            double total = atof(rows[i][3].c_str());
            return total * 0.02 * weeks;
        }
    }
    return 0.0;
}

void generateReceipt(const string& roll, const string& semester) {
    vector<vector<string>> rows = readTXT(FEES_FILE);
    for (int i = 0; i < (int)rows.size(); i++) {
        if (rows[i].size() > 8 && rows[i][1] == roll && rows[i][2] == semester) {
            double total   = atof(rows[i][3].c_str());
            double paid    = atof(rows[i][4].c_str());
            double fine    = computeLateFine(roll, semester);
            double balance = (total + fine) - paid;
            cout << "\n" << string(50,'=') << "\n";
            cout << setw(35) << right << "FEE RECEIPT\n";
            cout << string(50,'=') << "\n";
            cout << left << setw(20) << "Roll No:"      << roll      << "\n";
            cout << left << setw(20) << "Semester:"     << semester  << "\n";
            cout << left << setw(20) << "Due Date:"     << rows[i][5] << "\n";
            cout << left << setw(20) << "Payment Date:" << rows[i][6] << "\n";
            cout << left << setw(20) << "Method:"       << rows[i][7] << "\n";
            cout << string(50,'-') << "\n";
            cout << left << setw(30) << "Tuition Fee:"  << right << setw(10) << fixed << setprecision(2) << total   << "\n";
            if (fine > 0) cout << left << setw(30) << "Late Fine:" << right << setw(10) << fine << "\n";
            cout << left << setw(30) << "Total Due:"    << right << setw(10) << (total+fine) << "\n";
            cout << left << setw(30) << "Amount Paid:"  << right << setw(10) << paid    << "\n";
            cout << string(50,'-') << "\n";
            cout << left << setw(30) << "Balance:"      << right << setw(10) << balance << "\n";
            cout << string(50,'=') << "\n";
            cout << (balance <= 0 ? "Status: FULLY PAID\n" : "Status: PENDING\n");
            cout << string(50,'=') << "\n";
            return;
        }
    }
    cout << "Fee record not found.\n";
}

vector<vector<string>> getDefaulters() {
    vector<vector<string>> rows = readTXT(FEES_FILE);
    vector<vector<string>> def;
    for (int i = 0; i < (int)rows.size(); i++) {
        if (rows[i].size() > 8) {
            double total = atof(rows[i][3].c_str());
            double paid  = atof(rows[i][4].c_str());
            if (paid < total) {
                char buf[16]; sprintf(buf,"%.2f", total-paid);
                vector<string> e = rows[i];
                e.push_back(string(buf));
                def.push_back(e);
            }
        }
    }
    // Bubble sort descending by balance
    for (int i = 0; i < (int)def.size(); i++)
        for (int j = 0; j < (int)def.size()-i-1; j++)
            if (atof(def[j].back().c_str()) < atof(def[j+1].back().c_str())) {
                vector<string> tmp = def[j]; def[j] = def[j+1]; def[j+1] = tmp;
            }
    return def;
}

void feeMenu() {
    int choice;
    do {
        cout << "\n===== FEE MANAGEMENT =====\n";
        cout << "1. Record Payment\n2. Compute Late Fine\n3. Generate Receipt\n4. View Defaulters\n0. Back\n";
        cout << "Choice: "; cin >> choice; cin.ignore();
        if (choice == 1) {
            string roll, sem, date, method; double amount;
            cout << "Roll: "; getline(cin,roll);
            cout << "Semester: "; getline(cin,sem);
            cout << "Amount: "; cin >> amount; cin.ignore();
            cout << "Payment date (DD-MM-YYYY): "; getline(cin,date);
            cout << "Method (Cash/Online/Bank): "; getline(cin,method);
            recordPayment(roll,sem,amount,date,method);
        } else if (choice == 2) {
            string roll, sem;
            cout << "Roll: "; getline(cin,roll);
            cout << "Semester: "; getline(cin,sem);
            cout << "Late fine: Rs. " << fixed << setprecision(2) << computeLateFine(roll,sem) << "\n";
        } else if (choice == 3) {
            string roll, sem;
            cout << "Roll: "; getline(cin,roll);
            cout << "Semester: "; getline(cin,sem);
            generateReceipt(roll,sem);
        } else if (choice == 4) {
            vector<vector<string>> d = getDefaulters();
            if (d.empty()) { cout << "No defaulters.\n"; }
            else {
                cout << "\n" << string(55,'-') << "\n";
                cout << left << setw(15)<<"Roll" << setw(5)<<"Sem" << setw(12)<<"Total" << setw(12)<<"Paid" << "Balance\n";
                cout << string(55,'-') << "\n";
                for (int i = 0; i < (int)d.size(); i++)
                    if (d[i].size() > 9)
                        cout << left << setw(15)<<d[i][1] << setw(5)<<d[i][2] << setw(12)<<d[i][3] << setw(12)<<d[i][4] << d[i].back() << "\n";
            }
        }
    } while (choice != 0);
}
