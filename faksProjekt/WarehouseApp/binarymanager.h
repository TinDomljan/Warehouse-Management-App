#ifndef BINARYMANAGER_H
#define BINARYMANAGER_H

#include <QString>
#include <QtGlobal>
#include <vector>
#include <ctime>


struct AnalysisSnapshotRecord {
    int    threadId;
    int    rangeStart;
    int    rangeEnd;
    int    lowStockCount;
    double totalValue;
    int    mostExpensiveProductId;
    char   mostExpensiveProductName[50];
    double mostExpensiveProductPrice;
    qint64 timestamp;
};

// zaglavlje
struct SnapshotHeader {
    char magic[4];
    int version;            // za sada 1
    int recordCount;
    time_t timestamp;
};

class BinaryManager {
public:
    BinaryManager(const QString& filePath);


    bool saveSnapshot(const std::vector<AnalysisSnapshotRecord>& records);


    bool loadSnapshot(SnapshotHeader& header, std::vector<AnalysisSnapshotRecord>& records);


    bool snapshotExists() const;

private:
    QString filePath_;
};

#endif