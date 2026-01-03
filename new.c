#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <dirent.h>

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

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void addStudent() {
    FILE *fp;
    char name[50];
    char department[30];
    char matric_num[20];  // Increased buffer size
    char choice;

    fp = fopen("students.csv", "a"); 
    if (fp == NULL) {
        printf("Error opening file.\n");
        return;
    }

    do {
        printf("\n--- Add New Student ---\n");
        
        printf("Enter student name: ");
        if (fgets(name, sizeof(name), stdin) == NULL) break;
        name[strcspn(name, "\n")] = 0;  // Remove newline
        
        printf("Enter student matric number: ");
        if (fgets(matric_num, sizeof(matric_num), stdin) == NULL) break;
        matric_num[strcspn(matric_num, "\n")] = 0;  // Remove newline
        
        printf("Enter student department: ");
        if (fgets(department, sizeof(department), stdin) == NULL) break;
        department[strcspn(department, "\n")] = 0;  // Remove newline

        fprintf(fp, "%s,%s,%s\n", name, matric_num, department);
        printf("\nStudent data saved successfully!\n");
        
        printf("\nDo you want to add another student? (y/n): ");
        scanf(" %c", &choice);
        clearInputBuffer();

    } while(choice == 'y' || choice == 'Y');

    fclose(fp);
}

void markAttendance() {
    FILE *fpStudents, *fpAttendance;
    char name[50], department[30], matric_num[20];  // Increased buffer size
    char att;
    char date[11], time_str[9];
    char filename[100];
    char buffer[200];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    sprintf(date, "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    sprintf(time_str, "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
    sprintf(filename, "attendance_%s_%02d%02d%02d.csv", date, tm.tm_hour, tm.tm_min, tm.tm_sec);

    fpStudents = fopen("students.csv", "r");
    if(fpStudents == NULL) {
        printf("\nNo students found. Please add students first.\n");
        return;
    }

    fpAttendance = fopen(filename, "w");
    if(fpAttendance == NULL) {
        printf("Error creating attendance file.\n");
        fclose(fpStudents);
        return;
    }

    fprintf(fpAttendance, "Name,Matric,Department,Status\n");

    fgets(buffer, sizeof(buffer), fpStudents);

    printf("\n=== MARK ATTENDANCE ===\n");
    printf("Date: %s | Time: %s\n", date, time_str);
    printf("Enter P for Present, A for Absent\n\n");

    int count = 0;
    while(fscanf(fpStudents, "%[^,],%[^,],%[^\n]\n", name, matric_num, department) != EOF) {
        count++;
        do {
            printf("%d. %-30s: ", count, name);
            scanf(" %c", &att);
            clearInputBuffer();
            att = toupper(att);
            
            if(att != 'P' && att != 'A') {
                printf("   Invalid! Please enter P or A only.\n");
            }
        } while(att != 'P' && att != 'A');

        fprintf(fpAttendance, "%s,%s,%s,%c\n", name, matric_num, department, att);
    }

    if(count == 0) {
        printf("\nNo students found in the system.\n");
    } else {
        printf("\nAttendance recorded successfully!\n");
        printf("Total students marked: %d\n", count);
        printf("Saved as: %s\n", filename);
    }

    fclose(fpStudents);
    fclose(fpAttendance);
}

void listAttendanceFiles(char files[][100], int *count) {
    DIR *d;
    struct dirent *dir;
    *count = 0;

    d = opendir(".");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (strncmp(dir->d_name, "attendance_", 11) == 0 && 
                strstr(dir->d_name, ".csv") != NULL) {
                strcpy(files[*count], dir->d_name);
                (*count)++;
            }
        }
        closedir(d);
    }
    
    for(int i = 0; i < *count - 1; i++) {
        for(int j = i + 1; j < *count; j++) {
            if(strcmp(files[i], files[j]) < 0) {
                char temp[100];
                strcpy(temp, files[i]);
                strcpy(files[i], files[j]);
                strcpy(files[j], temp);
            }
        }
    }
}

void extractDateTimeFromFilename(char *filename, char *date, char *time_str) {
    char time_part[7];
    sscanf(filename, "attendance_%10[^_]_%6s", date, time_part);
    
    char temp[9];
    sprintf(temp, "%c%c:%c%c:%c%c", 
            time_part[0], time_part[1], 
            time_part[2], time_part[3], 
            time_part[4], time_part[5]);
    strcpy(time_str, temp);
}

void viewAttendanceSummary() {
    char files[100][100];
    int fileCount = 0;
    int choice;

    listAttendanceFiles(files, &fileCount);

    if(fileCount == 0) {
        printf("\nNo attendance records found.\n");
        return;
    }

    printf("\n=== AVAILABLE ATTENDANCE RECORDS ===\n");
    for(int i = 0; i < fileCount; i++) {
        char date[11], time_str[9];
        extractDateTimeFromFilename(files[i], date, time_str);
        printf("%d. %s at %s\n", i + 1, date, time_str);
    }

    printf("\nSelect attendance record (1-%d): ", fileCount);
    scanf("%d", &choice);
    clearInputBuffer();

    if(choice < 1 || choice > fileCount) {
        printf("\nInvalid selection!\n");
        return;
    }

    FILE *fp;
    char name[50], matric_num[20], department[30];  // Increased buffer size
    char att;
    char buffer[200];
    char date[11], time_str[9];

    fp = fopen(files[choice - 1], "r");
    if(fp == NULL) {
        printf("\nError opening file.\n");
        return;
    }

    extractDateTimeFromFilename(files[choice - 1], date, time_str);

    fgets(buffer, sizeof(buffer), fp);

    printf("\n=== ATTENDANCE SUMMARY ===\n");
    printf("Date: %s | Time: %s\n\n", date, time_str);
    printf("%-25s %-12s %-18s %-10s\n", "Name", "Matric", "Department", "Status");
    printf("-----------------------------------------------------------------------\n");

    int count = 0;
    int present = 0, absent = 0;
    while(fgets(buffer, sizeof(buffer), fp)) {
        // Parse the CSV line
        if(sscanf(buffer, "%[^,],%[^,],%[^,],%c", name, matric_num, department, &att) == 4) {
            char status[10];
            if(att == 'P') {
                strcpy(status, "Present");
                present++;
            } else {
                strcpy(status, "Absent");
                absent++;
            }
            printf("%-25s %-12s %-18s %-10s\n", name, matric_num, department, status);
            count++;
        }
    }

    printf("-----------------------------------------------------------------------\n");
    printf("Total: %d | Present: %d | Absent: %d\n", count, present, absent);

    fclose(fp);
}

void viewAllStudents() {
    FILE *fp;
    char name[50], department[30], matric_num[20];  // Increased buffer size
    char buffer[200];

    fp = fopen("students.csv", "r");
    if(fp == NULL) {
        printf("\nNo students found in the system.\n");
        return;
    }

    fgets(buffer, sizeof(buffer), fp);

    printf("\n=== ALL REGISTERED STUDENTS ===\n");
    printf("%-25s %-12s %-18s\n", "Name", "Matric", "Department");
    printf("-----------------------------------------------------------\n");

    int count = 0;
    while(fscanf(fp, "%[^,],%[^,],%[^\n]\n", name, matric_num, department) != EOF) {
        printf("%-25s %-12s %-18s\n", name, matric_num, department);
        count++;
    }

    printf("-----------------------------------------------------------\n");
    if(count == 0) {
        printf("No students registered yet.\n");
    } else {
        printf("Total students: %d\n", count);
    }

    fclose(fp);
}

int main() {
    ensureStudentsFileExists();
    int choice;

    printf("\n=== STUDENT ATTENDANCE SYSTEM ===\n");

    do {
        printf("\n=== MAIN MENU ===\n");
        printf("1. Add new student\n");
        printf("2. Mark attendance\n");
        printf("3. View attendance summary\n");
        printf("4. View all students\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        clearInputBuffer();

        switch(choice) {
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
    } while(choice != 5);

    return 0;
}