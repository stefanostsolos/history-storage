#include "benchmarker.hpp"

size_t Benchmarker::measureMemoryUsage(const HistoryStorage &storage)
{
    return storage.getMemoryUsage();
}

size_t Benchmarker::measureDiskUsage(const HistoryStorage &storage)
{
    return storage.getDiskUsage();
}

Benchmarker::BenchmarkResult Benchmarker::runWriteBenchmark(HistoryStorage &storage, const std::vector<std::unique_ptr<HistoryEntry>> &entries)
{
    auto start = std::chrono::high_resolution_clock::now();

    for (const auto &entry : entries)
    {
        storage.store(entry->clone());
    }

    auto end = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration<double>(end - start).count();

    return {
        "Write Benchmark",
        duration,
        measureMemoryUsage(storage),
        measureDiskUsage(storage)};
}

Benchmarker::BenchmarkResult Benchmarker::runReadBenchmark(HistoryStorage &storage, std::time_t start, std::time_t end)
{
    auto startTime = std::chrono::high_resolution_clock::now();

    auto results = storage.retrieve(start, end);

    auto endTime = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration<double>(endTime - startTime).count();

    return {
        "Read Benchmark",
        duration,
        measureMemoryUsage(storage),
        measureDiskUsage(storage)};
}

// Not used currently in run_benchmarks.cpp, but can be added to have a read/write mixed benchmark
Benchmarker::BenchmarkResult Benchmarker::runMixedBenchmark(HistoryStorage &storage, const std::vector<std::unique_ptr<HistoryEntry>> &entries, std::time_t start, std::time_t end)
{
    auto startTime = std::chrono::high_resolution_clock::now();

    for (const auto &entry : entries)
    {
        storage.store(entry->clone());
    }

    auto results = storage.retrieve(start, end);

    auto endTime = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration<double>(endTime - startTime).count();

    return {
        "Mixed Benchmark",
        duration,
        measureMemoryUsage(storage),
        measureDiskUsage(storage)};
}