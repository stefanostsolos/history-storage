cmake_minimum_required(VERSION 3.10)
project(HistoryStorage)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

# Source files
set(SOURCES
    src/history_entry.cpp
    src/circular_buffer.cpp
    src/disk_storage.cpp
    src/history_storage.cpp
    src/sqlite_disk_storage.cpp
    src/benchmarker.cpp
    src/sqlite3.c
)

# Library target
add_library(history_storage STATIC ${SOURCES})
# On Windows, we need to explicitly link against "advapi32" for SQLite
if(WIN32)
    target_link_libraries(history_storage advapi32)
endif()

# Benchmarking executable
add_executable(run_benchmarks benchmarks/run_benchmarks.cpp)
target_link_libraries(run_benchmarks history_storage)

# Ensure that the SQLite code is compiled as C
set_source_files_properties(src/sqlite3.c PROPERTIES LANGUAGE C)

# Add compiler definitions for SQLite
target_compile_definitions(history_storage PRIVATE 
    SQLITE_ENABLE_RTREE
    SQLITE_ENABLE_FTS4
    SQLITE_ENABLE_FTS5
    SQLITE_ENABLE_JSON1
    SQLITE_ENABLE_RBU
    SQLITE_ENABLE_DBSTAT_VTAB
)