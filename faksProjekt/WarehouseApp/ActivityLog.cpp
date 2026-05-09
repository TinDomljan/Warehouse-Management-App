#include "ActivityLog.h"

ActivityLog::ActivityLog() {
    nextId_ = 1;
}

void ActivityLog::addEntry(const std::string& username, const std::string& action,
    const std::string& target) {
    LogEntry entry;
    entry.id = nextId_;
    entry.username = username;
    entry.action = action;
    entry.target = target;
    entry.timestamp = time(nullptr);

    entries_.push_back(entry);
    nextId_++;
}

void ActivityLog::deleteEntry(int id) {
    for (int i = 0; i < entries_.size(); i++) {
        if (entries_[i].id == id) {
            entries_.erase(entries_.begin() + i);
            return;
        }
    }
}

std::vector<LogEntry> ActivityLog::getAllEntries() const {
    return entries_;
}

std::vector<LogEntry> ActivityLog::getEntriesByUser(const std::string& username) const {
    std::vector<LogEntry> result;
    for (int i = 0; i < entries_.size(); i++) {
        if (entries_[i].username == username) {
            result.push_back(entries_[i]);
        }
    }
    return result;
}

std::vector<LogEntry> ActivityLog::getEntriesBetween(time_t from, time_t to) const {
    std::vector<LogEntry> result;
    for (int i = 0; i < entries_.size(); i++) {
        if (entries_[i].timestamp >= from && entries_[i].timestamp <= to) {
            result.push_back(entries_[i]);
        }
    }
    return result;
}

int ActivityLog::getEntryCount() const {
    return entries_.size();
}

std::string ActivityLog::formatEntry(const LogEntry& entry) const {
    char timeBuffer[26];
    ctime_s(timeBuffer, sizeof(timeBuffer), &entry.timestamp);
    std::string timeStr(timeBuffer);
    // Remove the trailing newline that ctime_s adds
    if (!timeStr.empty() && timeStr.back() == '\n') {
        timeStr.pop_back();
    }

    return "[" + std::to_string(entry.id) + "] " +
        timeStr + " | " +
        entry.username + " | " +
        entry.action + " | " +
        entry.target;
}

std::string ActivityLog::toString() const {
    std::string result = "=== Activity Log (" + std::to_string(entries_.size()) + " entries) ===\n";
    for (int i = 0; i < entries_.size(); i++) {
        result += formatEntry(entries_[i]) + "\n";
    }
    return result;
}