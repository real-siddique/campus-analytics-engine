#ifndef GRADES_H
#define GRADES_H

#include <string>
#include <vector>
using namespace std;

// Helper struct to hold class statistics
struct Stats {
    double highest;
    double lowest;
    double mean;
    double median;
};

// Enter marks for a student in a course
// Stores quiz array (up to 5), assignment array, mid, final in grades.txt
bool enterMarks(const string& roll, const string& courseCode, const string& semester);

// Best 3 of 5 quizzes: excludes 2 lowest, returns average of remaining
// Handles n < 3 edge case
double bestThreeOfFive(double quizzes[], int n);

// Compute weighted total:
// quiz*0.10 + asgn*0.10 + mid*0.30 + final*0.50
double computeWeightedTotal(double quizAvg, double asgnAvg, double mid, double finalMark);

// Map numeric total to letter grade
// >=85 A, >=80 B+, >=70 B, >=65 C+, >=60 C, >=50 D, else F
string getLetterGrade(double total);

// Compute semester GPA (credit-weighted) for a student
double computeGPA(const string& roll, const string& semester);

// Compute class statistics for a course
Stats computeClassStats(const string& courseCode);

// If attendance < 75%, override grade to F
void applyAttendancePenalty(const string& roll, const string& courseCode);

// Interactive menu for grades operations
void gradesMenu();

#endif
