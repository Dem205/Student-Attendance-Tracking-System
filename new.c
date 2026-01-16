#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_SESSIONS 10

// Define struct globally
struct StudentScore {
  char matric[20];
  int score;
};

void clearInputBuffer() {
  int c;
  while ((c = getchar()) != '\n' && c != EOF)
    ;
}

void ensureStudentsFileExists() {
  FILE *fp = fopen("students.csv", "r");
  if (fp == NULL) {
    fp = fopen("students.csv", "w");
    if (fp != NULL) {
      fprintf(fp, "Name,Matric,Department\n");
      fclose(fp);
    }
  } else {
    fclose(fp);
  }
}

// Check if string ends with suffix
int endsWith(const char *str, const char *suffix) {
  if (!str || !suffix)
    return 0;
  size_t lenstr = strlen(str);
  size_t lensuffix = strlen(suffix);
  if (lensuffix > lenstr)
    return 0;
  return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

// Check if string starts with prefix
int startsWith(const char *str, const char *prefix) {
  if (!str || !prefix)
    return 0;
  size_t lenstr = strlen(str);
  size_t lenprefix = strlen(prefix);
  if (lenprefix > lenstr)
    return 0;
  return strncmp(str, prefix, lenprefix) == 0;
}

// Helper function to get next CSV token
char* getNextToken(char **str, const char *delim) {
  if (*str == NULL) return NULL;
  
  char *start = *str;
  char *end = strstr(start, delim);
  
  if (end) {
    *end = '\0';
    *str = end + strlen(delim);
  } else {
    *str = NULL;
  }
  
  return start;
}

// Helper to list valid attendance files for a specific course
void listAttendanceFiles(char files[100][100], int *fileCount, const char *courseCode) {
  DIR *d;
  struct dirent *dir;
  *fileCount = 0;

  d = opendir(".");
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      int match = 1;
      
      // Filter by course code if provided
      if (courseCode && strlen(courseCode) > 0) {
        char prefix[100];
        sprintf(prefix, "%s-", courseCode);
        if (!startsWith(dir->d_name, prefix)) {
          match = 0;
        }
      }

      // Basic validity checks
      if (match) {
        if (!endsWith(dir->d_name, ".csv")) match = 0;
        else if (strlen(dir->d_name) < 24) match = 0; // Min length for code-YYYY-MM-DD_HH-MM-SS.csv
        else if (strcmp(dir->d_name, "students.csv") == 0) match = 0;
      }

      if (match) {
        strcpy(files[*fileCount], dir->d_name);
        (*fileCount)++;
        if (*fileCount >= 100)
          break;
      }
    }
    closedir(d);
  }

  // Sort files by Date Descending (Latest first)[I believe we discussed this ]
  // Compare the timestamp part: YYYY-MM-DD_HH-MM-SS.csv (Last 23 chars)
  for (int i = 0; i < *fileCount - 1; i++) {
    for (int j = 0; j < *fileCount - i - 1; j++) {
      char *f1 = files[j];
      char *f2 = files[j + 1];
      
      // Safety check for lengths
      if (strlen(f1) < 24 || strlen(f2) < 24) continue;

      char *s1 = f1 + strlen(f1) - 23; 
      char *s2 = f2 + strlen(f2) - 23;
      
      // strcmp(s1, s2) returns < 0 if s1 is "older/smaller" than s2.
      // We want Descending (Newest first), so if s1 < s2, we need to swap s1 to be after s2.
      if (strcmp(s1, s2) < 0) {
        char temp[100];
        strcpy(temp, files[j]);
        strcpy(files[j], files[j + 1]);
        strcpy(files[j + 1], temp);
      }
    }
  }
}

void extractDateTimeFromFilename(char *filename, char *date, char *time_str) {
  // filename format expected: COURSECODE-YYYY-MM-DD_HH-MM-SS.csv
  // Date time part is always at the end: YYYY-MM-DD_HH-MM-SS.csv (23 chars)
  size_t len = strlen(filename);
  if (len < 24) { // Min length check
    strcpy(date, "Unknown");
    strcpy(time_str, "Unknown");
    return;
  }

  // Extract Date: YYYY-MM-DD (10 chars)
  // Position found by going back 23 chars from .csv end
  // Filename is ".....YYYY-MM-DD_HH-MM-SS.csv"
  // .csv is 4 chars. Time is 8 chars. _ is 1. Date is 10.
  // Total suffix length = 4+8+1+10 = 23 chars.
  // Start index = len - 23
  
  char *start = filename + len - 23;

  strncpy(date, start, 10);
  date[10] = '\0';

  // Extract Time: HH-MM-SS (8 chars)
  // Start index = len - 12
  strncpy(time_str, start + 11, 8);
  time_str[8] = '\0';

  // Replace '-' in time with ':' for display
  for (int i = 0; i < 8; i++) {
    if (time_str[i] == '-')
      time_str[i] = ':';
  }
}

// Calculate scores by reading ALL attendance files
void getAttendanceStats(int *unique_sessions,
                        struct StudentScore *student_scores,
                        int *student_count, const char *courseCode) {

  char files[100][100];
  int fileCount = 0;
  listAttendanceFiles(files, &fileCount, courseCode);
  *unique_sessions = fileCount;
  *student_count = 0;

  for (int f = 0; f < fileCount; f++) {
    FILE *fp = fopen(files[f], "r");
    if (!fp)
      continue;

    char line[256];
    // Skip Header
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp)) {
      char matric[20], status_char;
      // Format expected: Name,Matric,Dept,Status (Char P/A)

      char lineCopy[256];
      strcpy(lineCopy, line);
      char *ptr = lineCopy;
      char *token;

      // Name (skip)
      token = getNextToken(&ptr, ",");
      if (!token)
        continue;

      // Matric
      token = getNextToken(&ptr, ",");
      if (!token)
        continue;
      strcpy(matric, token);

      // Dept (skip)
      token = getNextToken(&ptr, ",");
      if (!token)
        continue;

      // Status
      token = getNextToken(&ptr, ",");
      if (!token)
        continue;
      status_char = token[0];

      // Update Score
      int found_idx = -1;
      for (int i = 0; i < *student_count; i++) {
        if (strcmp(student_scores[i].matric, matric) == 0) {
          found_idx = i;
          break;
        }
      }
      if (found_idx == -1) {
        found_idx = *student_count;
        strcpy(student_scores[found_idx].matric, matric);
        student_scores[found_idx].score = 0;
        (*student_count)++;
      }

      if (toupper(status_char) == 'P') {
        if (student_scores[found_idx].score < 100) {
          student_scores[found_idx].score += 10;
        }
      }
    }
    fclose(fp);
  }
}

void markAttendance() {
  char courseCode[50];
  printf("\nEnter Course Code: ");
  if (fgets(courseCode, sizeof(courseCode), stdin) == NULL) return;
  courseCode[strcspn(courseCode, "\n")] = 0;
  if (strlen(courseCode) == 0) return;

  // 1. Check Limits
  int unique_sessions = 0;
  struct StudentScore student_scores[100];
  int student_count_stats = 0;

  getAttendanceStats(&unique_sessions, student_scores, &student_count_stats, courseCode);

  if (unique_sessions >= MAX_SESSIONS) {
    printf("\n[ERROR] Maximum number of sessions (%d) reached.\n",
           MAX_SESSIONS);
    printf("Marking new attendance is prohibited.\n");
    return;
  }

  int current_session_num = unique_sessions + 1;

  // Make filename
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  char filename[100];
  // Format: COURSECODE-YYYY-MM-DD_HH-MM-SS.csv
  sprintf(filename, "%s-%04d-%02d-%02d_%02d-%02d-%02d.csv", courseCode,
          tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min,
          tm.tm_sec);

  printf("\n=== MARK ATTENDANCE (Session #%d) ===\n", current_session_num);
  printf("Creating file: %s\n", filename);

  if (current_session_num == MAX_SESSIONS)
    printf(
        "NOTE: This is the FINAL session. Eligibility will be calculated.\n");

  FILE *fpStudents = fopen("students.csv", "r");
  if (fpStudents == NULL) {
    printf("\nNo students found. Please add students first.\n");
    return;
  }

  FILE *fpAttendance = fopen(filename, "w");
  if (fpAttendance == NULL) {
    printf("Error creating attendance file.\n");
    fclose(fpStudents);
    return;
  }

  // Header matching user's view expectation
  fprintf(fpAttendance, "Name,Matric,Department,Status\n");

  char buffer[200];
  fgets(buffer, sizeof(buffer), fpStudents); // Skip student header

  char name[50], department[30], matric_num[20];
  int count_students_pcessed = 0;

  while (fscanf(fpStudents, "%[^,],%[^,],%[^\n]\n", name, matric_num,
                department) != EOF) {
    count_students_pcessed++;
    int current_score = 0;
    for (int i = 0; i < student_count_stats; i++) {
      if (strcmp(student_scores[i].matric, matric_num) == 0) {
        current_score = student_scores[i].score;
        break;
      }
    }

    char att;
    int valid = 0;
    do {
      printf("%d. %-30s (Current Score: %d%%): ", count_students_pcessed, name,
             current_score);

      char input_buf[10];
      if (fgets(input_buf, sizeof(input_buf), stdin) == NULL) {
        att = 'A';
      } else {
        input_buf[strcspn(input_buf, "\n")] = 0;
        if (strlen(input_buf) == 1) {
          att = toupper(input_buf[0]);
          if (att == 'P' || att == 'A') {
            valid = 1;
          }
        }
      }

      if (!valid) {
        printf("   [Invalid] Enter 'P' for Present or 'A' for Absent only.\n");
      }
    } while (!valid);

    // Write to file
    fprintf(fpAttendance, "%s,%s,%s,%c\n", name, matric_num, department, att);
  }

  fclose(fpStudents);
  fclose(fpAttendance);

  printf("\nAttendance recorded successfully!\n");

  // Implicitly calculate eligibility for display if this was session 10
  if (current_session_num == MAX_SESSIONS) {
    printf("\n=== FINAL ELIGIBILITY REPORT ===\n");
    printf("%-30s %-15s %-10s %s\n", "Name", "Matric", "Score", "Status");
    printf("-------------------------------------------------------------\n");

    // Need to re-calculate including just-added session
    // Quickest way: just iterate the students we just processed or re-run stats
    // Let's re-run stats for accuracy
    getAttendanceStats(&unique_sessions, student_scores, &student_count_stats, courseCode);

    FILE *fpStudentsRe = fopen("students.csv", "r");
    fgets(buffer, sizeof(buffer), fpStudentsRe);
    while (fscanf(fpStudentsRe, "%[^,],%[^,],%[^\n]\n", name, matric_num,
                  department) != EOF) {
      int s = 0;
      for (int k = 0; k < student_count_stats; k++) {
        if (strcmp(student_scores[k].matric, matric_num) == 0)
          s = student_scores[k].score;
      }
      const char *status = (s >= 70) ? "ELIGIBLE" : "INELIGIBLE";
      printf("%-30s %-15s %-10d %s\n", name, matric_num, s, status);
    }
    fclose(fpStudentsRe);
  }
}

void viewAttendanceSummary() {
  /*
  char courseCode[50];
  printf("\nEnter Course Code: ");
  if (fgets(courseCode, sizeof(courseCode), stdin) == NULL) return;
  courseCode[strcspn(courseCode, "\n")] = 0;
  */

  char files[100][100];
  int fileCount = 0;
  int choice;

  // Pass NULL to list ALL attendance files
  listAttendanceFiles(files, &fileCount, NULL);

  if (fileCount == 0) {
    printf("\nNo attendance records found.\n");
    return;
  }

  printf("\n=== AVAILABLE ATTENDANCE RECORDS ===\n");
  for (int i = 0; i < fileCount; i++) {
    char date[11], time_str[9];
    extractDateTimeFromFilename(files[i], date, time_str);
    
    // Extract Course Code
    // Filename: CODE-YYYY...
    // We know the suffixes are 24 chars (including the dash before date? No, suffix is 23: YYYY...)
    // Suffix start index: len - 23.
    // Separator index: len - 24.
    char courseCode[50] = "Unknown";
    size_t len = strlen(files[i]);
    if (len >= 24) {
      size_t codeLen = len - 24;
      if (codeLen > 49) codeLen = 49;
      strncpy(courseCode, files[i], codeLen);
      courseCode[codeLen] = '\0';
    }

    printf("%d. [%s] %s at %s\n", i + 1, courseCode, date, time_str);
  }

  // Safety check
  int valid_choice = 0;
  do {
    printf("\nSelect attendance record (1-%d): ", fileCount);
    if (scanf("%d", &choice) == 1) {
      if (choice >= 1 && choice <= fileCount)
        valid_choice = 1;
    }
    clearInputBuffer();
    if (!valid_choice)
      printf("Invalid selection!\n");
  } while (!valid_choice);

  FILE *fp;
  char name[50], matric_num[20], department[30];
  char att;
  char buffer[200];
  char date[11], time_str[9];

  fp = fopen(files[choice - 1], "r");
  if (fp == NULL) {
    printf("\nError opening file.\n");
    return;
  }

  extractDateTimeFromFilename(files[choice - 1], date, time_str);

  fgets(buffer, sizeof(buffer), fp); // Skip Header

  printf("\n=== ATTENDANCE SUMMARY ===\n");
  printf("Date: %s | Time: %s\n\n", date, time_str);
  printf("%-25s %-12s %-18s %-10s\n", "Name", "Matric", "Department", "Status");
  printf("-----------------------------------------------------------------------\n");

  while (fgets(buffer, sizeof(buffer), fp)) {
    char lineCopy[256];
    strcpy(lineCopy, buffer);
    char *ptr = lineCopy;
    char *token;

    // Name
    token = getNextToken(&ptr, ",");
    if (token)
      strcpy(name, token);
    else
      continue;

    // Matric
    token = getNextToken(&ptr, ",");
    if (token)
      strcpy(matric_num, token);
    else
      continue;

    // Dept
    token = getNextToken(&ptr, ",");
    if (token)
      strcpy(department, token);
    else
      continue;

    // Status (often at end, might have newline)
    if (ptr)
      att = ptr[0];
    else
      att = '?';

    char status[10];
    if (toupper(att) == 'P') {
      strcpy(status, "Present");
    } else {
      strcpy(status, "Absent");
    }
    printf("%-25s %-12s %-18s %-10s\n", name, matric_num, department, status);
  }
  fclose(fp);
}

void addStudent() {
  FILE *fp;
  char name[50];
  char department[30];
  char matric_num[20];
  char choice;

  ensureStudentsFileExists();

  fp = fopen("students.csv", "a");
  if (fp == NULL) {
    printf("Error opening file.\n");
    return;
  }

  do {
    printf("\n--- Add New Student ---\n");

    printf("Enter student name: ");
    if (fgets(name, sizeof(name), stdin) == NULL)
      break;
    name[strcspn(name, "\n")] = 0;

    printf("Enter student matric number: ");
    if (fgets(matric_num, sizeof(matric_num), stdin) == NULL)
      break;
    matric_num[strcspn(matric_num, "\n")] = 0;

    printf("Enter student department: ");
    if (fgets(department, sizeof(department), stdin) == NULL)
      break;
    department[strcspn(department, "\n")] = 0;

    fprintf(fp, "%s,%s,%s\n", name, matric_num, department);
    printf("\nStudent data saved successfully!\n");

    printf("\nDo you want to add another student? (y/n): ");
    scanf(" %c", &choice);
    clearInputBuffer();

  } while (choice == 'y' || choice == 'Y');

  fclose(fp);
}

void viewAllStudents() {
  FILE *fp;
  char name[50], department[30], matric_num[20];
  char buffer[200];

  fp = fopen("students.csv", "r");
  if (fp == NULL) {
    printf("\nNo students found in the system.\n");
    return;
  }

  fgets(buffer, sizeof(buffer), fp);

  printf("\n=== ALL REGISTERED STUDENTS ===\n");
  printf("%-25s %-12s %-18s\n", "Name", "Matric", "Department");
  printf("-----------------------------------------------------------\n");

  int count = 0;
  while (fscanf(fp, "%[^,],%[^,],%[^\n]\n", name, matric_num, department) !=
         EOF) {
    printf("%-25s %-12s %-18s\n", name, matric_num, department);
    count++;
  }

  printf("-----------------------------------------------------------\n");
  if (count == 0) {
    printf("No students registered yet.\n");
  } else {
    printf("Total students: %d\n", count);
  }

  fclose(fp);
}

int main() {
  // Ensure we have directories/files needed
  ensureStudentsFileExists();
  int choice;

  printf("\n=== STUDENT ATTENDANCE SYSTEM ===\n");

  do {
    printf("\n=== MAIN MENU ===\n");
    printf("1. Add new student\n");
    printf("2. Mark attendance\n");
    printf("3. View attendance log\n");
    printf("4. View all students\n");
    printf("5. Exit\n");
    printf("Enter your choice: ");
    if (scanf("%d", &choice) != 1) {
      clearInputBuffer();
      choice = 0;
    } else {
      clearInputBuffer();
    }

    switch (choice) {
    case 1:
      addStudent();
      break;
    case 2:
      markAttendance();
      break;
    case 3:
      viewAttendanceSummary();
      break;
    case 4:
      viewAllStudents();
      break;
    case 5:
      printf("\nThank you for using the system!\n");
      break;
    default:
      printf("\nInvalid choice! Please select 1-5.\n");
    }
  } while (choice != 5);

  return 0;
}
