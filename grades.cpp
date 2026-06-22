#include "grades.h"
#include "filehandler.h"
#include "student_ops.h"
#include "attendance.h"
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cstdlib>

using namespace std;

#define GRADES_FILE    "grades.txt"
#define COURSES_FILE   "courses.txt"
#define ENROLLMENTS_FILE "enrollments.txt"
#define GRADES_HEADER  "roll_no,course_code,semester,quiz_avg,asgn_avg,mid,final,total,letter_grade"

double bestThreeOfFive(double quizzes[], int n) {
    if (n <= 0) return 0.0;
    if (n == 1) return quizzes[0];
    if (n == 2) return (quizzes[0] + quizzes[1]) / 2.0;

    // For n >= 3: exclude 2 lowest using accumulator loop (no sort)
    int use = (n < 5) ? n : 5;
    int idx1 = 0; // index of lowest
    for (int i = 1; i < use; i++)
        if (quizzes[i] < quizzes[idx1]) idx1 = i;

    int idx2 = (idx1 == 0) ? 1 : 0; // index of second lowest
    for (int i = 0; i < use; i++) {
        if (i != idx1 && quizzes[i] < quizzes[idx2]) idx2 = i;
    }

    // If n < 5, only remove lowest 2 if n == 5
    // For n = 3 or 4, use all (project says "up to 5", best 3 of the ones given)
    int removeCount = (use == 5) ? 2 : (use == 4 ? 1 : 0);
    double sum = 0;
    int count = 0;

    if (removeCount == 2) {
        for (int i = 0; i < use; i++)
            if (i != idx1 && i != idx2) { sum += quizzes[i]; count++; }
    } else if (removeCount == 1) {
        for (int i = 0; i < use; i++)
            if (i != idx1) { sum += quizzes[i]; count++; }
    } else {
        for (int i = 0; i < use; i++) { sum += quizzes[i]; count++; }
    }
    return count > 0 ? sum / count : 0.0;
}

double computeWeightedTotal(double quizAvg, double asgnAvg, double mid, double finalMark) {
    return quizAvg * 0.10 + asgnAvg * 0.10 + mid * 0.30 + finalMark * 0.50;
}

string getLetterGrade(double total) {
    if (total >= 85) return "A";
    if (total >= 80) return "B+";
    if (total >= 70) return "B";
    if (total >= 65) return "C+";
    if (total >= 60) return "C";
    if (total >= 50) return "D";
    return "F";
}

bool enterMarks(const string& roll, const string& courseCode, const string& semester) {
    // Validate student and enrollment
    vector<string> student = searchByRoll(roll);
    if (student.empty()) {
        cout << "Error: Student not found.\n"; return false;
    }

    // Check if grades already exist
    vector<vector<string>> existing = readTXT(GRADES_FILE);
    for (int i = 0; i < (int)existing.size(); i++) {
        if (existing[i].size() > 2 &&
            existing[i][0] == roll &&
            existing[i][1] == courseCode &&
            existing[i][2] == semester) {
            cout << "Error: Marks already entered for " << roll << " in " << courseCode << ".\n";
            return false;
        }
    }

    cout << "\n--- Entering Marks for " << roll << " | Course: " << courseCode << " ---\n";

    // Quizzes (up to 5)
    int numQuizzes;
    cout << "Number of quizzes (1-5): ";
    cin >> numQuizzes;
    if (numQuizzes < 1) numQuizzes = 1;
    if (numQuizzes > 5) numQuizzes = 5;

    double quizzes[5] = {0};
    for (int i = 0; i < numQuizzes; i++) {
        do {
            cout << "Quiz " << (i+1) << " (0-10): ";
            cin >> quizzes[i];
        } while (quizzes[i] < 0 || quizzes[i] > 10);
    }

    // Assignments (up to 5)
    int numAsgn;
    cout << "Number of assignments (1-5): ";
    cin >> numAsgn;
    if (numAsgn < 1) numAsgn = 1;
    if (numAsgn > 5) numAsgn = 5;

    double asgns[5] = {0};
    double asgnSum = 0;
    for (int i = 0; i < numAsgn; i++) {
        do {
            cout << "Assignment " << (i+1) << " (0-10): ";
            cin >> asgns[i];
        } while (asgns[i] < 0 || asgns[i] > 10);
        asgnSum += asgns[i];
    }

    double mid, finalMark;
    do {
        cout << "Mid exam (0-40): ";
        cin >> mid;
    } while (mid < 0 || mid > 40);

    do {
        cout << "Final exam (0-60): ";
        cin >> finalMark;
    } while (finalMark < 0 || finalMark > 60);

    // Compute averages (scale quizzes/assignments to 10)
    double quizAvg = bestThreeOfFive(quizzes, numQuizzes);
    double asgnAvg = numAsgn > 0 ? asgnSum / numAsgn : 0;

    // Scale mid and final to % of their weights
    double midPct = (mid / 40.0) * 100.0;
    double finalPct = (finalMark / 60.0) * 100.0;
    double quizPct = (quizAvg / 10.0) * 100.0;
    double asgnPct = (asgnAvg / 10.0) * 100.0;

    double total = computeWeightedTotal(quizPct, asgnPct, midPct, finalPct);
    string grade = getLetterGrade(total);

    char qBuf[16], aBuf[16], mBuf[16], fBuf[16], tBuf[16];
    sprintf(qBuf, "%.2f", quizPct);
    sprintf(aBuf, "%.2f", asgnPct);
    sprintf(mBuf, "%.2f", midPct);
    sprintf(fBuf, "%.2f", finalPct);
    sprintf(tBuf, "%.2f", total);

    vector<string> row;
    row.push_back(roll);
    row.push_back(courseCode);
    row.push_back(semester);
    row.push_back(string(qBuf));
    row.push_back(string(aBuf));
    row.push_back(string(mBuf));
    row.push_back(string(fBuf));
    row.push_back(string(tBuf));
    row.push_back(grade);
    appendTXT(GRADES_FILE, row);

    cout << "\nMarks saved. Total: " << tBuf << "% | Grade: " << grade << "\n";

    // Apply attendance penalty
    applyAttendancePenalty(roll, courseCode);
    return true;
}

void applyAttendancePenalty(const string& roll, const string& courseCode) {
    double pct = getAttendancePct(roll, courseCode);
    if (pct < 75.0) {
        // Override grade to F in grades.txt
        vector<vector<string>> rows = readTXT(GRADES_FILE);
        for (int i = 0; i < (int)rows.size(); i++) {
            if (rows[i].size() > 8 &&
                rows[i][0] == roll &&
                rows[i][1] == courseCode) {
                rows[i][8] = "F";
                cout << "WARNING: " << roll << " failed " << courseCode
                     << " due to attendance < 75% (" << pct << "%). Grade set to F.\n";
            }
        }
        writeTXT(GRADES_FILE, GRADES_HEADER, rows);
    }
}

// GPA point mapping
static double gradeToGPA(const string& g) {
    if (g == "A")  return 4.0;
    if (g == "B+") return 3.5;
    if (g == "B")  return 3.0;
    if (g == "C+") return 2.5;
    if (g == "C")  return 2.0;
    if (g == "D")  return 1.0;
    return 0.0; // F
}

double computeGPA(const string& roll, const string& semester) {
    vector<vector<string>> grades = readTXT(GRADES_FILE);
    vector<vector<string>> courses = readTXT(COURSES_FILE);
    vector<vector<string>> enrollments = readTXT(ENROLLMENTS_FILE);

    double totalWeighted = 0;
    int totalCredits = 0;

    for (int i = 0; i < (int)grades.size(); i++) {
        if (grades[i].size() > 8 &&
            grades[i][0] == roll &&
            grades[i][2] == semester) {

            string code = grades[i][1];
            string letterGrade = grades[i][8];

            // Find credits
            int credits = 3; // default
            for (int j = 0; j < (int)courses.size(); j++) {
                if (courses[j].size() > 2 && courses[j][0] == code) {
                    credits = atoi(courses[j][2].c_str());
                    break;
                }
            }
            totalWeighted += gradeToGPA(letterGrade) * credits;
            totalCredits += credits;
        }
    }
    if (totalCredits == 0) return 0.0;
    return totalWeighted / totalCredits;
}

Stats computeClassStats(const string& courseCode) {
    vector<vector<string>> grades = readTXT(GRADES_FILE);
    vector<double> totals;

    for (int i = 0; i < (int)grades.size(); i++) {
        if (grades[i].size() > 7 && grades[i][1] == courseCode) {
            totals.push_back(atof(grades[i][7].c_str()));
        }
    }

    Stats s = {0, 0, 0, 0};
    if (totals.empty()) return s;

    s.highest = totals[0];
    s.lowest  = totals[0];
    double sum = 0;

    for (int i = 0; i < (int)totals.size(); i++) {
        if (totals[i] > s.highest) s.highest = totals[i];
        if (totals[i] < s.lowest)  s.lowest  = totals[i];
        sum += totals[i];
    }
    s.mean = sum / totals.size();

    // Compute median via selection sort on copy
    vector<double> sorted = totals;
    for (int i = 0; i < (int)sorted.size(); i++) {
        int minIdx = i;
        for (int j = i+1; j < (int)sorted.size(); j++)
            if (sorted[j] < sorted[minIdx]) minIdx = j;
        double tmp = sorted[i]; sorted[i] = sorted[minIdx]; sorted[minIdx] = tmp;
    }
    int n = sorted.size();
    s.median = (n % 2 == 0) ? (sorted[n/2-1] + sorted[n/2]) / 2.0 : sorted[n/2];

    return s;
}

void gradesMenu() {
    int choice;
    do {
        cout << "\n===== GRADES MANAGEMENT =====\n";
        cout << "1. Enter Marks\n";
        cout << "2. View Student Grade\n";
        cout << "3. Compute Semester GPA\n";
        cout << "4. Class Statistics\n";
        cout << "5. Apply Attendance Penalty\n";
        cout << "0. Back\n";
        cout << "Choice: ";
        cin >> choice;
        cin.ignore();

        if (choice == 1) {
            string roll, code, sem;
            cout << "Roll: "; getline(cin, roll);
            cout << "Course code: "; getline(cin, code);
            cout << "Semester: "; getline(cin, sem);
            enterMarks(roll, code, sem);

        } else if (choice == 2) {
            string roll, code;
            cout << "Roll: "; getline(cin, roll);
            cout << "Course code: "; getline(cin, code);
            vector<string> row = findRow(GRADES_FILE, 0, roll);
            // Search specifically for roll+course combo
            vector<vector<string>> all = readTXT(GRADES_FILE);
            bool found = false;
            for (int i = 0; i < (int)all.size(); i++) {
                if (all[i].size() > 8 && all[i][0] == roll && all[i][1] == code) {
                    cout << "\nGrades for " << roll << " in " << code << ":\n";
                    cout << "Quiz Avg:  " << all[i][3] << "%\n";
                    cout << "Asgn Avg:  " << all[i][4] << "%\n";
                    cout << "Mid:       " << all[i][5] << "%\n";
                    cout << "Final:     " << all[i][6] << "%\n";
                    cout << "Total:     " << all[i][7] << "%\n";
                    cout << "Grade:     " << all[i][8] << "\n";
                    found = true; break;
                }
            }
            if (!found) cout << "No grades found.\n";

        } else if (choice == 3) {
            string roll, sem;
            cout << "Roll: "; getline(cin, roll);
            cout << "Semester: "; getline(cin, sem);
            double gpa = computeGPA(roll, sem);
            cout << "Semester GPA for " << roll << ": " << fixed << setprecision(2) << gpa << "\n";

        } else if (choice == 4) {
            string code;
            cout << "Course code: "; getline(cin, code);
            Stats st = computeClassStats(code);
            cout << "\nClass Statistics for " << code << ":\n";
            cout << "Highest: " << st.highest << "%\n";
            cout << "Lowest:  " << st.lowest  << "%\n";
            cout << "Mean:    " << st.mean    << "%\n";
            cout << "Median:  " << st.median  << "%\n";

        } else if (choice == 5) {
            string roll, code;
            cout << "Roll: "; getline(cin, roll);
            cout << "Course code: "; getline(cin, code);
            applyAttendancePenalty(roll, code);
        }
    } while (choice != 0);
}
