#ifndef JSONMANAGER_H
#define JSONMANAGER_H

#include <QString>
#include <vector>
#include "ActivityLog.h"

// JsonManager handles all JSON file operations for activity log data
// Uses Qt's built-in QJsonDocument/QJsonArray/QJsonObject
//
// Professor's equivalent: System.JSON (TJSONObject, TJSONArray) — Udžbenik str. 59-63
// Our equivalent: QJsonDocument + QJsonArray + QJsonObject
//
// JSON structure in the file:
// {
//   "entries": [
//     { "id": 1, "username": "admin", "action": "LOGIN", "target": "System", "timestamp": 1234567890 },
//     { "id": 2, "username": "admin", "action": "ADD_PRODUCT", "target": "Laptop", "timestamp": 1234567891 }
//   ]
// }

class JsonManager {
public:
    JsonManager(const QString& filePath);

    // CRUD operations
    void saveLog(const ActivityLog& log);             // Create/Write — save entire log
    std::vector<LogEntry> loadEntries();               // Read — load all entries
    void addEntry(const LogEntry& entry);              // Create — append one entry
    void deleteEntry(int id);                          // Delete — remove by ID
    void clearLog();                                   // Delete all entries

private:
    QString filePath_;
    void ensureFileExists();
};

#endif // JSONMANAGER_H