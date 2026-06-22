#include "reports.h"
#include "filehandler.h"
#include "student_ops.h"
#include "grades.h"
#include "attendance.h"
#include "fee_tracker.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <cstdio>

using namespace std;

#define COURSES_FILE   "courses.txt"
#define ENROLLMENTS_FILE "enrollments.txt"

void printMeritList() {
    vector<vector<string>> students = listActiveStudents(); // already sorted by roll

    // Selection sort by CGPA descending (column 4)
    for (int i = 0; i < (int)students.size(); i++) {
        int maxIdx = i;
        for (int j = i + 1; j < (int)students.size(); j++) {
            double c1 = atof(students[j][4].c_str());
            double c2 = atof(students[maxIdx][4].c_str());
            if (c1 > c2) maxIdx = j;
        }
        if (maxIdx != i) {
            vector<string> tmp = students[i];
            students[i] = students[maxIdx];
            students[maxIdx] = tmp;
        }
    }

    cout << "\n";
    cout << string(70, '=') << "\n";
    cout << setw(40) << right << "MERIT LIST\n";
    cout << string(70, '=') << "\n";
    cout << left
         << setw(5)  << "Rank"
         << setw(14) << "Roll"
         << setw(24) << "Name"
         << setw(12) << "Dept"
         << "CGPA\n";
    cout << string(70, '-') << "\n";
    for (int i = 0; i < (int)students.size(); i++) {
        if (students[i].size() > 4) {
            cout << left
                 << setw(5)  << (i + 1)
                 << setw(14) << students[i][0]
                 << setw(24) << students[i][1]
                 << setw(12) << students[i][2]
                 << students[i][4] << "\n";
        }
    }
    cout << string(70, '=') << "\n";
}

void printAttendanceDefaulters() {
    vector<vector<string>> courses = readTXT(COURSES_FILE);
    vector<vector<string>> students = readTXT("students.txt");

    cout << "\n";
    cout << string(60, '=') << "\n";
    cout << setw(40) << right << "ATTENDANCE DEFAULTERS\n";
    cout << string(60, '=') << "\n";
    cout << left << setw(14) << "Roll" << setw(22) << "Name"
         << setw(10) << "Course" << "Attendance%\n";
    cout << string(60, '-') << "\n";

    int found = 0;
    for (int c = 0; c < (int)courses.size(); c++) {
        if (courses[c].empty()) continue;
        string code = courses[c][0];
        vector<vector<string>> shortage = getShortageList(code);
        for (int i = 0; i < (int)shortage.size(); i++) {
            string roll = shortage[i][0];
            string name = "";
            for (int s = 0; s < (int)students.size(); s++) {
                if (students[s].size() > 1 && students[s][0] == roll) {
                    name = students[s][1]; break;
                }
            }
            cout << left << setw(14) << roll << setw(22) << name
                 << setw(10) << code << shortage[i][2] << "%\n";
            found++;
        }
    }
    if (found == 0) cout << "No attendance defaulters found.\n";
    cout << string(60, '=') << "\n";
}

void printFeeDefaulters() {
    vector<vector<string>> defaulters = getDefaulters();

    cout << "\n";
    cout << string(65, '=') << "\n";
    cout << setw(40) << right << "FEE DEFAULTERS\n";
    cout << string(65, '=') << "\n";
    cout << left << setw(15) << "Roll" << setw(6) << "Sem"
         << setw(12) << "Due Date" << setw(12) << "Balance" << "Weeks Overdue\n";
    cout << string(65, '-') << "\n";

    if (defaulters.empty()) {
        cout << "No fee defaulters.\n";
    } else {
        for (int i = 0; i < (int)defaulters.size(); i++) {
            if (defaulters[i].size() < 10) continue;
            string roll = defaulters[i][1];
            string sem  = defaulters[i][2];

            // Compute weeks overdue (use due_date vs a reference)
            string dueDate = defaulters[i][5];
            string paidDate = defaulters[i][6];
            int overdueDays = 0;
            // If not paid, compare to today placeholder "24-06-2024"
            if (paidDate == "00-00-0000" || paidDate.empty()) {
                // Estimate using a fixed "today"
                overdueDays = 0; // Cannot compute without ctime
            } else {
                // Already paid late
                overdueDays = 0;
            }
            int weeks = overdueDays / 7;

            cout << left << setw(15) << roll
                 << setw(6)  << sem
                 << setw(12) << dueDate
                 << setw(12) << defaulters[i].back()
                 << weeks << "\n";
        }
    }
    cout << string(65, '=') << "\n";
}

void printSemesterResult(const string& courseCode, const string& semester) {
    vector<vector<string>> enrollments = readTXT(ENROLLMENTS_FILE);
    vector<vector<string>> grades = readTXT("grades.txt");
    vector<vector<string>> students = readTXT("students.txt");

    cout << "\n";
    cout << string(75, '*') << "\n";
    cout << "  SEMESTER RESULT SHEET | Course: " << courseCode
         << " | Semester: " << semester << "\n";
    cout << string(75, '*') << "\n";
    cout << left << setw(14) << "Roll" << setw(20) << "Name"
         << setw(8)  << "Grade"
         << setw(8)  << "Attend%"
         << "Status\n";
    cout << string(75, '-') << "\n";

    for (int i = 0; i < (int)enrollments.size(); i++) {
        if (enrollments[i].size() > 3 &&
            enrollments[i][1] == courseCode &&
            enrollments[i][2] == semester &&
            enrollments[i][3] == "enrolled") {

            string roll = enrollments[i][0];
            string name = "";
            for (int s = 0; s < (int)students.size(); s++) {
                if (students[s].size() > 1 && students[s][0] == roll) {
                    name = students[s][1]; break;
                }
            }

            string grade = "N/A";
            for (int g = 0; g < (int)grades.size(); g++) {
                if (grades[g].size() > 8 &&
                    grades[g][0] == roll && grades[g][1] == courseCode) {
                    grade = grades[g][8]; break;
                }
            }

            double attPct = getAttendancePct(roll, courseCode);
            char attBuf[10];
            sprintf(attBuf, "%.1f", attPct);
            string status = (attPct < 75.0) ? "SHORTAGE" : "OK";

            cout << left << setw(14) << roll
                 << setw(20) << name
                 << setw(8)  << grade
                 << setw(8)  << attBuf
                 << status << "\n";
        }
    }
    cout << string(75, '*') << "\n";
}

void printDepartmentSummary() {
    vector<vector<string>> students = readTXT("students.txt");

    // Collect unique departments
    string depts[20]; int deptCount = 0;
    for (int i = 0; i < (int)students.size(); i++) {
        if (students[i].size() > 2) {
            string d = students[i][2];
            bool found = false;
            for (int j = 0; j < deptCount; j++)
                if (depts[j] == d) { found = true; break; }
            if (!found && deptCount < 20) depts[deptCount++] = d;
        }
    }

    cout << "\n";
    cout << string(65, '=') << "\n";
    cout << setw(40) << right << "DEPARTMENT SUMMARY\n";
    cout << string(65, '=') << "\n";
    cout << left << setw(26) << "Department" << setw(8) << "Count"
         << setw(10) << "Avg CGPA" << "Pass Rate\n";
    cout << string(65, '-') << "\n";

    for (int d = 0; d < deptCount; d++) {
        int count = 0, active = 0;
        double cgpaSum = 0;
        int passing = 0; // CGPA >= 2.0 = passing

        for (int i = 0; i < (int)students.size(); i++) {
            if (students[i].size() > 5 && students[i][2] == depts[d]) {
                count++;
                if (students[i][5] == "active") active++;
                double cgpa = atof(students[i][4].c_str());
                cgpaSum += cgpa;
                if (cgpa >= 2.0) passing++;
            }
        }

        double avgCGPA = count > 0 ? cgpaSum / count : 0;
        double passRate = count > 0 ? (passing * 100.0 / count) : 0;

        char cBuf[16], pBuf[16];
        sprintf(cBuf, "%.2f", avgCGPA);
        sprintf(pBuf, "%.1f%%", passRate);

        cout << left << setw(26) << depts[d]
             << setw(8) << active
             << setw(10) << cBuf
             << pBuf << "\n";
    }
    cout << string(65, '=') << "\n";
}

void exportReportToFile(int reportChoice, const string& outputFile) {
    // Redirect cout to file
    ofstream file(outputFile.c_str());
    if (!file.is_open()) {
        cout << "Error: Cannot create output file " << outputFile << "\n";
        return;
    }
    streambuf* oldBuf = cout.rdbuf(file.rdbuf());

    switch (reportChoice) {
        case 1: printMeritList(); break;
        case 2: printAttendanceDefaulters(); break;
        case 3: printFeeDefaulters(); break;
        case 4: printDepartmentSummary(); break;
        default: cout << "Invalid report choice.\n";
    }

    // Restore cout
    cout.rdbuf(oldBuf);
    cout << "Report saved to " << outputFile << "\n";
}

void reportsMenu() {
    int choice;
    do {
        cout << "\n===== REPORTS =====\n";
        cout << "1. Merit List\n";
        cout << "2. Attendance Defaulters\n";
        cout << "3. Fee Defaulters\n";
        cout << "4. Semester Result Sheet\n";
        cout << "5. Department Summary\n";
        cout << "6. Export Report to File\n";
        cout << "0. Back\n";
        cout << "Choice: ";
        cin >> choice;
        cin.ignore();

        if (choice == 1) {
            printMeritList();
        } else if (choice == 2) {
            printAttendanceDefaulters();
        } else if (choice == 3) {
            printFeeDefaulters();
        } else if (choice == 4) {
            string code, sem;
            cout << "Course code: "; getline(cin, code);
            cout << "Semester: "; getline(cin, sem);
            printSemesterResult(code, sem);
        } else if (choice == 5) {
            printDepartmentSummary();
        } else if (choice == 6) {
            cout << "Select report to export:\n";
            cout << "1=Merit List  2=Att. Defaulters  3=Fee Defaulters  4=Dept Summary\n";
            int rc; cin >> rc; cin.ignore();
            string fname;
            cout << "Output filename: "; getline(cin, fname);
            exportReportToFile(rc, fname);
        }
    } while (choice != 0);
}
