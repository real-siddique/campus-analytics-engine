#ifndef STUDENT_OPS_H
#define STUDENT_OPS_H

#include <string>
#include <vector>
using namespace std;

// Validate roll number format: BSAI-YY-XXX
// YY = two digits, XXX = three digits
bool validateRollFormat(const string& roll);

// Add a new student (validates roll, name, cgpa; appends to students.txt)
bool addStudent(const string& roll, const string& name, const string& dept,
                const string& semester, double cgpa);

// Search student by exact roll number
// Returns student row or empty vector
vector<string> searchByRoll(const string& roll);

// Search students by name substring (case-insensitive)
// Returns all matching active+inactive rows
vector<vector<string>> searchByName(const string& namePart);

// Update a specific field of a student (cannot update roll)
// fieldIndex: 1=name, 2=dept, 3=semester, 4=cgpa, 5=status
bool updateStudent(const string& roll, int fieldIndex, const string& newValue);

// Soft-delete: sets status to 'inactive'
bool softDelete(const string& roll);

// Returns all active students sorted by roll number (selection sort)
vector<vector<string>> listActiveStudents();

// Interactive menu for student operations
void studentMenu();

#endif
