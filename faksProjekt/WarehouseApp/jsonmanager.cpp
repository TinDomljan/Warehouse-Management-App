#include "jsonmanager.h"


#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

JsonManager::JsonManager(const QString& filePath)
    : filePath_(filePath) {
    ensureFileExists();
}


void JsonManager::ensureFileExists() {
    QFile file(filePath_);
    if (!file.exists()) {

        QJsonObject root;
        root["entries"] = QJsonArray();

        QJsonDocument doc(root);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(doc.toJson(QJsonDocument::Indented));
            file.close();
        }
    }
}


std::vector<LogEntry> JsonManager::loadEntries() {
    std::vector<LogEntry> entries;


    QFile file(filePath_);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return entries;
    }
    QByteArray rawData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(rawData);
    if (doc.isNull()) {
        return entries;
    }


    QJsonObject root = doc.object();
    QJsonArray entriesArray = root["entries"].toArray();

    // loopamo kroz stablo
    for (int i = 0; i < entriesArray.size(); i++) {


        QJsonObject item = entriesArray[i].toObject();

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



void JsonManager::saveLog(const ActivityLog& log) {

    QJsonArray entriesArray;


    std::vector<LogEntry> entries = log.getAllEntries();

    for (int i = 0; i < entries.size(); i++) {


        QJsonObject item;
        item["id"] = entries[i].id;
        item["username"] = QString::fromStdString(entries[i].username);
        item["action"] = QString::fromStdString(entries[i].action);
        item["target"] = QString::fromStdString(entries[i].target);
        item["timestamp"] = static_cast<double>(entries[i].timestamp);


        entriesArray.append(item);
    }

    // Wrapamo u root objekt
    QJsonObject root;
    root["entries"] = entriesArray;


    QJsonDocument doc(root);
    QFile file(filePath_);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    }
}



void JsonManager::addEntry(const LogEntry& entry) {

    QFile file(filePath_);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    QJsonObject root = doc.object();
    QJsonArray entriesArray = root["entries"].toArray();

    // kreiramo novi json objekt
    QJsonObject item;
    item["id"] = entry.id;
    item["username"] = QString::fromStdString(entry.username);
    item["action"] = QString::fromStdString(entry.action);
    item["target"] = QString::fromStdString(entry.target);
    item["timestamp"] = static_cast<double>(entry.timestamp);

    // appendamo
    entriesArray.append(item);
    root["entries"] = entriesArray;


    QJsonDocument newDoc(root);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        file.write(newDoc.toJson(QJsonDocument::Indented));
        file.close();
    }
}

void JsonManager::updateEntry(const LogEntry& entry) {
    QFile file(filePath_);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    QJsonObject root = doc.object();
    QJsonArray entriesArray = root["entries"].toArray();

    // nađemo objekt s tim id-om i prepišemo polja
    for (int i = 0; i < entriesArray.size(); i++) {
        QJsonObject item = entriesArray[i].toObject();
        if (item["id"].toInt() == entry.id) {
            item["username"] = QString::fromStdString(entry.username);
            item["action"] = QString::fromStdString(entry.action);
            item["target"] = QString::fromStdString(entry.target);
            item["timestamp"] = static_cast<double>(entry.timestamp);
            entriesArray[i] = item;
        }
    }

    root["entries"] = entriesArray;

    QJsonDocument newDoc(root);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        file.write(newDoc.toJson(QJsonDocument::Indented));
        file.close();
    }
}

void JsonManager::deleteEntry(int id) {
    QFile file(filePath_);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    QJsonObject root = doc.object();
    QJsonArray entriesArray = root["entries"].toArray();

    // novi array bez obrisanog
    QJsonArray newArray;
    for (int i = 0; i < entriesArray.size(); i++) {
        QJsonObject item = entriesArray[i].toObject();
        if (item["id"].toInt() != id) {
            newArray.append(item);
        }
    }

    root["entries"] = newArray;

    QJsonDocument newDoc(root);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        file.write(newDoc.toJson(QJsonDocument::Indented));
        file.close();
    }
}


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