#include "jsonmanager.h"

// Qt JSON classes — our equivalents of the professor's TJSONObject, TJSONArray
#include <QJsonDocument>   // the whole JSON document (like QDomDocument for XML)
#include <QJsonObject>     // a JSON object { "key": "value" }
#include <QJsonArray>      // a JSON array [ item1, item2, item3 ]
#include <QFile>

JsonManager::JsonManager(const QString& filePath)
    : filePath_(filePath) {
    ensureFileExists();
}

// Create empty JSON file if it doesn't exist
void JsonManager::ensureFileExists() {
    QFile file(filePath_);
    if (!file.exists()) {
        // Create a minimal JSON structure: { "entries": [] }
        QJsonObject root;
        root["entries"] = QJsonArray();

        QJsonDocument doc(root);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(doc.toJson(QJsonDocument::Indented));
            file.close();
        }
    }
}

// ============================================================
// READ: Load all entries from the JSON file
//
// Professor's approach (Udžbenik str. 60-61):
//   TJSONObject* json = (TJSONObject*)TJSONObject::ParseJSONValue(jsonString);
//   TJSONArray* arr = (TJSONArray*)json->GetValue("entries");
//   for (int i = 0; i < arr->Count(); i++) {
//       TJSONObject* item = (TJSONObject*)arr->Items[i];
//       String username = item->GetValue("username")->Value();
//   }
//
// Our approach: same logic, different class names
// ============================================================
std::vector<LogEntry> JsonManager::loadEntries() {
    std::vector<LogEntry> entries;

    // Step 1: Read the file content
    QFile file(filePath_);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return entries;
    }
    QByteArray rawData = file.readAll();
    file.close();

    // Step 2: Parse the raw bytes into a JSON document
    // Professor equivalent: TJSONObject::ParseJSONValue(jsonString)
    QJsonDocument doc = QJsonDocument::fromJson(rawData);
    if (doc.isNull()) {
        return entries;  // invalid JSON
    }

    // Step 3: Get the root object and the "entries" array
    // Professor equivalent: json->GetValue("entries")
    QJsonObject root = doc.object();
    QJsonArray entriesArray = root["entries"].toArray();

    // Step 4: Loop through array items
    // Professor equivalent: for (i = 0; i < arr->Count(); i++)
    for (int i = 0; i < entriesArray.size(); i++) {
        // Each item is a JSON object like:
        // { "id": 1, "username": "admin", "action": "LOGIN", ... }
        QJsonObject item = entriesArray[i].toObject();

        // Step 5: Extract values from each object
        // Professor equivalent: item->GetValue("username")->Value()
        LogEntry entry;
        entry.id = item["id"].toInt();
        entry.username = item["username"].toString().toStdString();
        entry.action = item["action"].toString().toStdString();
        entry.target = item["target"].toString().toStdString();
        entry.timestamp = static_cast<time_t>(item["timestamp"].toDouble());

        entries.push_back(entry);
    }

    return entries;
}

// ============================================================
// WRITE: Save the entire activity log to JSON
// This replaces the file with the current state of the log
// ============================================================
void JsonManager::saveLog(const ActivityLog& log) {
    // Build the JSON structure in memory
    QJsonArray entriesArray;

    // Get all entries from the ActivityLog
    std::vector<LogEntry> entries = log.getAllEntries();

    for (int i = 0; i < entries.size(); i++) {
        // Create a JSON object for each entry
        // Professor equivalent: TJSONObject* item = new TJSONObject();
        //                       item->AddPair("username", entries[i].username);
        QJsonObject item;
        item["id"] = entries[i].id;
        item["username"] = QString::fromStdString(entries[i].username);
        item["action"] = QString::fromStdString(entries[i].action);
        item["target"] = QString::fromStdString(entries[i].target);
        item["timestamp"] = static_cast<double>(entries[i].timestamp);

        // Add to the array
        entriesArray.append(item);
    }

    // Wrap the array in a root object
    QJsonObject root;
    root["entries"] = entriesArray;

    // Write to file
    // QJsonDocument::Indented makes it human-readable (like XML with indentation)
    QJsonDocument doc(root);
    QFile file(filePath_);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    }
}

// ============================================================
// CREATE: Add a single entry to the existing JSON file
// Loads existing entries, appends the new one, saves everything
// ============================================================
void JsonManager::addEntry(const LogEntry& entry) {
    // Load existing file
    QFile file(filePath_);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    QJsonObject root = doc.object();
    QJsonArray entriesArray = root["entries"].toArray();

    // Create new JSON object for this entry
    QJsonObject item;
    item["id"] = entry.id;
    item["username"] = QString::fromStdString(entry.username);
    item["action"] = QString::fromStdString(entry.action);
    item["target"] = QString::fromStdString(entry.target);
    item["timestamp"] = static_cast<double>(entry.timestamp);

    // Append to array
    entriesArray.append(item);
    root["entries"] = entriesArray;

    // Save back
    QJsonDocument newDoc(root);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        file.write(newDoc.toJson(QJsonDocument::Indented));
        file.close();
    }
}

// ============================================================
// DELETE: Remove an entry by ID
// ============================================================
void JsonManager::deleteEntry(int id) {
    QFile file(filePath_);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    QJsonObject root = doc.object();
    QJsonArray entriesArray = root["entries"].toArray();

    // Build a new array without the deleted entry
    QJsonArray newArray;
    for (int i = 0; i < entriesArray.size(); i++) {
        QJsonObject item = entriesArray[i].toObject();
        if (item["id"].toInt() != id) {
            newArray.append(item);  // keep everything except the one with matching ID
        }
    }

    root["entries"] = newArray;

    QJsonDocument newDoc(root);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        file.write(newDoc.toJson(QJsonDocument::Indented));
        file.close();
    }
}

// ============================================================
// DELETE ALL: Clear the entire log
// ============================================================
void JsonManager::clearLog() {
    QJsonObject root;
    root["entries"] = QJsonArray();

    QJsonDocument doc(root);
    QFile file(filePath_);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    }
}