#pragma once
#include "history_storage.hpp"
#include <chrono>
#include <vector>
#include <string>
#include <memory>

class Benchmarker
{
public:
    struct BenchmarkResult
    {
        std::string name;
        double duration;
        size_t memoryUsage;
        size_t diskUsage;
    };

    static BenchmarkResult runWriteBenchmark(HistoryStorage &storage, const std::vector<std::unique_ptr<HistoryEntry>> &entries);
    static BenchmarkResult runReadBenchmark(HistoryStorage &storage, std::time_t start, std::time_t end);
    static BenchmarkResult runMixedBenchmark(HistoryStorage &storage, const std::vector<std::unique_ptr<HistoryEntry>> &entries, std::time_t start, std::time_t end);

private:
    static size_t measureMemoryUsage(const HistoryStorage &storage);
    static size_t measureDiskUsage(const HistoryStorage &storage);
};