# HistoryStorage Task

This project implements a history storage system with in-memory and on-disk components using SQLite.

## Prerequisites

- CMake (version 3.10 or higher)
- C++17 compatible compiler
- SQLite3 library

## Building the Project

1. Clone the repository:
   ```
   git clone https://github.com/stefanostsolos/history-storage.git
   cd history-storage
   ```

2. Create a build directory and navigate to it:
   ```
   mkdir build
   cd build
   ```

3. Run CMake and build the project (you may need to use different commands for building depending on your working environment/):
   ```
   cmake ..
   make
   ```

## Running the Benchmarks

After building the project, you can run the benchmarks using the following command:

```
./run_benchmarks
```

If there are permission issues when running the executable, you might need to make it executable:
```chmod +x run_benchmarks```

This will execute a series of benchmarks and output the results to the console. A detailed report will be saved in the file named `benchmark_report_NUM.txt`, where ```NUM``` the number of dataset size.

## Project Structure

- `src/`: Contains the source files for the project
- `include/`: Contains the header files
- `benchmarks/`: Contains the benchmark runner
- `CMakeLists.txt`: CMake configuration file

## Files in Repository

The following files and directories are included in the Git repository:

- All `.cpp` and `.hpp` files
- `CMakeLists.txt`
- `src/` directory
- `include/` directory
- `benchmarks/` directory
- `README.md`
- `.gitignore`

Build artifacts, IDE-specific files, and other generated files are excluded from the repository. See the `.gitignore` file for details.
