#include "binarymanager.h"
#include <QFile>
#include <cstring>  // for strncpy, memcmp

BinaryManager::BinaryManager(const QString& filePath)
    : filePath_(filePath) {
}

bool BinaryManager::snapshotExists() const {
    return QFile::exists(filePath_);
}

// ============================================================
// WRITE: Save a stock snapshot to binary file
//
// Professor's approach (Udžbenik str. 65-66):
//   FILE* f = fopen("data.bin", "wb");
//   fwrite(&header, sizeof(Header), 1, f);
//   for (int i = 0; i < count; i++)
//       fwrite(&records[i], sizeof(Record), 1, f);
//   fclose(f);
//
// Our approach: same concept using QFile
// ============================================================
bool BinaryManager::saveSnapshot(const std::vector<SnapshotRecord>& records) {
    QFile file(filePath_);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    // Step 1: Build the header
    SnapshotHeader header;
    // Magic bytes — "WHSE" identifies this as our warehouse snapshot format
    // Like how PNG files start with specific bytes, or ZIP with "PK"
    header.magic[0] = 'W';
    header.magic[1] = 'H';
    header.magic[2] = 'S';
    header.magic[3] = 'E';
    header.version = 1;
    header.recordCount = static_cast<int>(records.size());
    header.timestamp = time(nullptr);  // current time

    // Step 2: Write header as raw bytes
    // This writes exactly sizeof(SnapshotHeader) bytes to the file
    // Professor equivalent: fwrite(&header, sizeof(Header), 1, f)
    file.write(reinterpret_cast<const char*>(&header), sizeof(SnapshotHeader));

    // Step 3: Write each record as raw bytes
    // Professor equivalent: fwrite(&records[i], sizeof(Record), 1, f)
    for (int i = 0; i < records.size(); i++) {
        file.write(reinterpret_cast<const char*>(&records[i]), sizeof(SnapshotRecord));
    }

    file.close();
    return true;
}

// ============================================================
// READ: Load a stock snapshot from binary file
//
// Professor's approach:
//   fread(&header, sizeof(Header), 1, f);
//   for (int i = 0; i < header.count; i++)
//       fread(&records[i], sizeof(Record), 1, f);
//
// Key validation: check magic bytes to confirm it's our file format
// ============================================================
bool BinaryManager::loadSnapshot(SnapshotHeader& header,
                                 std::vector<SnapshotRecord>& records) {
    QFile file(filePath_);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    // Step 1: Read the header
    file.read(reinterpret_cast<char*>(&header), sizeof(SnapshotHeader));

    // Step 2: Validate magic bytes — make sure this is actually our file format
    // Without this check, opening a random .bin file would produce garbage data
    if (header.magic[0] != 'W' || header.magic[1] != 'H' ||
        header.magic[2] != 'S' || header.magic[3] != 'E') {
        file.close();
        return false;  // not our file format
    }

    // Step 3: Validate version
    if (header.version != 1) {
        file.close();
        return false;  // unsupported version
    }

    // Step 4: Read all records
    records.clear();
    for (int i = 0; i < header.recordCount; i++) {
        SnapshotRecord record;
        file.read(reinterpret_cast<char*>(&record), sizeof(SnapshotRecord));
        records.push_back(record);
    }

    file.close();
    return true;
}