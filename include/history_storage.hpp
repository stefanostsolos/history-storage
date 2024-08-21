#pragma once
#include "history_entry.hpp"
#include "circular_buffer.hpp"
#include "disk_storage.hpp"
#include <vector>
#include <memory>
#include <chrono>

class HistoryStorage
{
public:
    virtual ~HistoryStorage() = default;
    virtual void store(std::unique_ptr<HistoryEntry> entry) = 0;
    virtual std::vector<std::unique_ptr<HistoryEntry>> retrieve(std::time_t start, std::time_t end) = 0;
    virtual void flush() = 0;
    virtual size_t getMemoryUsage() const = 0;
    virtual size_t getDiskUsage() const = 0;
};

class ConcreteHistoryStorage : public HistoryStorage
{
private:
    CircularBuffer<HistoryEntry> ramBuffer;
    DiskStorage *diskStorage;
    std::chrono::steady_clock::time_point lastFlushTime;
    size_t entriesSinceLastFlush;
    const std::chrono::seconds FLUSH_INTERVAL;
    size_t totalFlushCount;

    const double HIGH_WATERMARK; // % of RAM capacity
    const double LOW_WATERMARK;  // % of RAM capacity

public:
    ConcreteHistoryStorage(size_t ramCapacity, DiskStorage *disk,
                           std::chrono::seconds flushInterval, double highWatermark, double lowWatermark);

    void store(std::unique_ptr<HistoryEntry> entry) override;
    std::vector<std::unique_ptr<HistoryEntry>> retrieve(std::time_t start, std::time_t end) override;
    void flush() override;
    size_t getMemoryUsage() const override;
    size_t getDiskUsage() const override;

    size_t getInRamCount() const;
    size_t getFlushCount() const;

private:
    bool isRamBufferNearlyFull() const
    {
        // Consider the buffer nearly full when it's at HIGH_WATERMARK% capacity
        return ramBuffer.getSize() >= (ramBuffer.getCapacity() * HIGH_WATERMARK);
    }
    std::vector<std::unique_ptr<HistoryEntry>> retrieveFromRAM(std::time_t start, std::time_t end) const;
};