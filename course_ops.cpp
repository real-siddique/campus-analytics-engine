#include "course_ops.h"
#include "filehandler.h"
#include "student_ops.h"
#include <iostream>
#include <iomanip>
#include <cstdlib>

using namespace std;

#define COURSES_FILE    "courses.txt"
#define ENROLLMENTS_FILE "enrollments.txt"
#define ATTENDANCE_FILE  "attendance_log.txt"
#define GRADES_FILE      "grades.txt"
#define ENROLLMENTS_HEADER "roll_no,course_code,semester,status"

void printEnrollResult(EnrollResult r) {
    switch(r) {
        case ENROLL_SUCCESS:          cout << "Enrollment successful.\n"; break;
        case ENROLL_STUDENT_INACTIVE: cout << "Error: Student is not active.\n"; break;
        case ENROLL_COURSE_NOT_FOUND: cout << "Error: Course not found.\n"; break;
        case ENROLL_NO_SEATS:         cout << "Error: No seats available in the course.\n"; break;
        case ENROLL_ALREADY_ENROLLED: cout << "Error: Student is already enrolled in this course.\n"; break;
        case ENROLL_CREDIT_OVERLOAD:  cout << "Error: Credit load would exceed 21 hours.\n"; break;
        case ENROLL_PREREQ_NOT_MET:   cout << "Error: Prerequisite course not passed.\n"; break;
        case ENROLL_STUDENT_NOT_FOUND:cout << "Error: Student not found.\n"; break;
    }
}

// Count how many students are actively enrolled in a course
static int countEnrolled(const string& courseCode) {
    vector<vector<string>> rows = readTXT(ENROLLMENTS_FILE);
    int count = 0;
    for (int i = 0; i < (int)rows.size(); i++) {
        if (rows[i].size() > 3 && rows[i][1] == courseCode && rows[i][3] == "enrolled") {
            count++;
        }
    }
    return count;
}

bool checkPrerequisite(const string& roll, const string& courseCode) {
    // Find the prereq course code
    vector<string> course = findRow(COURSES_FILE, 0, courseCode);
    if (course.empty() || course.size() < 7) return true; // no prereq info = pass

    string prereq = course[6]; // prerequisite column
    if (prereq == "NONE" || prereq.empty()) return true;

    // Check if student has a non-F grade in the prereq course in grades.txt
    // grades.txt: roll_no,course_code,semester,quiz_avg,asgn_avg,mid,final,total,letter_grade
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
        if (enrollments[i].size() > 3 &&
            enrollments[i][0] == roll &&
            enrollments[i][2] == semester &&
            enrollments[i][3] == "enrolled") {

            string code = enrollments[i][1];
            // Find credits for this course
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
    // 1. Student exists?
    vector<string> student = searchByRoll(roll);
    if (student.empty()) return ENROLL_STUDENT_NOT_FOUND;

    // 2. Student active?
    if (student.size() < 6 || student[5] != "active") return ENROLL_STUDENT_INACTIVE;

    // 3. Course exists?
    vector<string> course = findRow(COURSES_FILE, 0, courseCode);
    if (course.empty()) return ENROLL_COURSE_NOT_FOUND;

    // 4. Seats available? course[4]=capacity, count enrolled
    if (course.size() < 5) return ENROLL_COURSE_NOT_FOUND;
    int capacity = atoi(course[4].c_str());
    int enrolled  = countEnrolled(courseCode);
    if (enrolled >= capacity) return ENROLL_NO_SEATS;

    // 5. Already enrolled?
    vector<vector<string>> allEnroll = readTXT(ENROLLMENTS_FILE);
    for (int i = 0; i < (int)allEnroll.size(); i++) {
        if (allEnroll[i].size() > 3 &&
            allEnroll[i][0] == roll &&
            allEnroll[i][1] == courseCode &&
            allEnroll[i][2] == semester &&
            allEnroll[i][3] == "enrolled") {
            return ENROLL_ALREADY_ENROLLED;
        }
    }

    // 6. Credit load <= 21?
    int credits = atoi(course[2].c_str());
    int currentLoad = getCreditLoad(roll, semester);
    if (currentLoad + credits > 21) return ENROLL_CREDIT_OVERLOAD;

    // 7. Prerequisite?
    if (!checkPrerequisite(roll, courseCode)) return ENROLL_PREREQ_NOT_MET;

    // All checks passed — append enrollment
    vector<string> row;
    row.push_back(roll);
    row.push_back(courseCode);
    row.push_back(semester);
    row.push_back("enrolled");
    appendTXT(ENROLLMENTS_FILE, row);
    return ENROLL_SUCCESS;
}

bool dropCourse(const string& roll, const string& courseCode, const string& semester) {
    // Check if any attendance rows exist for this combination
    vector<vector<string>> att = readTXT(ATTENDANCE_FILE);
    for (int i = 0; i < (int)att.size(); i++) {
        if (att[i].size() > 2 &&
            att[i][0] == roll &&
            att[i][1] == courseCode) {
            // Check semester via enrollment
            cout << "Error: Cannot drop course — attendance already recorded.\n";
            return false;
        }
    }

    // Update enrollment status to 'dropped'
    vector<vector<string>> rows = readTXT(ENROLLMENTS_FILE);
    bool found = false;
    for (int i = 0; i < (int)rows.size(); i++) {
        if (rows[i].size() > 3 &&
            rows[i][0] == roll &&
            rows[i][1] == courseCode &&
            rows[i][2] == semester &&
            rows[i][3] == "enrolled") {
            rows[i][3] = "dropped";
            found = true;
            break;
        }
    }
    if (!found) {
        cout << "Error: Enrollment record not found.\n";
        return false;
    }
    writeTXT(ENROLLMENTS_FILE, ENROLLMENTS_HEADER, rows);
    cout << "Course " << courseCode << " dropped for " << roll << ".\n";
    return true;
}

vector<vector<string>> listEnrolledStudents(const string& courseCode) {
    vector<vector<string>> enrollments = readTXT(ENROLLMENTS_FILE);
    vector<vector<string>> result;
    for (int i = 0; i < (int)enrollments.size(); i++) {
        if (enrollments[i].size() > 3 &&
            enrollments[i][1] == courseCode &&
            enrollments[i][3] == "enrolled") {
            result.push_back(enrollments[i]);
        }
    }
    return result;
}

void courseMenu() {
    int choice;
    do {
        cout << "\n===== COURSE MANAGEMENT =====\n";
        cout << "1. Enroll Student\n";
        cout << "2. Drop Course\n";
        cout << "3. Check Credit Load\n";
        cout << "4. List Enrolled Students\n";
        cout << "0. Back\n";
        cout << "Choice: ";
        cin >> choice;
        cin.ignore();

        if (choice == 1) {
            string roll, code, sem;
            cout << "Roll: "; getline(cin, roll);
            cout << "Course code: "; getline(cin, code);
            cout << "Semester: "; getline(cin, sem);
            EnrollResult r = enrollStudent(roll, code, sem);
            printEnrollResult(r);

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
            int load = getCreditLoad(roll, sem);
            cout << "Current credit load for " << roll << " in semester " << sem
                 << ": " << load << " hours\n";

        } else if (choice == 4) {
            string code;
            cout << "Course code: "; getline(cin, code);
            vector<vector<string>> students = listEnrolledStudents(code);
            if (students.empty()) cout << "No students enrolled in " << code << ".\n";
            else {
                cout << "\nEnrolled in " << code << " (" << students.size() << " students):\n";
                cout << string(40, '-') << "\n";
                cout << left << setw(15) << "Roll" << setw(12) << "Semester" << "Status\n";
                cout << string(40, '-') << "\n";
                for (int i = 0; i < (int)students.size(); i++) {
                    if (students[i].size() > 3)
                        cout << left << setw(15) << students[i][0]
                             << setw(12) << students[i][2]
                             << students[i][3] << "\n";
                }
            }
        }
    } while (choice != 0);
}
