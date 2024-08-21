#include "circular_buffer.hpp"
#include "history_entry.hpp"

template class CircularBuffer<HistoryEntry *>;

template <typename T>
size_t getCircularBufferMemoryUsage(const CircularBuffer<T> &buffer)
{
    return sizeof(CircularBuffer<T>) + sizeof(T) * buffer.getCapacity();
}

template size_t getCircularBufferMemoryUsage(const CircularBuffer<HistoryEntry *> &buffer);