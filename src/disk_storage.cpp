#include "disk_storage.hpp"
#include <iostream>

// The DiskStorage class is an abstract base class (interface),
// so there's no implementation in this file.

size_t estimateEntriesSize(const std::vector<HistoryEntry *> &entries)
{
    size_t totalSize = 0;
    for (const auto &entry : entries)
    {
        totalSize += entry->getSize();
    }
    return totalSize;
}

void logDiskOperation(const std::string &operation, size_t entryCount)
{
    std::cout << "Disk operation: " << operation << ", Entries: " << entryCount << std::endl;
}