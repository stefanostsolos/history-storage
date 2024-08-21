#include "benchmarker.hpp"
#include "sqlite_disk_storage.hpp"
#include <iostream>
#include <vector>
#include <random>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <sstream>

std::vector<std::unique_ptr<HistoryEntry>> generateTestData(size_t count)
{
    std::vector<std::unique_ptr<HistoryEntry>> entries;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis_double(0, 1000);
    std::uniform_int_distribution<> dis_int(0, 1000);
    std::uniform_int_distribution<> dis_bool(0, 1);

    auto now = std::time(nullptr);
    for (size_t i = 0; i < count; ++i)
    {
        switch (i % 4)
        {
        case 0:
            entries.push_back(std::make_unique<TypedHistoryEntry<double>>(now + i, dis_double(gen)));
            break;
        case 1:
            entries.push_back(std::make_unique<TypedHistoryEntry<int>>(now + i, dis_int(gen)));
            break;
        case 2:
            entries.push_back(std::make_unique<TypedHistoryEntry<bool>>(now + i, dis_bool(gen)));
            break;
        case 3:
            entries.push_back(std::make_unique<TypedHistoryEntry<std::string>>(now + i, std::string(50, 'a' + (i % 26))));
            break;
        }
    }
    return entries;
}

void runBenchmark(size_t ramCapacity, std::chrono::seconds flushInterval,
                  const std::vector<std::unique_ptr<HistoryEntry>> &testData, const std::string &configName,
                  std::ofstream &reportFile, double highWatermark, double lowWatermark)
{
    std::cout << "Running benchmark for " << configName << " configuration" << std::endl;
    std::cout << "RAM Capacity: " << ramCapacity << ", Flush Interval: " << flushInterval.count()
              << "s, High Watermark: " << highWatermark << ", Low Watermark: " << lowWatermark << std::endl;

    std::string dbName = "benchmark_" + configName + ".db";
    std::stringstream benchmarkOutput;

    try
    {
        auto diskStorage = std::make_unique<SQLiteDiskStorage>(dbName);
        diskStorage->clear();
        auto storage = std::make_unique<ConcreteHistoryStorage>(ramCapacity, diskStorage.get(), flushInterval, highWatermark, lowWatermark);

        size_t storedInRam = 0;
        size_t storedInDb = 0;
        size_t totalStored = 0;
        size_t flushCount = 0;

        auto startWrite = std::chrono::high_resolution_clock::now();
        for (const auto &entry : testData)
        {
            storage->store(entry->clone());
            totalStored++;

            if (totalStored % 1000 == 0)
            {
                storedInDb = diskStorage->getEntryCount();
                storedInRam = storage->getInRamCount();
                flushCount = storage->getFlushCount();
                benchmarkOutput << "Stored " << totalStored << " entries (RAM: " << storedInRam
                                << ", DB: " << storedInDb << ", Flushes: " << flushCount << ")" << std::endl;
            }
        }
        auto endWrite = std::chrono::high_resolution_clock::now();
        double writeDuration = std::chrono::duration<double>(endWrite - startWrite).count();
        double writeSpeed = totalStored / writeDuration;

        storedInDb = diskStorage->getEntryCount();
        storedInRam = storage->getInRamCount();
        flushCount = storage->getFlushCount();

        auto startRead = std::chrono::high_resolution_clock::now();
        auto retrievedData = storage->retrieve(testData.front()->getTimestamp(), testData.back()->getTimestamp());
        auto endRead = std::chrono::high_resolution_clock::now();
        double readDuration = std::chrono::duration<double>(endRead - startRead).count();
        double readSpeed = retrievedData.size() / readDuration;

        std::cout << "Benchmark completed for " << configName << " configuration" << std::endl;
        std::cout << "Total entries stored: " << totalStored << " (RAM: " << storedInRam << ", DB: " << storedInDb << ")" << std::endl;
        std::cout << "Total flushes: " << flushCount << std::endl;
        std::cout << "Write Speed: " << std::fixed << std::setprecision(2) << writeSpeed << " entries/second" << std::endl;
        std::cout << "Read Speed: " << std::fixed << std::setprecision(2) << readSpeed << " entries/second" << std::endl;
        std::cout << std::endl;

        // Write detailed results to the report file
        reportFile << "=== Benchmark Results for " << configName << " configuration ===" << std::endl;
        reportFile << "RAM Capacity: " << ramCapacity << ", Flush Interval: " << flushInterval.count()
                   << "s, High Watermark: " << highWatermark << ", Low Watermark: " << lowWatermark << std::endl;
        reportFile << benchmarkOutput.str();
        reportFile << "Total entries stored: " << totalStored << " (RAM: " << storedInRam << ", DB: " << storedInDb << ")" << std::endl;
        reportFile << "Total flushes: " << flushCount << std::endl;
        reportFile << "Write Speed: " << std::fixed << std::setprecision(2) << writeSpeed << " entries/second" << std::endl;
        reportFile << "Read Speed: " << std::fixed << std::setprecision(2) << readSpeed << " entries/second" << std::endl;
        reportFile << "Final Memory Usage: " << storage->getMemoryUsage() << " bytes" << std::endl;
        reportFile << "Final Disk Usage: " << storage->getDiskUsage() << " bytes" << std::endl;
        reportFile << "Retrieved Entries: " << retrievedData.size() << std::endl;
        reportFile << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error during benchmark: " << e.what() << std::endl;
        reportFile << "Error during benchmark: " << e.what() << std::endl;
    }
}

int main()
{
    std::vector<size_t> dataSizes = {10000, 50000, 100000};
    std::vector<std::pair<double, double>> watermarkConfigs = {
        {0.95, 0.80}, // Low configuration
        {0.98, 0.85}, // Medium configuration
        {0.99, 0.90}, // More aggressive configuration
    };

    // Run benchmarks for different data sizes
    for (size_t dataSize : dataSizes)
    {
        std::cout << "Generating " << dataSize << " test entries." << std::endl;
        auto testData = generateTestData(dataSize);

        std::ofstream reportFile("benchmark_report_" + std::to_string(dataSize) + ".txt");

        for (const auto &[highWatermark, lowWatermark] : watermarkConfigs)
        {
            std::string watermarkConfig = "H" + std::to_string(int(highWatermark * 100)) +
                                          "L" + std::to_string(int(lowWatermark * 100));

            // Run benchmarks for different configurations
            runBenchmark(2000, std::chrono::seconds(60), testData,
                         "small_" + watermarkConfig + "_" + std::to_string(dataSize),
                         reportFile, highWatermark, lowWatermark);

            runBenchmark(5000, std::chrono::seconds(120), testData,
                         "medium_" + watermarkConfig + "_" + std::to_string(dataSize),
                         reportFile, highWatermark, lowWatermark);

            runBenchmark(10000, std::chrono::seconds(300), testData,
                         "large_" + watermarkConfig + "_" + std::to_string(dataSize),
                         reportFile, highWatermark, lowWatermark);
        }

        std::cout << "Benchmarks completed for data size " << dataSize
                  << ". Detailed results written to benchmark_report_" << dataSize << ".txt" << std::endl;
    }

    return 0;
}