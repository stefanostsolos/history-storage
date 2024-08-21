#pragma once
#include <vector>
#include <stdexcept>
#include <memory>
#include "history_entry.hpp"

template <typename T>
class CircularBuffer
{
private:
    std::vector<std::unique_ptr<T>> buffer;
    size_t head = 0, tail = 0, size = 0;
    size_t capacity;

public:
    CircularBuffer(size_t cap) : capacity(cap), buffer(cap) {}

    void push(std::unique_ptr<T> item)
    {
        if (size == capacity)
        {
            buffer[head] = std::move(item);
            head = (head + 1) % capacity;
            tail = (tail + 1) % capacity;
        }
        else
        {
            buffer[tail] = std::move(item);
            tail = (tail + 1) % capacity;
            size++;
        }
    }

    std::unique_ptr<T> pop()
    {
        if (size == 0)
            throw std::runtime_error("Buffer is empty");
        std::unique_ptr<T> item = std::move(buffer[head]);
        head = (head + 1) % capacity;
        size--;
        return item;
    }

    const T &at(size_t index) const
    {
        if (index >= size)
            throw std::out_of_range("Index out of range");
        return *buffer[(head + index) % capacity];
    }

    size_t getSize() const { return size; }
    size_t getCapacity() const { return capacity; }
    bool isEmpty() const { return size == 0; }
    bool isFull() const { return size == capacity; }
};