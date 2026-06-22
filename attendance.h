#ifndef ATTENDANCE_H
#define ATTENDANCE_H

#include <string>
#include <vector>
using namespace std;

// Mark attendance for all enrolled students in a course on a given date
// Returns false if no enrolled students found
bool markAttendance(const string& courseCode, const string& semester, const string& date);

// Compute attendance percentage for a student in a course
// Formula: (present + 0.5 * late) / totalSessions * 100
double getAttendancePct(const string& roll, const string& courseCode);

// Get list of students with attendance < 75% in a course
// Returns: vector of {roll, courseCode, percentage_string}
vector<vector<string>> getShortageList(const string& courseCode);

// Undo last marked session (restores from in-memory backup)
// Returns false if no backup exists
bool undoLastSession();

// Print daily attendance sheet for a course on a given date
void printDailySheet(const string& courseCode, const string& date);

// Interactive menu for attendance operations
void attendanceMenu();

#endif
