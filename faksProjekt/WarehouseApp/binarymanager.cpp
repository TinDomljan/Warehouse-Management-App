#include "binarymanager.h"
#include <QFile>
#include <cstring>

BinaryManager::BinaryManager(const QString& filePath)
    : filePath_(filePath) {
}

bool BinaryManager::snapshotExists() const {
    return QFile::exists(filePath_);
}


bool BinaryManager::saveSnapshot(const std::vector<AnalysisSnapshotRecord>& records) {
    QFile file(filePath_);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }


    SnapshotHeader header;
    // whse
    header.magic[0] = 'W';
    header.magic[1] = 'H';
    header.magic[2] = 'S';
    header.magic[3] = 'E';
    header.version = 1;
    header.recordCount = static_cast<int>(records.size()); //koliko recorda zapisujemo
    header.timestamp = time(nullptr);

    //odakle i kolioko
    //reinterpret cast zbog Snapshotheade* i const char*
    file.write(reinterpret_cast<const char*>(&header), sizeof(SnapshotHeader));


    for (int i = 0; i < records.size(); i++) {
        file.write(reinterpret_cast<const char*>(&records[i]), sizeof(AnalysisSnapshotRecord));
    }

    file.close();
    return true;
}


bool BinaryManager::loadSnapshot(SnapshotHeader& header,
                                 std::vector<AnalysisSnapshotRecord>& records) {
    QFile file(filePath_);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    // citamo zaglavlje
    file.read(reinterpret_cast<char*>(&header), sizeof(SnapshotHeader));


    if (header.magic[0] != 'W' || header.magic[1] != 'H' ||
        header.magic[2] != 'S' || header.magic[3] != 'E') {
        file.close();
        return false;
    }


    if (header.version != 1) {
        file.close();
        return false;
    }


    records.clear();
    for (int i = 0; i < header.recordCount; i++) {
        AnalysisSnapshotRecord record;
        file.read(reinterpret_cast<char*>(&record), sizeof(AnalysisSnapshotRecord));
        records.push_back(record);
    }

    file.close();
    return true;
}