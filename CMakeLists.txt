cmake_minimum_required(VERSION 3.10)
project(MyCProject)

set(CMAKE_C_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

find_package(SQLite3 REQUIRED)

add_executable(MyCProject-ex src/main.c)

add_executable(Swap-ex src/swap_main.c)

add_executable(Print_hist-ex src/printHist_main.c src/itec.c)

add_executable(sql-ex sql_main.c)
target_link_libraries(sql-ex SQLite::SQLite3)
