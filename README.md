# Student Attendance System (C, CSV-based)

## Overview

This project is a **menu-driven student attendance management system** written in **C**, using **CSV files** for persistent storage.

It allows a teacher or lecturer to:
- Register students
- Mark daily attendance
- View attendance summaries

All interactions happen **via the terminal**, and data is stored externally so it persists between program runs.

---

## Features

- Add new students (name, matric number, department)
- Mark attendance by date (Present / Absent)
- View attendance summary
- Persistent storage using CSV files
- Simple, menu-based user interface

---

##  Technologies Used

- **Language:** C
- **Storage:** CSV files (`students.csv`, `attendance.csv`)
- **Compiler:** GCC / Clang
- **Platform:** Cross-platform (Linux, macOS, Windows)

---

## Project Structure

```
SEN PROJECT/
│
├── main.c
├── README.md
├── .gitignore
│
├── students.sample.csv
├── attendance.sample.csv
│
├── students.csv          (generated at runtime)
├── attendance.csv        (generated at runtime)
│
├── main                  (compiled binary)
├── main.dSYM/
└── tempCodeRunnerFile.c
```

---

## CSV File Formats

### students.csv
```csv
Name,Matric,Department
John Doe,2024/00000,Computer Science
```

### attendance.csv
```csv
Date,Name,Matric,Department,Status
2025-01-10,John Doe,2024/00000,Computer Science,P
```

---

## ▶ How to Compile and Run

### Compile
```bash
gcc main.c -o attendance
```

### Run
```bash
./attendance
```

---

## Menu Options

```
1. Add new student
2. Mark attendance
3. View attendance summary
4. Exit
```

---

## Git & Data Handling

- Runtime CSV files are ignored via `.gitignore`
- Sample CSV files are tracked for reference
- Prevents data leaks and merge conflicts

---

## Future Improvements

- Duplicate attendance prevention
- Attendance percentage calculation
- Lecturer authentication
- Proxy / proximity-based attendance
- Database migration (SQLite)

---

## Author

Ogundele Ademilade
Information Technology
Bells University of Technology


