#ifndef COURSE_OPS_H
#define COURSE_OPS_H

#include <string>
#include <vector>
using namespace std;

// Enum for enroll result
enum EnrollResult {
    ENROLL_SUCCESS,
    ENROLL_STUDENT_INACTIVE,
    ENROLL_COURSE_NOT_FOUND,
    ENROLL_NO_SEATS,
    ENROLL_ALREADY_ENROLLED,
    ENROLL_CREDIT_OVERLOAD,
    ENROLL_PREREQ_NOT_MET,
    ENROLL_STUDENT_NOT_FOUND
};

// Enroll a student in a course for a semester
EnrollResult enrollStudent(const string& roll, const string& courseCode, const string& semester);

// Drop a course (only if no attendance exists for roll+course+semester)
bool dropCourse(const string& roll, const string& courseCode, const string& semester);

// Get total credit hours for all active enrollments in a semester
int getCreditLoad(const string& roll, const string& semester);

// Check prerequisite: returns true if student passed the prereq course
bool checkPrerequisite(const string& roll, const string& courseCode);

// List all actively enrolled students in a course
vector<vector<string>> listEnrolledStudents(const string& courseCode);

// Print human-readable EnrollResult message
void printEnrollResult(EnrollResult r);

// Interactive menu for course operations
void courseMenu();

#endif
