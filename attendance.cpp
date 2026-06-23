#include "attendance.h"
#include "filehandler.h"
#include "course_ops.h"
#include <iostream>
#include <iomanip>
#include <cstdio>

using namespace std;

// attendance_log.txt: 0=log_id, 1=roll, 2=course, 3=date, 4=status
#define ATTENDANCE_FILE "attendance_log.txt"
#define ATTENDANCE_HEADER "log_id,roll_no,course_code,session_date,status"

static vector<vector<string>> attendanceBackup;
static bool backupExists = false;

bool markAttendance(const string& courseCode, const string& semester, const string& date) {
    vector<vector<string>> enrolled = listEnrolledStudents(courseCode);
    if (enrolled.empty()) { cout << "No enrolled students in " << courseCode << ".\n"; return false; }

    vector<vector<string>> existing = readTXT(ATTENDANCE_FILE);
    // Check if already marked: col2=course, col3=date
    for (int i = 0; i < (int)existing.size(); i++) {
        if (existing[i].size() > 3 && existing[i][2] == courseCode && existing[i][3] == date) {
            cout << "Error: Attendance for " << courseCode << " on " << date << " already recorded.\n";
            return false;
        }
    }

    attendanceBackup = existing;
    backupExists = true;

    cout << "\n--- Marking Attendance for " << courseCode << " | Date: " << date << " ---\n";
    cout << "Enter P (Present), A (Absent), L (Late):\n";

    vector<vector<string>> newRows;
    int nextId = (int)existing.size() + 1;
    for (int i = 0; i < (int)enrolled.size(); i++) {
        string roll = enrolled[i][1]; // col1=roll in enrollments
        string status = "";
        while (status != "P" && status != "A" && status != "L") {
            cout << roll << ": "; cin >> status;
            for (int j = 0; j < (int)status.length(); j++) status[j] = toupper(status[j]);
            if (status != "P" && status != "A" && status != "L")
                cout << "Invalid. Enter P, A, or L.\n";
        }
        char idBuf[16];
        sprintf(idBuf, "L%05d", nextId++);
        vector<string> row;
        row.push_back(string(idBuf));
        row.push_back(roll);
        row.push_back(courseCode);
        row.push_back(date);
        row.push_back(status);
        newRows.push_back(row);
    }
    for (int i = 0; i < (int)newRows.size(); i++) appendTXT(ATTENDANCE_FILE, newRows[i]);
    cout << "Attendance recorded for " << newRows.size() << " students.\n";
    return true;
}

double getAttendancePct(const string& roll, const string& courseCode) {
    vector<vector<string>> rows = readTXT(ATTENDANCE_FILE);
    int totalSessions = 0;
    double weighted = 0.0;
    for (int i = 0; i < (int)rows.size(); i++) {
        // col1=roll, col2=course, col4=status
        if (rows[i].size() > 4 && rows[i][1] == roll && rows[i][2] == courseCode) {
            totalSessions++;
            if (rows[i][4] == "P") weighted += 1.0;
            else if (rows[i][4] == "L") weighted += 0.5;
        }
    }
    if (totalSessions == 0) return 0.0;
    return (weighted / totalSessions) * 100.0;
}

vector<vector<string>> getShortageList(const string& courseCode) {
    vector<vector<string>> enrolled = listEnrolledStudents(courseCode);
    vector<vector<string>> result;
    for (int i = 0; i < (int)enrolled.size(); i++) {
        string roll = enrolled[i][1]; // col1=roll
        double pct = getAttendancePct(roll, courseCode);
        if (pct < 75.0) {
            char buf[16]; sprintf(buf, "%.1f", pct);
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
    if (!backupExists) { cout << "Error: No backup available to undo.\n"; return false; }
    writeTXT(ATTENDANCE_FILE, ATTENDANCE_HEADER, attendanceBackup);
    backupExists = false;
    cout << "Last attendance session undone successfully.\n";
    return true;
}

void printDailySheet(const string& courseCode, const string& date) {
    vector<vector<string>> rows = readTXT(ATTENDANCE_FILE);
    cout << "\n" << string(40, '=') << "\n";
    cout << "Course: " << courseCode << " | Date: " << date << "\n";
    cout << string(40, '=') << "\n";
    cout << left << setw(15) << "Roll" << "Status\n";
    cout << string(25, '-') << "\n";
    int count = 0;
    for (int i = 0; i < (int)rows.size(); i++) {
        if (rows[i].size() > 4 && rows[i][2] == courseCode && rows[i][3] == date) {
            cout << left << setw(15) << rows[i][1] << rows[i][4] << "\n";
            count++;
        }
    }
    if (count == 0) cout << "No attendance found.\n";
    cout << string(40, '=') << "\n";
}

void attendanceMenu() {
    int choice;
    do {
        cout << "\n===== ATTENDANCE MANAGEMENT =====\n";
        cout << "1. Mark Attendance\n2. View Attendance %\n3. Shortage List (<75%)\n4. Undo Last Session\n5. Print Daily Sheet\n0. Back\n";
        cout << "Choice: "; cin >> choice; cin.ignore();

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
            cout << "Attendance: " << pct << "%\n";
            if (pct < 75.0) cout << "WARNING: Below 75%!\n";
        } else if (choice == 3) {
            string code;
            cout << "Course code: "; getline(cin, code);
            vector<vector<string>> s = getShortageList(code);
            if (s.empty()) cout << "No shortage students.\n";
            else {
                cout << left << setw(15) << "Roll" << "Attendance%\n";
                cout << string(30, '-') << "\n";
                for (int i = 0; i < (int)s.size(); i++)
                    cout << left << setw(15) << s[i][0] << s[i][2] << "%\n";
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
