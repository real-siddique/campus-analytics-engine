#include "attendance.h"
#include "filehandler.h"
#include "course_ops.h"
#include <iostream>
#include <iomanip>
#include <cstdio>

using namespace std;

#define ATTENDANCE_FILE "attendance_log.txt"
#define ATTENDANCE_HEADER "roll_no,course_code,date,status"

// In-memory backup for undo (stores rows before last session write)
static vector<vector<string>> attendanceBackup;
static bool backupExists = false;

bool markAttendance(const string& courseCode, const string& semester, const string& date) {
    // Get all enrolled students in this course
    vector<vector<string>> enrolled = listEnrolledStudents(courseCode);
    if (enrolled.empty()) {
        cout << "No enrolled students in course " << courseCode << ".\n";
        return false;
    }

    // Check date not already marked for this course
    vector<vector<string>> existing = readTXT(ATTENDANCE_FILE);
    for (int i = 0; i < (int)existing.size(); i++) {
        if (existing[i].size() > 2 &&
            existing[i][1] == courseCode &&
            existing[i][2] == date) {
            cout << "Error: Attendance for " << courseCode << " on " << date << " already recorded.\n";
            return false;
        }
    }

    // Save backup before writing
    attendanceBackup = existing;
    backupExists = true;

    cout << "\n--- Marking Attendance for " << courseCode << " | Date: " << date << " ---\n";
    cout << "Enter P (Present), A (Absent), L (Late) for each student:\n";

    vector<vector<string>> newRows;
    for (int i = 0; i < (int)enrolled.size(); i++) {
        string roll = enrolled[i][0];
        string status = "";
        while (status != "P" && status != "A" && status != "L") {
            cout << roll << ": ";
            cin >> status;
            // Convert to uppercase
            for (int j = 0; j < (int)status.length(); j++)
                status[j] = toupper(status[j]);
            if (status != "P" && status != "A" && status != "L")
                cout << "Invalid. Enter P, A, or L.\n";
        }
        vector<string> row;
        row.push_back(roll);
        row.push_back(courseCode);
        row.push_back(date);
        row.push_back(status);
        newRows.push_back(row);
    }

    // Append all new rows
    for (int i = 0; i < (int)newRows.size(); i++) {
        appendTXT(ATTENDANCE_FILE, newRows[i]);
    }
    cout << "Attendance recorded for " << newRows.size() << " students.\n";
    return true;
}

double getAttendancePct(const string& roll, const string& courseCode) {
    vector<vector<string>> rows = readTXT(ATTENDANCE_FILE);
    int totalSessions = 0;
    double weightedPresent = 0.0;

    for (int i = 0; i < (int)rows.size(); i++) {
        if (rows[i].size() > 3 &&
            rows[i][0] == roll &&
            rows[i][1] == courseCode) {
            totalSessions++;
            if (rows[i][3] == "P") weightedPresent += 1.0;
            else if (rows[i][3] == "L") weightedPresent += 0.5;
            // A = 0
        }
    }
    if (totalSessions == 0) return 0.0;
    return (weightedPresent / totalSessions) * 100.0;
}

vector<vector<string>> getShortageList(const string& courseCode) {
    vector<vector<string>> enrolled = listEnrolledStudents(courseCode);
    vector<vector<string>> result;

    for (int i = 0; i < (int)enrolled.size(); i++) {
        string roll = enrolled[i][0];
        double pct = getAttendancePct(roll, courseCode);
        if (pct < 75.0) {
            char buf[16];
            sprintf(buf, "%.1f", pct);
            vector<string> entry;
            entry.push_back(roll);
            entry.push_back(courseCode);
            entry.push_back(string(buf));
            result.push_back(entry);
        }
    }
    return result;
}

bool undoLastSession() {
    if (!backupExists) {
        cout << "Error: No backup available to undo.\n";
        return false;
    }
    writeTXT(ATTENDANCE_FILE, ATTENDANCE_HEADER, attendanceBackup);
    backupExists = false;
    cout << "Last attendance session undone successfully.\n";
    return true;
}

void printDailySheet(const string& courseCode, const string& date) {
    vector<vector<string>> rows = readTXT(ATTENDANCE_FILE);
    cout << "\n========================================\n";
    cout << "Attendance Sheet | Course: " << courseCode << " | Date: " << date << "\n";
    cout << "========================================\n";
    cout << left << setw(15) << "Roll" << setw(10) << "Status\n";
    cout << string(25, '-') << "\n";

    int count = 0;
    for (int i = 0; i < (int)rows.size(); i++) {
        if (rows[i].size() > 3 &&
            rows[i][1] == courseCode &&
            rows[i][2] == date) {
            cout << left << setw(15) << rows[i][0] << rows[i][3] << "\n";
            count++;
        }
    }
    if (count == 0) cout << "No attendance found for this course/date.\n";
    cout << "========================================\n";
}

void attendanceMenu() {
    int choice;
    do {
        cout << "\n===== ATTENDANCE MANAGEMENT =====\n";
        cout << "1. Mark Attendance\n";
        cout << "2. View Attendance %\n";
        cout << "3. Shortage List (<75%)\n";
        cout << "4. Undo Last Session\n";
        cout << "5. Print Daily Sheet\n";
        cout << "0. Back\n";
        cout << "Choice: ";
        cin >> choice;
        cin.ignore();

        if (choice == 1) {
            string code, sem, date;
            cout << "Course code: "; getline(cin, code);
            cout << "Semester: "; getline(cin, sem);
            cout << "Date (DD-MM-YYYY): "; getline(cin, date);
            markAttendance(code, sem, date);

        } else if (choice == 2) {
            string roll, code;
            cout << "Roll: "; getline(cin, roll);
            cout << "Course code: "; getline(cin, code);
            double pct = getAttendancePct(roll, code);
            cout << "Attendance for " << roll << " in " << code << ": " << pct << "%\n";
            if (pct < 75.0) cout << "WARNING: Below 75% threshold!\n";

        } else if (choice == 3) {
            string code;
            cout << "Course code: "; getline(cin, code);
            vector<vector<string>> shortage = getShortageList(code);
            if (shortage.empty()) cout << "No shortage students in " << code << ".\n";
            else {
                cout << "\nAttendance Shortage in " << code << ":\n";
                cout << left << setw(15) << "Roll" << "Attendance%\n";
                cout << string(30, '-') << "\n";
                for (int i = 0; i < (int)shortage.size(); i++)
                    cout << left << setw(15) << shortage[i][0] << shortage[i][2] << "%\n";
            }

        } else if (choice == 4) {
            undoLastSession();

        } else if (choice == 5) {
            string code, date;
            cout << "Course code: "; getline(cin, code);
            cout << "Date (DD-MM-YYYY): "; getline(cin, date);
            printDailySheet(code, date);
        }
    } while (choice != 0);
}
