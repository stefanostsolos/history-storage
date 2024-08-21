#pragma once
#include "history_entry.hpp"
#include <vector>
#include <memory>

class DiskStorage
{
public:
    virtual ~DiskStorage() = default;
    virtual void flush(const std::vector<std::unique_ptr<HistoryEntry>> &entries) = 0;
    virtual std::vector<std::unique_ptr<HistoryEntry>> retrieve(std::time_t start, std::time_t end) = 0;
    virtual size_t getDiskUsage() const = 0;
};