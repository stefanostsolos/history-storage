#include "history_storage.hpp"
#include <algorithm>
#include <stdexcept>
#include <iostream>

ConcreteHistoryStorage::ConcreteHistoryStorage(size_t ramCapacity, DiskStorage *disk,
                                               std::chrono::seconds flushInterval, double highWatermark, double lowWatermark)
    : ramBuffer(ramCapacity),
      diskStorage(disk),
      lastFlushTime(std::chrono::steady_clock::now()),
      entriesSinceLastFlush(0),
      FLUSH_INTERVAL(flushInterval),
      totalFlushCount(0),
      HIGH_WATERMARK(highWatermark),
      LOW_WATERMARK(lowWatermark)
{
}

void ConcreteHistoryStorage::store(std::unique_ptr<HistoryEntry> entry)
{
    if (isRamBufferNearlyFull())
    {
        flush();
    }

    ramBuffer.push(std::move(entry));
    entriesSinceLastFlush++;

    auto now = std::chrono::steady_clock::now();
    if (now - lastFlushTime >= FLUSH_INTERVAL)
    {
        flush();
        lastFlushTime = now;
        entriesSinceLastFlush = 0;
    }

    if (entriesSinceLastFlush % 100 == 0) // Print basic info every 100 entries
    {
        std::cout << "Stored " << entriesSinceLastFlush << " entries (RAM fill ratio: "
                  << static_cast<double>(ramBuffer.getSize()) / ramBuffer.getCapacity()
                  << ")" << std::endl;
    }
}

std::vector<std::unique_ptr<HistoryEntry>> ConcreteHistoryStorage::retrieve(std::time_t start, std::time_t end)
{
    auto ramEntries = retrieveFromRAM(start, end);
    auto diskEntries = diskStorage->retrieve(start, end);

    std::vector<std::unique_ptr<HistoryEntry>> allEntries;
    allEntries.reserve(ramEntries.size() + diskEntries.size());

    for (auto &entry : ramEntries)
    {
        allEntries.push_back(std::move(entry));
    }
    for (auto &entry : diskEntries)
    {
        allEntries.push_back(std::move(entry));
    }

    std::sort(allEntries.begin(), allEntries.end(),
              [](const std::unique_ptr<HistoryEntry> &a, const std::unique_ptr<HistoryEntry> &b)
              {
                  return a->getTimestamp() < b->getTimestamp();
              });

    return allEntries;
}

void ConcreteHistoryStorage::flush()
{
    size_t currentSize = ramBuffer.getSize();
    size_t capacity = ramBuffer.getCapacity();
    size_t lowWatermarkSize = static_cast<size_t>(capacity * LOW_WATERMARK);
    size_t entriesaboutToFlush = currentSize > lowWatermarkSize ? currentSize - lowWatermarkSize : 0;

    std::vector<std::unique_ptr<HistoryEntry>> entriesToFlush;
    for (size_t i = 0; i < entriesaboutToFlush && !ramBuffer.isEmpty(); ++i)
    {
        entriesToFlush.push_back(ramBuffer.pop());
    }

    if (!entriesToFlush.empty())
    {
        diskStorage->flush(entriesToFlush);
        totalFlushCount++;
        std::cout << "Flushed " << entriesToFlush.size() << " entries (RAM fill ratio before flush: "
                  << static_cast<double>(currentSize) / capacity
                  << ", after flush: " << static_cast<double>(ramBuffer.getSize()) / capacity
                  << ")" << std::endl;
    }
}

size_t ConcreteHistoryStorage::getMemoryUsage() const
{
    size_t total = 0;
    for (size_t i = 0; i < ramBuffer.getSize(); ++i)
    {
        total += ramBuffer.at(i).getSize();
    }
    return total;
}

size_t ConcreteHistoryStorage::getDiskUsage() const
{
    return diskStorage->getDiskUsage();
}

std::vector<std::unique_ptr<HistoryEntry>> ConcreteHistoryStorage::retrieveFromRAM(std::time_t start, std::time_t end) const
{
    std::vector<std::unique_ptr<HistoryEntry>> result;
    for (size_t i = 0; i < ramBuffer.getSize(); ++i)
    {
        const auto &entry = ramBuffer.at(i);
        if (entry.getTimestamp() >= start && entry.getTimestamp() <= end)
        {
            result.push_back(entry.clone());
        }
    }
    return result;
}

size_t ConcreteHistoryStorage::getInRamCount() const
{
    return ramBuffer.getSize();
}

size_t ConcreteHistoryStorage::getFlushCount() const
{
    return totalFlushCount;
}