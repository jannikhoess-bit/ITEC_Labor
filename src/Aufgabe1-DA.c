#include <stdio.h>
#include <sqlite3.h>
#include <stdio.h>


int main(int argc, char *argv[]) {
    sqlite3 *db;
    
    if (open_database(&db, "test.db") != 0) {
        return 1;
    }

    const char *sql = "CREATE TABLE IF NOT EXISTS students (id INTEGER PRIMARY KEY, name TEXT, age INTEGER);"
                      "INSERT INTO students (name, age) VALUES ('Charlie', 23);";

    execute_sql(db, sql);
    
    const char *sql_select = "SELECT * FROM students;";

    execute_sql(db, sql_select);

    sqlite3_close(db);
    
    return 0;
}
