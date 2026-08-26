#ifndef JSONMANAGER_H
#define JSONMANAGER_H

#include <QString>
#include <vector>
#include "ActivityLog.h"



class JsonManager {
public:
    JsonManager(const QString& filePath);


    void saveLog(const ActivityLog& log);
    std::vector<LogEntry> loadEntries();
    void addEntry(const LogEntry& entry);
    void updateEntry(const LogEntry& entry);
    void deleteEntry(int id);
    void clearLog();

private:
    QString filePath_;
    void ensureFileExists();
};

#endif // JSONMANAGER_H