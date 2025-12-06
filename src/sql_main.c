



// int open_database(sqlite3 **db, const char *db_name) {
//     int rc = sqlite3_open(db_name, db);
    
//     if (rc != SQLITE_OK) {
//         fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(*db));
//         return 1;
//     }
//     return 0;
// }

// int execute_sql(sqlite3 *db, const char *sql) {
//     const char *err_msg = 0;
//     sqlite3_stmt *stmt;
//     int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, &err_msg);
    
//     if (rc != SQLITE_OK) {
//         fprintf(stderr, "SQL error: %s\n", err_msg);
//         sqlite3_free((void*)err_msg);
//         return rc;
//     }
    
//     while (sqlite3_step(stmt) == SQLITE_ROW) {
//         int cols = sqlite3_column_count(stmt);
//         for (int i = 0; i < cols; i++) {
//             printf("%s | ", sqlite3_column_text(stmt, i));
//         }
//         printf("\n");
//     }
    
//     sqlite3_finalize(stmt);

//     return SQLITE_OK;
// }

// int main() {
//     sqlite3 *db;
    
//     if (open_database(&db, "test.db") != 0) {
//         return 1;
//     }

//     const char *sql = "CREATE TABLE IF NOT EXISTS students (id INTEGER PRIMARY KEY, name TEXT, age INTEGER);"
//                       "INSERT INTO students (name, age) VALUES ('Charlie', 23);";

//     execute_sql(db, sql);
    
//     const char *sql_select = "SELECT * FROM students;";

//     execute_sql(db, sql_select);

//     sqlite3_close(db);
    
//     return 0;
// }

//Chat GPT
#include <stdio.h>
#include <sqlite3.h>

#include "itec.h"


int main() {
    sqlite3 *db;
    
    if (open_database(&db, "test.db") != 0) {
        return 1;
    }

    execute_sql(db,
        "CREATE TABLE IF NOT EXISTS students (id INTEGER PRIMARY KEY, name TEXT, age INTEGER);");
    
    execute_sql(db,
        "INSERT INTO students (name, age) VALUES ('Charlie', 23);");

    printf("\n--- Tabelleninhalt ---\n");
    execute_sql(db,
        "SELECT * FROM students;");

    sqlite3_close(db);
    
    return 0;
}
