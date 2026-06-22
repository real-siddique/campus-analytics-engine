#include "filehandler.h"
#include <fstream>
#include <iostream>

using namespace std;

// Split a CSV line into fields; handles quoted fields containing commas
vector<string> splitLine(const string& line) {
    vector<string> fields;
    string current = "";
    bool inQuotes = false;

    for (int i = 0; i < (int)line.length(); i++) {
        char c = line[i];
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
            fields.push_back(current);
            current = "";
        } else if (c == '\r') {
            // skip carriage return
        } else {
            current += c;
        }
    }
    fields.push_back(current);
    return fields;
}

// Join fields into CSV line; wraps fields containing commas in quotes
string joinRow(const vector<string>& fields) {
    string result = "";
    for (int i = 0; i < (int)fields.size(); i++) {
        if (i > 0) result += ",";
        bool hasComma = false;
        for (int j = 0; j < (int)fields[i].length(); j++) {
            if (fields[i][j] == ',') { hasComma = true; break; }
        }
        if (hasComma) {
            result += "\"" + fields[i] + "\"";
        } else {
            result += fields[i];
        }
    }
    return result;
}

// Read all rows from file, skip header row
vector<vector<string>> readTXT(const string& filename) {
    vector<vector<string>> rows;
    ifstream file(filename.c_str());
    if (!file.is_open()) {
        return rows;
    }
    string line;
    bool firstLine = true;
    while (true) {
        // Read line character by character (no getline split on delimiter)
        line = "";
        char c;
        bool got = false;
        while (file.get(c)) {
            got = true;
            if (c == '\n') break;
            line += c;
        }
        if (!got) break;

        if (firstLine) { firstLine = false; continue; } // skip header
        if (line.empty() || line == "\r") continue;

        vector<string> row = splitLine(line);
        rows.push_back(row);
    }
    file.close();
    return rows;
}

// Overwrite file: write header then all rows
void writeTXT(const string& filename, const string& header, const vector<vector<string>>& rows) {
    ofstream file(filename.c_str());
    if (!file.is_open()) {
        cout << "Error: Cannot open file " << filename << " for writing.\n";
        return;
    }
    file << header << "\n";
    for (int i = 0; i < (int)rows.size(); i++) {
        file << joinRow(rows[i]) << "\n";
    }
    file.close();
}

// Append a single row to existing file
void appendTXT(const string& filename, const vector<string>& row) {
    ofstream file(filename.c_str(), ios::app);
    if (!file.is_open()) {
        cout << "Error: Cannot open file " << filename << " for appending.\n";
        return;
    }
    file << joinRow(row) << "\n";
    file.close();
}

// Find first row where row[colIndex] == value
vector<string> findRow(const string& filename, int colIndex, const string& value) {
    vector<vector<string>> rows = readTXT(filename);
    for (int i = 0; i < (int)rows.size(); i++) {
        if (colIndex < (int)rows[i].size() && rows[i][colIndex] == value) {
            return rows[i];
        }
    }
    return vector<string>();
}

// Check if any row has row[colIndex] == value
bool rowExists(const string& filename, int colIndex, const string& value) {
    vector<string> row = findRow(filename, colIndex, value);
    return !row.empty();
}
