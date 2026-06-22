#include "student_ops.h"
#include "filehandler.h"
#include <iostream>
#include <iomanip>

using namespace std;

#define STUDENTS_FILE "students.txt"
#define STUDENTS_HEADER "roll_no,name,department,semester,cgpa,status"

// Validate roll format: BSAI-YY-XXX
bool validateRollFormat(const string& roll) {
    // Must be exactly 12 chars: BSAI-23-001
    if (roll.length() != 12) return false;
    // Check prefix "BSAI-"
    if (roll.substr(0, 5) != "BSAI-") return false;
    // Check YY (positions 5,6) are digits
    if (!isdigit(roll[5]) || !isdigit(roll[6])) return false;
    // Check dash at position 7
    if (roll[7] != '-') return false;
    // Check XXX (positions 8,9,10) are digits
    if (!isdigit(roll[8]) || !isdigit(roll[9]) || !isdigit(roll[10])) return false;
    // Extra char check (should only be 12 but substr already checked length)
    return true;
}

// Check name has no digits
static bool nameValid(const string& name) {
    if (name.empty()) return false;
    for (int i = 0; i < (int)name.length(); i++) {
        if (isdigit(name[i])) return false;
    }
    return true;
}

bool addStudent(const string& roll, const string& name, const string& dept,
                const string& semester, double cgpa) {
    if (!validateRollFormat(roll)) {
        cout << "Error: Invalid roll format. Use BSAI-YY-XXX (e.g. BSAI-23-001)\n";
        return false;
    }
    if (!nameValid(name)) {
        cout << "Error: Name must not contain digits and cannot be empty.\n";
        return false;
    }
    if (cgpa < 0.0 || cgpa > 4.0) {
        cout << "Error: CGPA must be between 0.0 and 4.0\n";
        return false;
    }
    if (rowExists(STUDENTS_FILE, 0, roll)) {
        cout << "Error: Student with roll " << roll << " already exists.\n";
        return false;
    }

    // Build CGPA string (2 decimal places)
    char cgpaBuf[16];
    sprintf(cgpaBuf, "%.2f", cgpa);

    vector<string> row;
    row.push_back(roll);
    row.push_back(name);
    row.push_back(dept);
    row.push_back(semester);
    row.push_back(string(cgpaBuf));
    row.push_back("active");

    appendTXT(STUDENTS_FILE, row);
    cout << "Student " << roll << " added successfully.\n";
    return true;
}

vector<string> searchByRoll(const string& roll) {
    return findRow(STUDENTS_FILE, 0, roll);
}

// Case-insensitive substring match
static bool containsIgnoreCase(const string& haystack, const string& needle) {
    if (needle.empty()) return true;
    if (haystack.length() < needle.length()) return false;

    string h = haystack, n = needle;
    for (int i = 0; i < (int)h.length(); i++) h[i] = tolower(h[i]);
    for (int i = 0; i < (int)n.length(); i++) n[i] = tolower(n[i]);

    for (int i = 0; i <= (int)h.length() - (int)n.length(); i++) {
        if (h.substr(i, n.length()) == n) return true;
    }
    return false;
}

vector<vector<string>> searchByName(const string& namePart) {
    vector<vector<string>> all = readTXT(STUDENTS_FILE);
    vector<vector<string>> result;
    for (int i = 0; i < (int)all.size(); i++) {
        if (all[i].size() > 1 && containsIgnoreCase(all[i][1], namePart)) {
            result.push_back(all[i]);
        }
    }
    return result;
}

bool updateStudent(const string& roll, int fieldIndex, const string& newValue) {
    if (fieldIndex == 0) {
        cout << "Error: Roll number cannot be updated.\n";
        return false;
    }
    vector<vector<string>> rows = readTXT(STUDENTS_FILE);
    bool found = false;
    for (int i = 0; i < (int)rows.size(); i++) {
        if (!rows[i].empty() && rows[i][0] == roll) {
            if (fieldIndex >= (int)rows[i].size()) {
                cout << "Error: Invalid field index.\n";
                return false;
            }
            rows[i][fieldIndex] = newValue;
            found = true;
            break;
        }
    }
    if (!found) {
        cout << "Error: Student " << roll << " not found.\n";
        return false;
    }
    writeTXT(STUDENTS_FILE, STUDENTS_HEADER, rows);
    cout << "Student " << roll << " updated successfully.\n";
    return true;
}

bool softDelete(const string& roll) {
    return updateStudent(roll, 5, "inactive");
}

// Selection sort on roll number (lexicographic is fine for BSAI-YY-XXX)
vector<vector<string>> listActiveStudents() {
    vector<vector<string>> all = readTXT(STUDENTS_FILE);
    vector<vector<string>> active;

    for (int i = 0; i < (int)all.size(); i++) {
        if (all[i].size() > 5 && all[i][5] == "active") {
            active.push_back(all[i]);
        }
    }

    // Selection sort by roll (column 0)
    for (int i = 0; i < (int)active.size(); i++) {
        int minIdx = i;
        for (int j = i + 1; j < (int)active.size(); j++) {
            if (active[j][0] < active[minIdx][0]) minIdx = j;
        }
        if (minIdx != i) {
            vector<string> temp = active[i];
            active[i] = active[minIdx];
            active[minIdx] = temp;
        }
    }
    return active;
}

static void printStudentRow(const vector<string>& s) {
    if (s.size() < 6) return;
    cout << left
         << setw(14) << s[0]
         << setw(22) << s[1]
         << setw(26) << s[2]
         << setw(5)  << s[3]
         << setw(7)  << s[4]
         << s[5] << "\n";
}

static void printStudentHeader() {
    cout << string(82, '-') << "\n";
    cout << left
         << setw(14) << "Roll No"
         << setw(22) << "Name"
         << setw(26) << "Department"
         << setw(5)  << "Sem"
         << setw(7)  << "CGPA"
         << "Status\n";
    cout << string(82, '-') << "\n";
}

void studentMenu() {
    int choice;
    do {
        cout << "\n===== STUDENT MANAGEMENT =====\n";
        cout << "1. Add Student\n";
        cout << "2. Search by Roll\n";
        cout << "3. Search by Name\n";
        cout << "4. Update Student\n";
        cout << "5. Delete Student (soft)\n";
        cout << "6. List All Active Students\n";
        cout << "0. Back\n";
        cout << "Choice: ";
        cin >> choice;
        cin.ignore();

        if (choice == 1) {
            string roll, name, dept, sem;
            double cgpa;
            cout << "Roll (BSAI-YY-XXX): "; getline(cin, roll);
            cout << "Name: "; getline(cin, name);
            cout << "Department: "; getline(cin, dept);
            cout << "Semester: "; getline(cin, sem);
            cout << "CGPA (0.0-4.0): "; cin >> cgpa; cin.ignore();
            addStudent(roll, name, dept, sem, cgpa);

        } else if (choice == 2) {
            string roll;
            cout << "Enter roll number: "; getline(cin, roll);
            vector<string> s = searchByRoll(roll);
            if (s.empty()) cout << "Student not found.\n";
            else { printStudentHeader(); printStudentRow(s); }

        } else if (choice == 3) {
            string name;
            cout << "Enter name (or part): "; getline(cin, name);
            vector<vector<string>> results = searchByName(name);
            if (results.empty()) cout << "No students found.\n";
            else {
                printStudentHeader();
                for (int i = 0; i < (int)results.size(); i++) printStudentRow(results[i]);
            }

        } else if (choice == 4) {
            string roll;
            cout << "Roll number: "; getline(cin, roll);
            cout << "Field to update:\n";
            cout << "  1=Name  2=Department  3=Semester  4=CGPA  5=Status\n";
            int fi; cin >> fi; cin.ignore();
            string val;
            cout << "New value: "; getline(cin, val);
            updateStudent(roll, fi, val);

        } else if (choice == 5) {
            string roll;
            cout << "Roll to delete (soft): "; getline(cin, roll);
            softDelete(roll);

        } else if (choice == 6) {
            vector<vector<string>> active = listActiveStudents();
            if (active.empty()) cout << "No active students.\n";
            else {
                cout << "\nActive Students (" << active.size() << "):\n";
                printStudentHeader();
                for (int i = 0; i < (int)active.size(); i++) printStudentRow(active[i]);
            }
        }
    } while (choice != 0);
}
