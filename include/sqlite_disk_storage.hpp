#pragma once
#include "disk_storage.hpp"
#include "sqlite3.h"
#include <string>

class SQLiteDiskStorage : public DiskStorage
{
private:
    sqlite3 *db;
    sqlite3_stmt *insertStmt;
    std::string dbPath;

public:
    SQLiteDiskStorage(const std::string &dbPath);
    ~SQLiteDiskStorage();

    void flush(const std::vector<std::unique_ptr<HistoryEntry>> &entries) override;
    std::vector<std::unique_ptr<HistoryEntry>> retrieve(std::time_t start, std::time_t end) override;
    size_t getDiskUsage() const override;
    size_t getEntryCount() const;
    void clear();

private:
    void createTable();
    void prepareStatements();
    void optimizeConnection();
};