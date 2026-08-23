#pragma once
#include <string>
#include <vector>
#include <ctime>

struct LogEntry {
    int id;
    std::string username;
    std::string action;
    std::string target;
    time_t timestamp;
};

class ActivityLog {
public:
    ActivityLog();


    void addEntry(const std::string& username, const std::string& action,
        const std::string& target);
    void deleteEntry(int id);


    std::vector<LogEntry> getAllEntries() const;
    std::vector<LogEntry> getEntriesByUser(const std::string& username) const;
    std::vector<LogEntry> getEntriesBetween(time_t from, time_t to) const;
    int getEntryCount() const;

    std::string formatEntry(const LogEntry& entry) const;
    std::string toString() const;

private:
    std::vector<LogEntry> entries_;
    int nextId_;
};