cmake_minimum_required(VERSION 3.10)
project(MyCProject)

set(CMAKE_C_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_COMPILER "/usr/bin/gcc")

find_package(SQLite3 REQUIRED)

add_executable(MyCProject-ex src/main.c)

add_executable(Swap-ex src/swap_main.c)

add_executable(Print_hist-ex src/printHist_main.c src/itec.c)
target_link_libraries(Print_hist-ex SQLite::SQLite3) 

add_executable(sql-ex src/sql_main.c src/itec.c )
target_link_libraries(sql-ex SQLite::SQLite3)

add_executable(read_com_port-ex src/read_com_port.c src/itec.c)
target_link_libraries(read_com_port-ex SQLite::SQLite3)

add_executable(LaborAufgaben-ex src/LaborAufgaben.c src/itec.c)
target_link_libraries(LaborAufgaben-ex SQLite::SQLite3)

add_executable(serial-reader-example src/serial_reader-example.c src/itec.c)
target_link_libraries(serial-reader-example SQLite::SQLite3)