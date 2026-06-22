#ifndef REPORTS_H
#define REPORTS_H

#include <string>
#include <vector>
using namespace std;

// Print merit list: all active students sorted by CGPA descending
void printMeritList();

// Print students with any course attendance < 75%
void printAttendanceDefaulters();

// Print fee defaulters with outstanding amount and weeks overdue
void printFeeDefaulters();

// Print full semester result sheet for a course (grade, GPA, attendance per student)
void printSemesterResult(const string& courseCode, const string& semester);

// Print department summary: count, avg CGPA, pass rate per department
void printDepartmentSummary();

// Export a report to a .txt file (redirects output)
void exportReportToFile(int reportChoice, const string& outputFile);

// Interactive menu for reports
void reportsMenu();

#endif
