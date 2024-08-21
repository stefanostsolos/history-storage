#pragma once
#include <ctime>
#include <string>
#include <memory>

class HistoryEntry
{
public:
    virtual ~HistoryEntry() = default;
    virtual size_t getSize() const = 0;
    virtual std::time_t getTimestamp() const = 0;
    virtual std::unique_ptr<HistoryEntry> clone() const = 0;
};

template <typename T>
class TypedHistoryEntry : public HistoryEntry
{
private:
    std::time_t timestamp;
    T value;

public:
    TypedHistoryEntry(std::time_t ts, T val) : timestamp(ts), value(val) {}

    size_t getSize() const override
    {
        return sizeof(timestamp) + sizeof(T);
    }

    std::time_t getTimestamp() const override
    {
        return timestamp;
    }

    T getValue() const { return value; }

    std::unique_ptr<HistoryEntry> clone() const override
    {
        return std::make_unique<TypedHistoryEntry<T>>(*this);
    }
};

// Specialization for std::string
template <>
class TypedHistoryEntry<std::string> : public HistoryEntry
{
private:
    std::time_t timestamp;
    std::string value;
    static const size_t STRING_SIZE = 50;

public:
    TypedHistoryEntry(std::time_t ts, std::string val)
        : timestamp(ts), value(val.substr(0, STRING_SIZE))
    {
        value.resize(STRING_SIZE, ' '); // Pad with spaces if shorter than 50 chars
    }

    size_t getSize() const override
    {
        return sizeof(timestamp) + STRING_SIZE;
    }

    std::time_t getTimestamp() const override
    {
        return timestamp;
    }

    std::string getValue() const { return value; }

    std::unique_ptr<HistoryEntry> clone() const override
    {
        return std::make_unique<TypedHistoryEntry<std::string>>(*this);
    }
};

// Explicit instantiations for common types
extern template class TypedHistoryEntry<double>;
extern template class TypedHistoryEntry<int>;
extern template class TypedHistoryEntry<bool>;
extern template class TypedHistoryEntry<std::string>;