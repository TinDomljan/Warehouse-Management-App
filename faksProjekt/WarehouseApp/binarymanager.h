#ifndef BINARYMANAGER_H
#define BINARYMANAGER_H

#include <QString>
#include <vector>
#include <ctime>

// Fixed-size record for binary file — each product snapshot is exactly this size
// Using fixed char arrays instead of std::string because binary files need
// predictable sizes — we need to know exactly how many bytes each record takes
//
// Professor equivalent (Udžbenik str. 64-65):
// He uses structs with fixed-size fields written with fwrite/fread
struct SnapshotRecord {
    int productId;
    char productName[50];   // fixed 50 chars — padded with zeros if shorter
    int quantity;
    double price;
    double totalValue;      // quantity * price at time of snapshot
};

// File header — appears once at the beginning of the file
// Contains metadata about the file itself
struct SnapshotHeader {
    char magic[4];          // "WHSE" — identifies this as our file format
    int version;            // file format version (1 for now)
    int recordCount;        // how many records follow
    time_t timestamp;       // when the snapshot was taken
};

class BinaryManager {
public:
    BinaryManager(const QString& filePath);

    // Write a snapshot — saves header + all records
    bool saveSnapshot(const std::vector<SnapshotRecord>& records);

    // Read a snapshot — returns header info and all records
    bool loadSnapshot(SnapshotHeader& header, std::vector<SnapshotRecord>& records);

    // Check if a snapshot file exists
    bool snapshotExists() const;

private:
    QString filePath_;
};

#endif // BINARYMANAGER_H