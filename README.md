# Campus Analytics Engine

A multi-file, menu-driven data analytics system for a fictional university campus.  
Built with **pure C++ (no OOP, no STL algorithms, no ctime)** — Programming Fundamentals project.

---

## Features

- **Student Management** — Add, search, update, soft-delete students. Roll format validated as `BSAI-YY-XXX`.
- **Course Management** — Enroll/drop students with prerequisite, credit-load (≤21 hrs), and seat checks.
- **Attendance** — Mark P/A/L per student, compute %, flag <75% shortage, undo last session.
- **Grades** — Enter quiz/assignment/mid/final marks, compute weighted totals, letter grades, semester GPA. Attendance penalty (F if <75%).
- **Fee Tracker** — Record payments, compute late fines (2%/week, manual date math), generate formatted receipts.
- **Reports** — Merit list, attendance defaulters, fee defaulters, semester result sheet, department summary, export to file.
- **Bonus** — Search-as-you-type: prefix match on student names, refines after each character (option 7 from main menu).

---

## File Structure

```
campus_engine/
├── main.cpp            # Entry point, 3-level nested menu
├── filehandler.h/.cpp  # All TXT read/write/append/search
├── student_ops.h/.cpp  # Student CRUD + selection sort
├── course_ops.h/.cpp   # Enroll/drop + prereq/credit checks
├── attendance.h/.cpp   # Mark attendance, %, undo, shortage
├── grades.h/.cpp       # Marks, GPA, letter grades, stats
├── fee_tracker.h/.cpp  # Payments, late fines, receipts
├── reports.h/.cpp      # All formatted report tables
├── Makefile
├── students.txt
├── courses.txt
├── enrollments.txt
├── attendance_log.txt
├── fees.txt
└── grades.txt          # Created empty on first run
```

---

## How to Compile

### Using Makefile (recommended)
```bash
make
./campus_engine
```

### Manual compilation
```bash
g++ -Wall -std=c++11 -o campus_engine \
    main.cpp filehandler.cpp student_ops.cpp course_ops.cpp \
    attendance.cpp grades.cpp fee_tracker.cpp reports.cpp
./campus_engine
```

> **Important:** All `.txt` data files must be in the **same directory** as the executable.

---

## Sample Run

```
  ============================================================
  |          CAMPUS ANALYTICS ENGINE                        |
  |          BS Artificial Intelligence                     |
  ============================================================

===== MAIN MENU =====
1. Student Management
2. Course Management
3. Attendance Management
4. Grades Management
5. Fee Management
6. Reports
0. Exit
Choice: 6

===== REPORTS =====
1. Merit List
...

======================================================================
                             MERIT LIST
======================================================================
Rank Roll          Name                    Dept        CGPA
----------------------------------------------------------------------
1    BSAI-23-006   Sana Pervez             Artificial Intelligence  3.90
2    BSAI-23-012   Maham Javed             Artificial Intelligence  3.80
...
```

---

## Data File Formats

| File | Columns |
|------|---------|
| `students.txt` | roll_no, name, department, semester, cgpa, status |
| `courses.txt` | course_code, course_name, credit_hours, instructor, capacity, enrolled, prerequisite |
| `enrollments.txt` | roll_no, course_code, semester, status |
| `attendance_log.txt` | roll_no, course_code, date, status (P/A/L) |
| `fees.txt` | fee_id, roll_no, semester, total_fee, amount_paid, due_date, payment_date, payment_method, status |
| `grades.txt` | roll_no, course_code, semester, quiz_avg, asgn_avg, mid, final, total, letter_grade |

---

## PF Concepts Used

- File I/O with `ifstream` / `ofstream`
- `struct` based data (Stats struct in grades)
- Nested loops for cross-file lookups
- Selection sort (students by roll, CGPA)
- Bubble sort (fee defaulters by balance)
- Manual string parsing (CSV split, date arithmetic)
- Enums (`EnrollResult`)
- Static variables (attendance backup for undo)
- `setw` / `setfill` from `<iomanip>` for formatted output
- Input validation (roll format, date format, CGPA range)

---

## Constraints Respected

- ❌ No `#include <algorithm>`
- ❌ No `#include <map>` / `<set>`
- ❌ No `class` keyword (only `struct`)
- ❌ No `#include <ctime>` (date arithmetic done manually)
- ❌ No hardcoded data (all loaded from `.txt` files)
- ✅ Multi-file structure (8 source files)
- ✅ Cross-file function calls
