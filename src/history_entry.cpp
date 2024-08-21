#include "history_entry.hpp"

template class TypedHistoryEntry<double>;
template class TypedHistoryEntry<int>;
template class TypedHistoryEntry<bool>;
template class TypedHistoryEntry<std::string>;

std::string getEntryTypeName(const HistoryEntry *entry)
{
    if (dynamic_cast<const TypedHistoryEntry<double> *>(entry))
        return "double";
    if (dynamic_cast<const TypedHistoryEntry<int> *>(entry))
        return "int";
    if (dynamic_cast<const TypedHistoryEntry<bool> *>(entry))
        return "bool";
    if (dynamic_cast<const TypedHistoryEntry<std::string> *>(entry))
        return "string";
    return "Unknown";
}