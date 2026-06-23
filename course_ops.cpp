#include "course_ops.h"
#include "filehandler.h"
#include "student_ops.h"
#include <iostream>
#include <iomanip>
#include <cstdlib>

using namespace std;

// enrollments.txt columns: 0=id, 1=roll, 2=course, 3=semester, 4=date, 5=status
// courses.txt columns: 0=code, 1=name, 2=credits, 3=instructor, 4=capacity, 5=enrolled, 6=prereq
// attendance_log.txt columns: 0=id, 1=roll, 2=course, 3=date, 4=status

#define COURSES_FILE     "courses.txt"
#define ENROLLMENTS_FILE "enrollments.txt"
#define ATTENDANCE_FILE  "attendance_log.txt"
#define GRADES_FILE      "grades.txt"
#define ENROLLMENTS_HEADER "enrollment_id,roll_no,course_code,semester,enrollment_date,status"

void printEnrollResult(EnrollResult r) {
    switch(r) {
        case ENROLL_SUCCESS:           cout << "Enrollment successful.\n"; break;
        case ENROLL_STUDENT_INACTIVE:  cout << "Error: Student is not active.\n"; break;
        case ENROLL_COURSE_NOT_FOUND:  cout << "Error: Course not found.\n"; break;
        case ENROLL_NO_SEATS:          cout << "Error: No seats available.\n"; break;
        case ENROLL_ALREADY_ENROLLED:  cout << "Error: Student already enrolled.\n"; break;
        case ENROLL_CREDIT_OVERLOAD:   cout << "Error: Credit load would exceed 21 hours.\n"; break;
        case ENROLL_PREREQ_NOT_MET:    cout << "Error: Prerequisite course not passed.\n"; break;
        case ENROLL_STUDENT_NOT_FOUND: cout << "Error: Student not found.\n"; break;
    }
}

static int countEnrolled(const string& courseCode) {
    vector<vector<string>> rows = readTXT(ENROLLMENTS_FILE);
    int count = 0;
    for (int i = 0; i < (int)rows.size(); i++) {
        // col 2=course, col 5=status
        if (rows[i].size() > 5 && rows[i][2] == courseCode && rows[i][5] == "active") {
            count++;
        }
    }
    return count;
}

bool checkPrerequisite(const string& roll, const string& courseCode) {
    vector<string> course = findRow(COURSES_FILE, 0, courseCode);
    if (course.empty() || course.size() < 7) return true;
    string prereq = course[6];
    if (prereq == "NONE" || prereq.empty()) return true;

    vector<vector<string>> grades = readTXT(GRADES_FILE);
    for (int i = 0; i < (int)grades.size(); i++) {
        if (grades[i].size() > 8 && grades[i][0] == roll && grades[i][1] == prereq) {
            if (grades[i][8] != "F") return true;
        }
    }
    return false;
}

int getCreditLoad(const string& roll, const string& semester) {
    vector<vector<string>> enrollments = readTXT(ENROLLMENTS_FILE);
    vector<vector<string>> courses = readTXT(COURSES_FILE);
    int total = 0;
    for (int i = 0; i < (int)enrollments.size(); i++) {
        // col 1=roll, col 2=course, col 3=semester, col 5=status
        if (enrollments[i].size() > 5 &&
            enrollments[i][1] == roll &&
            enrollments[i][3] == semester &&
            enrollments[i][5] == "active") {
            string code = enrollments[i][2];
            for (int j = 0; j < (int)courses.size(); j++) {
                if (courses[j].size() > 2 && courses[j][0] == code) {
                    total += atoi(courses[j][2].c_str());
                    break;
                }
            }
        }
    }
    return total;
}

EnrollResult enrollStudent(const string& roll, const string& courseCode, const string& semester) {
    vector<string> student = searchByRoll(roll);
    if (student.empty()) return ENROLL_STUDENT_NOT_FOUND;
    if (student.size() < 6 || student[5] != "active") return ENROLL_STUDENT_INACTIVE;

    vector<string> course = findRow(COURSES_FILE, 0, courseCode);
    if (course.empty()) return ENROLL_COURSE_NOT_FOUND;

    // capacity is col 4
    if (course.size() < 5) return ENROLL_COURSE_NOT_FOUND;
    int capacity = atoi(course[4].c_str());
    int enrolled  = countEnrolled(courseCode);
    if (enrolled >= capacity) return ENROLL_NO_SEATS;

    // Check already enrolled: col1=roll, col2=course, col3=sem, col5=status
    vector<vector<string>> allEnroll = readTXT(ENROLLMENTS_FILE);
    for (int i = 0; i < (int)allEnroll.size(); i++) {
        if (allEnroll[i].size() > 5 &&
            allEnroll[i][1] == roll &&
            allEnroll[i][2] == courseCode &&
            allEnroll[i][3] == semester &&
            allEnroll[i][5] == "active") {
            return ENROLL_ALREADY_ENROLLED;
        }
    }

    // Credit load check: credits = col 2
    int credits = atoi(course[2].c_str());
    int currentLoad = getCreditLoad(roll, semester);
    if (currentLoad + credits > 21) return ENROLL_CREDIT_OVERLOAD;

    if (!checkPrerequisite(roll, courseCode)) return ENROLL_PREREQ_NOT_MET;

    // Generate new enrollment ID
    char idBuf[16];
    sprintf(idBuf, "E%04d", (int)allEnroll.size() + 1);

    vector<string> row;
    row.push_back(string(idBuf));
    row.push_back(roll);
    row.push_back(courseCode);
    row.push_back(semester);
    row.push_back("24-06-2024");
    row.push_back("active");
    appendTXT(ENROLLMENTS_FILE, row);
    return ENROLL_SUCCESS;
}

bool dropCourse(const string& roll, const string& courseCode, const string& semester) {
    // Check attendance: col1=roll, col2=course
    vector<vector<string>> att = readTXT(ATTENDANCE_FILE);
    for (int i = 0; i < (int)att.size(); i++) {
        if (att[i].size() > 2 && att[i][1] == roll && att[i][2] == courseCode) {
            cout << "Error: Cannot drop — attendance already recorded.\n";
            return false;
        }
    }

    vector<vector<string>> rows = readTXT(ENROLLMENTS_FILE);
    bool found = false;
    for (int i = 0; i < (int)rows.size(); i++) {
        if (rows[i].size() > 5 &&
            rows[i][1] == roll &&
            rows[i][2] == courseCode &&
            rows[i][3] == semester &&
            rows[i][5] == "active") {
            rows[i][5] = "dropped";
            found = true;
            break;
        }
    }
    if (!found) { cout << "Error: Enrollment record not found.\n"; return false; }
    writeTXT(ENROLLMENTS_FILE, ENROLLMENTS_HEADER, rows);
    cout << "Course " << courseCode << " dropped for " << roll << ".\n";
    return true;
}

vector<vector<string>> listEnrolledStudents(const string& courseCode) {
    vector<vector<string>> enrollments = readTXT(ENROLLMENTS_FILE);
    vector<vector<string>> result;
    for (int i = 0; i < (int)enrollments.size(); i++) {
        // col2=course, col5=status
        if (enrollments[i].size() > 5 &&
            enrollments[i][2] == courseCode &&
            enrollments[i][5] == "active") {
            result.push_back(enrollments[i]);
        }
    }
    return result;
}

void courseMenu() {
    int choice;
    do {
        cout << "\n===== COURSE MANAGEMENT =====\n";
        cout << "1. Enroll Student\n2. Drop Course\n3. Check Credit Load\n4. List Enrolled Students\n0. Back\n";
        cout << "Choice: ";
        cin >> choice; cin.ignore();

        if (choice == 1) {
            string roll, code, sem;
            cout << "Roll: "; getline(cin, roll);
            cout << "Course code: "; getline(cin, code);
            cout << "Semester: "; getline(cin, sem);
            printEnrollResult(enrollStudent(roll, code, sem));

        } else if (choice == 2) {
            string roll, code, sem;
            cout << "Roll: "; getline(cin, roll);
            cout << "Course code: "; getline(cin, code);
            cout << "Semester: "; getline(cin, sem);
            dropCourse(roll, code, sem);

        } else if (choice == 3) {
            string roll, sem;
            cout << "Roll: "; getline(cin, roll);
            cout << "Semester: "; getline(cin, sem);
            cout << "Credit load: " << getCreditLoad(roll, sem) << " hours\n";

        } else if (choice == 4) {
            string code;
            cout << "Course code: "; getline(cin, code);
            vector<vector<string>> students = listEnrolledStudents(code);
            if (students.empty()) { cout << "No students enrolled in " << code << ".\n"; }
            else {
                cout << "\nEnrolled in " << code << " (" << students.size() << " students):\n";
                cout << string(35, '-') << "\n";
                cout << left << setw(15) << "Roll" << setw(10) << "Semester" << "Status\n";
                cout << string(35, '-') << "\n";
                for (int i = 0; i < (int)students.size(); i++)
                    if (students[i].size() > 5)
                        cout << left << setw(15) << students[i][1]
                             << setw(10) << students[i][3]
                             << students[i][5] << "\n";
            }
        }
    } while (choice != 0);
}
