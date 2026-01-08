#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Define struct globally to avoid type mismatch errors
struct StudentScore {
  char matric[20];
  int score;
};

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

void ensureAttendanceFileExists() {
  FILE *fp = fopen("attendance.csv", "r");
  char line[256];
  int hasHeader = 0;

  if (fp != NULL) {
    if (fgets(line, sizeof(line), fp)) {
      // Simple check if it's the new header
      if (strstr(line, "AttendanceScore") != NULL) {
        hasHeader = 1;
      }
    }
    fclose(fp);
  }

  if (!hasHeader) {
    // If file doesn't exist or has old header, recreate/overwrite with new
    // header Note: In a real app we might migrate data, but for this task we
    // start fresh or append new format (which might be messy, so we act like
    // it's a fresh start for the structure) However, to be safe, we open in 'a'
    // mode but if it was empty/missing verify header.

    fp = fopen("attendance.csv", "r");
    if (fp == NULL) {
      fp = fopen("attendance.csv", "w");
      if (fp) {
        fprintf(
            fp,
            "Date,Name,Matric,Department,Status,AttendanceScore,Eligibility\n");
        fclose(fp);
      }
    } else {
      // File exists but header is wrong or verified 'r' ok.
      // If we strictly follow "create new column", we assume we can just append
      // for now or the user handles the file reset. We'll ensure it exists.
      fclose(fp);
    }
  }
}

void clearInputBuffer() {
  int c;
  while ((c = getchar()) != '\n' && c != EOF)
    ;
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

// Helper to get current stats
void getAttendanceStats(const char *check_date, int *unique_days,
                        struct StudentScore *student_scores,
                        int *student_count) {
  FILE *fp = fopen("attendance.csv", "r");
  if (!fp)
    return;

  char line[1024];
  char date[20], name[50], matric[20], dept[30], status[10], junk[50];
  char dates_seen[100][20]; // Store unique dates visible
  int date_count = 0;

  *student_count = 0;

  // Skip header
  fgets(line, sizeof(line), fp);

  while (fgets(line, sizeof(line), fp)) {
    // Parse basic csv
    // Format: Date,Name,Matric,Department,Status,AttendanceScore,Eligibility
    // We only care about Date, Matric, Status to recalc scores

    char *token;
    char *rest = line;

    // Date
    token = strsep(&rest, ",");
    if (!token)
      continue;
    strcpy(date, token);
    // Name
    token = strsep(&rest, ",");
    if (!token)
      continue; // strcpy(name, token);
    // Matric
    token = strsep(&rest, ",");
    if (!token)
      continue;
    strcpy(matric, token);
    // Dept
    token = strsep(&rest, ",");
    if (!token)
      continue;
    // Status
    token = strsep(&rest, ",");
    if (!token)
      continue;
    strcpy(status, token);

    // Track unique dates
    int date_found = 0;
    for (int i = 0; i < date_count; i++) {
      if (strcmp(dates_seen[i], date) == 0) {
        date_found = 1;
        break;
      }
    }
    if (!date_found) {
      strcpy(dates_seen[date_count++], date);
    }

    // Update student score
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

    if (strchr(status, 'P') || strchr(status, 'p')) {
      student_scores[found_idx].score += 10; // Accumulate 10%
    }
  }

  *unique_days = date_count;

  // Check if check_date is already in dates_seen, if not, we are adding a NEW
  // day
  int date_exists = 0;
  for (int i = 0; i < date_count; i++) {
    if (strcmp(dates_seen[i], check_date) == 0)
      date_exists = 1;
  }
  if (!date_exists) {
    (*unique_days)++;
  }

  fclose(fp);
}

void markAttendance() {
  FILE *fpStudents, *fpAttendance;
  char name[50], department[30], matric_num[20];
  char att;
  char date[11];
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  sprintf(date, "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

  ensureAttendanceFileExists();

  // 1. Gather current stats
  int unique_days = 0;
  struct StudentScore student_scores[100];
  int student_count_stats = 0;

  getAttendanceStats(date, &unique_days, student_scores, &student_count_stats);

  printf("\n=== MARK ATTENDANCE (Session #%d) ===\n", unique_days);
  if (unique_days == 10)
    printf("NOTE: This is the 10th session. Eligibility will be calculated.\n");

  fpStudents = fopen("students.csv", "r");
  if (fpStudents == NULL) {
    printf("\nNo students found. Please add students first.\n");
    return;
  }

  fpAttendance = fopen("attendance.csv", "a");
  if (fpAttendance == NULL) {
    printf("Error opening attendance file.\n");
    fclose(fpStudents);
    return;
  }

  // Skip Header of students.csv
  char buffer[200];
  fgets(buffer, sizeof(buffer), fpStudents);

  int count = 0;

  while (fscanf(fpStudents, "%[^,],%[^,],%[^\n]\n", name, matric_num,
                department) != EOF) {
    count++;
    // Find current accumulated score
    int current_score = 0;
    for (int i = 0; i < student_count_stats; i++) {
      if (strcmp(student_scores[i].matric, matric_num) == 0) {
        current_score = student_scores[i].score;
        break;
      }
    }

    do {
      printf("%d. %-30s (Current Score: %d%%): ", count, name, current_score);
      scanf(" %c", &att);
      clearInputBuffer();
      att = toupper(att);

      if (att != 'P' && att != 'A') {
        printf("   Invalid! Please enter P or A only.\n");
      }
    } while (att != 'P' && att != 'A');

    int score_gain = (att == 'P') ? 10 : 0;
    int new_score = current_score + score_gain;

    char eligibility[20] = "";
    if (unique_days >= 10) {
      if (new_score >= 70)
        strcpy(eligibility, "ELIGIBLE");
      else
        strcpy(eligibility, "INELIGIBLE");
    }

    fprintf(fpAttendance, "%s,%s,%s,%s,%c,%d,%s\n", date, name, matric_num,
            department, att, new_score, eligibility);
  }

  fclose(fpStudents);
  fclose(fpAttendance);

  printf("\nAttendance recorded successfully!\n");
}

void viewAttendanceSummary() {
  FILE *fp = fopen("attendance.csv", "r");
  if (fp == NULL) {
    printf("No attendance records found.\n");
    return;
  }

  char line[1024];
  // Read Header
  if (!fgets(line, sizeof(line), fp)) {
    printf("Empty file.\n");
    fclose(fp);
    return;
  }

  printf("\n=== ATTENDANCE LOG ===\n");
  // Print custom header
  printf("%-12s %-20s %-12s %-6s %-6s %-12s\n", "Date", "Name", "Matric",
         "Stat", "Score", "Elig");
  printf("---------------------------------------------------------------------"
         "-----------\n");

  while (fgets(line, sizeof(line), fp)) {
    char *date = strtok(line, ",");
    char *name = strtok(NULL, ",");
    char *matric = strtok(NULL, ",");
    char *dept = strtok(NULL, ",");
    char *status = strtok(NULL, ",");
    char *score = strtok(NULL, ",");
    char *elig = strtok(NULL, ","); // Might be final token including newline

    if (elig)
      elig[strcspn(elig, "\n")] = 0;

    if (date && name && matric && status && score) {
      printf("%-12s %-20s %-12s %-6s %-6s %-12s\n", date, name, matric, status,
             score, elig ? elig : "");
    }
  }

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
  ensureStudentsFileExists();
  ensureAttendanceFileExists(); // Ensure CSV is ready
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
    scanf("%d", &choice);
    clearInputBuffer();

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