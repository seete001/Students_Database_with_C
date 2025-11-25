#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

#define GREEN  "\033[32m"
#define RED    "\033[31m"
#define YELLOW "\033[33m"
#define RESET  "\033[0m"

#define FILE_PATH "students.db"

enum {SAVE_STUDENT_DATA=1, LOAD_ALL_STUDENTS, SEARCH_STUDENT, DELETE_STUDENT, EXIT};

typedef struct {
    int age;
    char name[50];
} Student;

/* -------------------- UTILITY FUNCTIONS -------------------- */

int safe_int_input() {
    int x;
    while (scanf("%d", &x) != 1) {
        printf(RED "Invalid input. Enter a valid number: " RESET);
        while (getchar() != '\n');
    }
    while (getchar() != '\n');
    return x;
}

void pause_screen() {
    printf(YELLOW "\nPress ENTER to continue..." RESET);
    getchar();
}

int show_menu() {
    printf("\033[2J\033[H");

    printf("+--------------------------------+\n");
    printf("|         STUDENT PORTAL         |\n");
    printf("+--------------------------------+\n");
    printf("| 1. Save Student Data           |\n");
    printf("| 2. Load All Students           |\n");
    printf("| 3. Search Student              |\n");
    printf("| 4. Delete Student              |\n");
    printf("| 5. Exit                        |\n");
    printf("+--------------------------------+\n");
    printf("Enter your choice: ");

    return safe_int_input();
}

/* -------------------- DATABASE FUNCTIONS -------------------- */

int db_init(const char *path) {
    sqlite3 *db;
    char *err_msg = NULL;

    int rc = sqlite3_open(path, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    const char *sql =
        "CREATE TABLE IF NOT EXISTS Students ("
        "Id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "Name TEXT, "
        "Age INT"
        ");";

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot execute command: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }

    printf(GREEN "Database initialized.\n" RESET);
    sqlite3_close(db);
    return 0;
}

int db_process_data(const char *sql) {
    sqlite3 *db;
    char *err_msg = NULL;

    int rc = sqlite3_open(FILE_PATH, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }

    sqlite3_close(db);
    return 0;
}

/* -------------------- CRUD OPERATIONS -------------------- */

int save_student(const Student *s) {
    char sql[200];
    snprintf(sql, sizeof(sql),
             "INSERT INTO Students (Name, Age) VALUES('%s', %d);",
             s->name, s->age);

    if (db_process_data(sql) == 0)
        printf(GREEN "Student saved successfully.\n" RESET);

    return 0;
}

int load_all_students() {
    sqlite3 *db;
    sqlite3_stmt *stmt;

    int rc = sqlite3_open(FILE_PATH, &db);
    if (rc != SQLITE_OK) {
        printf(RED "Cannot open database\n" RESET);
        return 1;
    }

    const char *sql = "SELECT Id, Name, Age FROM Students;";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        printf(RED "Failed to fetch data.\n" RESET);
        sqlite3_close(db);
        return 1;
    }

    printf("\n--- All Students ---\n");

    int printed_any = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printed_any = 1;
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char *name = sqlite3_column_text(stmt, 1);
        int age = sqlite3_column_int(stmt, 2);

        printf("ID: %d | Name: %s | Age: %d\n", id, name, age);
    }

    if (!printed_any)
        printf(YELLOW "No students found.\n" RESET);

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}

int search_student(const char *name) {
    sqlite3 *db;
    sqlite3_stmt *stmt;

    sqlite3_open(FILE_PATH, &db);

    const char *sql = "SELECT Id, Name, Age FROM Students WHERE Name = ?;";

    sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);

    printf("\n--- Search Results ---\n");

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        printf(GREEN "Student found:\n" RESET);
        printf("ID: %d | Name: %s | Age: %d\n",
               sqlite3_column_int(stmt, 0),
               sqlite3_column_text(stmt, 1),
               sqlite3_column_int(stmt, 2));
    } else {
        printf(RED "No student found with that name.\n" RESET);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}

int delete_student(const char *name) {
    char sql[200];
    snprintf(sql, sizeof(sql),
             "DELETE FROM Students WHERE Name = '%s';",
             name);

    if (db_process_data(sql) == 0)
        printf(GREEN "Student deleted (if existed).\n" RESET);

    return 0;
}

/* -------------------- MAIN PROGRAM -------------------- */

int main() {
    db_init(FILE_PATH);

    while (1) {
        Student student;
        char name[50];

        int choice = show_menu();

        switch (choice) {
            case SAVE_STUDENT_DATA:
                printf("Enter student name: ");
                scanf("%s", student.name);
                printf("Enter student age: ");
                student.age = safe_int_input();
                save_student(&student);
                pause_screen();
                break;

            case LOAD_ALL_STUDENTS:
                load_all_students();
                pause_screen();
                break;

            case SEARCH_STUDENT:
                printf("Enter name to search: ");
                scanf("%s", name);
                search_student(name);
                pause_screen();
                break;

            case DELETE_STUDENT:
                printf("Enter name to delete: ");
                scanf("%s", name);
                delete_student(name);
                pause_screen();
                break;

            case EXIT:
                printf("Goodbye!\n");
                goto end_loop;

            default:
                printf(RED "Invalid choice.\n" RESET);
                pause_screen();
        }
    }
end_loop:
	return 0;
}

