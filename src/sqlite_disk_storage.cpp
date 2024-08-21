#include "sqlite_disk_storage.hpp"
#include <stdexcept>
#include <filesystem>
#include <iostream>

SQLiteDiskStorage::SQLiteDiskStorage(const std::string &dbPath) : dbPath(dbPath)
{
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK)
    {
        throw std::runtime_error("Can't open database: " + std::string(sqlite3_errmsg(db)));
    }
    createTable();
    optimizeConnection();
    prepareStatements();
}

SQLiteDiskStorage::~SQLiteDiskStorage()
{
    sqlite3_finalize(insertStmt);
    sqlite3_close(db);
}

void SQLiteDiskStorage::createTable()
{
    const char *sql = "CREATE TABLE IF NOT EXISTS history ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "timestamp INTEGER NOT NULL,"
                      "type TEXT NOT NULL,"
                      "value BLOB NOT NULL)";

    char *errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        std::string error = "SQL error: " + std::string(errMsg);
        sqlite3_free(errMsg);
        throw std::runtime_error(error);
    }
}

void SQLiteDiskStorage::optimizeConnection()
{
    const char *sql = "PRAGMA synchronous = NORMAL; "
                      "PRAGMA journal_mode = WAL; "
                      "PRAGMA temp_store = MEMORY;";
    char *errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        std::string error = "SQL error: " + std::string(errMsg);
        sqlite3_free(errMsg);
        throw std::runtime_error(error);
    }
}

void SQLiteDiskStorage::prepareStatements()
{
    const char *sql = "INSERT INTO history (timestamp, type, value) VALUES (?, ?, ?)";
    if (sqlite3_prepare_v2(db, sql, -1, &insertStmt, nullptr) != SQLITE_OK)
    {
        throw std::runtime_error("Failed to prepare insert statement");
    }
}

void SQLiteDiskStorage::flush(const std::vector<std::unique_ptr<HistoryEntry>> &entries)
{
    sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);

    for (const auto &entry : entries)
    {
        sqlite3_bind_int64(insertStmt, 1, entry->getTimestamp());

        // Determine the type and bind appropriately
        if (auto *doubleEntry = dynamic_cast<TypedHistoryEntry<double> *>(entry.get()))
        {
            sqlite3_bind_text(insertStmt, 2, "double", -1, SQLITE_STATIC);
            sqlite3_bind_double(insertStmt, 3, doubleEntry->getValue());
        }
        else if (auto *intEntry = dynamic_cast<TypedHistoryEntry<int> *>(entry.get()))
        {
            sqlite3_bind_text(insertStmt, 2, "int", -1, SQLITE_STATIC);
            sqlite3_bind_int(insertStmt, 3, intEntry->getValue());
        }
        else if (auto *boolEntry = dynamic_cast<TypedHistoryEntry<bool> *>(entry.get()))
        {
            sqlite3_bind_text(insertStmt, 2, "bool", -1, SQLITE_STATIC);
            sqlite3_bind_int(insertStmt, 3, boolEntry->getValue() ? 1 : 0);
        }
        else if (auto *stringEntry = dynamic_cast<TypedHistoryEntry<std::string> *>(entry.get()))
        {
            sqlite3_bind_text(insertStmt, 2, "string", -1, SQLITE_STATIC);
            sqlite3_bind_text(insertStmt, 3, stringEntry->getValue().c_str(), -1, SQLITE_TRANSIENT);
        }
        else
        {
            throw std::runtime_error("Unknown entry type");
        }

        if (sqlite3_step(insertStmt) != SQLITE_DONE)
        {
            std::cerr << "Error inserting entry: " << sqlite3_errmsg(db) << std::endl;
        }

        sqlite3_reset(insertStmt);
    }

    sqlite3_exec(db, "END TRANSACTION", nullptr, nullptr, nullptr);
}

std::vector<std::unique_ptr<HistoryEntry>> SQLiteDiskStorage::retrieve(std::time_t start, std::time_t end)
{
    std::vector<std::unique_ptr<HistoryEntry>> results;
    const char *sql = "SELECT timestamp, type, value FROM history WHERE timestamp BETWEEN ? AND ?";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        throw std::runtime_error("Failed to prepare statement");
    }

    sqlite3_bind_int64(stmt, 1, start);
    sqlite3_bind_int64(stmt, 2, end);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        std::time_t timestamp = sqlite3_column_int64(stmt, 0);
        std::string type = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));

        if (type == "double")
        {
            double value = sqlite3_column_double(stmt, 2);
            results.push_back(std::make_unique<TypedHistoryEntry<double>>(timestamp, value));
        }
        else if (type == "int")
        {
            int value = sqlite3_column_int(stmt, 2);
            results.push_back(std::make_unique<TypedHistoryEntry<int>>(timestamp, value));
        }
        else if (type == "bool")
        {
            bool value = sqlite3_column_int(stmt, 2) != 0;
            results.push_back(std::make_unique<TypedHistoryEntry<bool>>(timestamp, value));
        }
        else if (type == "string")
        {
            const char *value = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
            results.push_back(std::make_unique<TypedHistoryEntry<std::string>>(timestamp, std::string(value)));
        }
        else
        {
            throw std::runtime_error("Unknown type in database");
        }
    }

    sqlite3_finalize(stmt);
    return results;
}

size_t SQLiteDiskStorage::getDiskUsage() const
{
    std::string mainDbPath = dbPath;
    std::string walPath = mainDbPath + "-wal";
    std::string shmPath = mainDbPath + "-shm";

    size_t mainSize = std::filesystem::file_size(mainDbPath);
    size_t walSize = std::filesystem::exists(walPath) ? std::filesystem::file_size(walPath) : 0;
    size_t shmSize = std::filesystem::exists(shmPath) ? std::filesystem::file_size(shmPath) : 0;

    std::cout << "Main DB file (" << mainDbPath << ") size: " << mainSize << " bytes" << std::endl;
    std::cout << "WAL file size: " << walSize << " bytes" << std::endl;
    std::cout << "SHM file size: " << shmSize << " bytes" << std::endl;

    return mainSize + walSize + shmSize;
}

size_t SQLiteDiskStorage::getEntryCount() const
{
    const char *sql = "SELECT COUNT(*) FROM history";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        throw std::runtime_error("Failed to prepare statement for count query");
    }

    size_t count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        count = static_cast<size_t>(sqlite3_column_int64(stmt, 0));
    }

    sqlite3_finalize(stmt);
    return count;
}

void SQLiteDiskStorage::clear()
{
    const char *sql = "DELETE FROM history; VACUUM;";
    char *errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        std::string error = "SQL error in clear(): " + std::string(errMsg);
        sqlite3_free(errMsg);
        throw std::runtime_error(error);
    }

    std::cout << "Database cleared successfully." << std::endl;
}