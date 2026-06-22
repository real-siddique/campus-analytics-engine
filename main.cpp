#include <iostream>
#include <string>
#include "filehandler.h"
#include "student_ops.h"
#include "course_ops.h"
#include "attendance.h"
#include "grades.h"
#include "fee_tracker.h"
#include "reports.h"

using namespace std;

void printBanner() {
    cout << "\n";
    cout << "  ============================================================\n";
    cout << "  |          CAMPUS ANALYTICS ENGINE                        |\n";
    cout << "  |          BS Artificial Intelligence                     |\n";
    cout << "  ============================================================\n";
}

void printMainMenu() {
    cout << "\n===== MAIN MENU =====\n";
    cout << "1. Student Management\n";
    cout << "2. Course Management\n";
    cout << "3. Attendance Management\n";
    cout << "4. Grades Management\n";
    cout << "5. Fee Management\n";
    cout << "6. Reports\n";
    cout << "0. Exit\n";
    cout << "Choice: ";
}

// Search-as-you-type bonus feature
void searchAsYouType() {
    cout << "\n--- SEARCH AS YOU TYPE ---\n";
    cout << "Type characters one by one. Press Enter on empty to stop.\n";
    string prefix = "";
    while (true) {
        cout << "Current prefix [" << prefix << "]: ";
        string ch;
        getline(cin, ch);
        if (ch.empty()) break;

        prefix += ch[0]; // take only the first character typed

        // Filter students whose name starts with prefix (prefix matching)
        vector<vector<string>> all = readTXT("students.txt");
        cout << "\nMatching students:\n";
        cout << "  Roll           Name\n";
        cout << "  " << string(35, '-') << "\n";
        int count = 0;
        for (int i = 0; i < (int)all.size(); i++) {
            if (all[i].size() > 1) {
                string name = all[i][1];
                // Prefix match using substr and length comparison
                if (name.length() >= prefix.length() &&
                    name.substr(0, prefix.length()) == prefix) {
                    cout << "  " << all[i][0] << "   " << name << "\n";
                    count++;
                }
            }
        }
        if (count == 0) cout << "  (no matches)\n";
    }
}

int main() {
    printBanner();

    int choice;
    do {
        printMainMenu();
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Invalid input. Please enter a number.\n";
            continue;
        }
        cin.ignore();

        switch (choice) {
            case 1: studentMenu();    break;
            case 2: courseMenu();     break;
            case 3: attendanceMenu(); break;
            case 4: gradesMenu();     break;
            case 5: feeMenu();        break;
            case 6: reportsMenu();    break;
            case 7: searchAsYouType(); break; // bonus (not shown in main menu)
            case 0: cout << "Goodbye!\n"; break;
            default: cout << "Invalid choice. Try again.\n";
        }
    } while (choice != 0);

    return 0;
}
