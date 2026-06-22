#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <string>
#include <vector>

using namespace std;

// Read all rows from a TXT file (skips header row)
// Returns vector of rows, each row is a vector of fields
vector<vector<string>> readTXT(const string& filename);

// Overwrite the file with header + all rows
void writeTXT(const string& filename, const string& header, const vector<vector<string>>& rows);

// Append a single row to the file without loading it fully
void appendTXT(const string& filename, const vector<string>& row);

// Linear search: find first row where row[colIndex] == value
// Returns matching row, or empty vector if not found
vector<string> findRow(const string& filename, int colIndex, const string& value);

// Returns true if any row has row[colIndex] == value
bool rowExists(const string& filename, int colIndex, const string& value);

// Join fields into a comma-separated line (wraps fields with commas in quotes)
string joinRow(const vector<string>& fields);

// Split a CSV line into fields (handles quoted fields)
vector<string> splitLine(const string& line);

#endif
