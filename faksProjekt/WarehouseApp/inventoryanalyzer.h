#pragma once
#include <QObject>
#include <QRunnable>
#include <QMutex>
#include <vector>
#include <mutex>
#include "Product.h"


class InventoryAnalyzer : public QObject, public QRunnable {
    Q_OBJECT

public:

    InventoryAnalyzer(const std::vector<Product>* products,
                      int startIdx, int endIdx,
                      int threadId,
                      QMutex*     counterMutex,
                      int*        processedCount,
                      std::mutex* fileMutex,
                      QObject*    parent = nullptr);

    void run() override;


    bool    hasResults    = false;
    int     lowStockCount = 0;
    double  totalValue    = 0.0;
    Product mostExpensive;
    int     rangeStart    = 0;
    int     rangeEnd      = 0;
    int     threadId      = 0;

signals:
    void finished(int threadId);

private:
    const std::vector<Product>* products_;
    int startIdx_;
    int endIdx_;


    QMutex*     counterMutex_;
    int*        processedCount_;
    std::mutex* fileMutex_;
};
