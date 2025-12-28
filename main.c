#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

void addStudent() {
    FILE *fp;
    char name[50];
    char department[30];
    char matric_num[10];
    char choice;

    fp = fopen("students.csv", "a"); // Open CSV in append mode
    if (fp == NULL) {
        printf("Error opening file.\n");
        return;
    }

    do {
        printf("Enter student name: ");
        scanf(" %[^\n]s", name); // allows spaces

        printf("Enter student matric number: ");
        scanf("%s", matric_num);

        printf("Enter student department: ");
        scanf(" %[^\n]s", department);

        fprintf(fp, "%s,%s,%s\n", name, matric_num, department);
        printf("Student data saved successfully!\n");

        printf("Do you want to add another student? (y/n): ");
        scanf(" %c", &choice);

    } while(choice == 'y' || choice == 'Y');

    fclose(fp);
}

void markAttendance() {
    FILE *fpStudents, *fpAttendance;
    char name[50], department[30], matric_num[10];
    char att;
    char date[11];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    sprintf(date, "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

    fpStudents = fopen("students.csv", "r");
    if(fpStudents == NULL) {
        printf("No students found. Please add students first.\n");
        return;
    }

    fpAttendance = fopen("attendance.csv", "a");
    if(fpAttendance == NULL) {
        printf("Error opening attendance file.\n");
        fclose(fpStudents);
        return;
    }

    printf("Mark attendance for date: %s\n", date);
    printf("Enter P for Present, A for Absent.\n");

    while(fscanf(fpStudents, "%[^,],%[^,],%[^\n]\n", name, matric_num, department) != EOF) {
        do {
            printf("%s (%s, %s): ", name, matric_num, department);
            scanf(" %c", &att);
            att = toupper(att);
        } while(att != 'P' && att != 'A');

        fprintf(fpAttendance, "%s,%s,%s,%s,%c\n", date, name, matric_num, department, att);
    }

    printf("Attendance recorded successfully!\n");

    fclose(fpStudents);
    fclose(fpAttendance);
}

void viewAttendanceSummary() {
    FILE *fp;
    char date[11], name[50], department[30], matric_num[10];
    char att;

    fp = fopen("attendance.csv", "r");
    if(fp == NULL) {
        printf("No attendance records found.\n");
        return;
    }

    printf("\n=== Attendance Summary ===\n");
    printf("Date       Name          Matric    Department  Attendance\n");
    printf("--------------------------------------------------------\n");

    while(fscanf(fp, "%[^,],%[^,],%[^,],%[^,],%c\n", date, name, matric_num, department, &att) != EOF) {
        printf("%s  %-12s %-8s %-10s %c\n", date, name, matric_num, department, att);
    }

    fclose(fp);
}

int main() {
    int choice;

    do {
        printf("\n=== Student Attendance System ===\n");
        printf("1. Add new student\n");
        printf("2. Mark attendance\n");
        printf("3. View attendance summary\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

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
                printf("Exiting program. Goodbye!\n");
                break;
            default:
                printf("Invalid choice! Please try again.\n");
        }
    } while(choice != 4 || !choice);

    return 0;
}
