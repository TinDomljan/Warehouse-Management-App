#include "inventoryanalyzer.h"
#include <QMutexLocker>
#include <fstream>
#include <iomanip>
#include <cmath>

InventoryAnalyzer::InventoryAnalyzer(const std::vector<Product>* products,
                                     int startIdx, int endIdx,
                                     int id,
                                     QMutex*     counterMutex,
                                     int*        processedCount,
                                     std::mutex* fileMutex,
                                     QObject*    parent)
    : QObject(parent),
      QRunnable(),
      products_(products),
      startIdx_(startIdx),
      endIdx_(endIdx),
      counterMutex_(counterMutex),
      processedCount_(processedCount),
      fileMutex_(fileMutex)
{
    threadId   = id;
    rangeStart = startIdx;
    rangeEnd   = endIdx;
    setAutoDelete(false);
}

void InventoryAnalyzer::run() {
    if (startIdx_ < endIdx_) {
        lowStockCount = 0;
        totalValue    = 0.0;
        mostExpensive = (*products_)[startIdx_];

        for (int i = startIdx_; i < endIdx_; ++i) {
            const Product& p = (*products_)[i];


            {
                QMutexLocker locker(counterMutex_);
                ++(*processedCount_);
            }

            if (p.getQuantity() < 10)
                ++lowStockCount;
            totalValue += p.getTotalValue();
            if (p.getPrice() > mostExpensive.getPrice())
                mostExpensive = p;


            {
                volatile double acc = 0.0;
                for (int j = 1; j <= 800000; ++j) {
                    acc += std::sqrt(p.getPrice() * j) / (j + 1.0);
                }
            }
        }
        hasResults = true;
    }


    {
        std::lock_guard<std::mutex> lock(*fileMutex_);

        std::ofstream file("analysis_result.txt", std::ios::app);
        if (file.is_open()) {
            file << "--- Thread " << threadId << " ---\n";
            file << "  Index range     : [" << rangeStart << ", " << rangeEnd - 1 << "]\n";
            if (hasResults) {
                file << "  Low-stock items : " << lowStockCount << "\n";
                file << "  Segment value   : $"
                     << std::fixed << std::setprecision(2) << totalValue << "\n";
                file << "  Most expensive  : "
                     << mostExpensive.getName()
                     << "  ($" << std::fixed << std::setprecision(2)
                     << mostExpensive.getPrice() << ")\n";
            } else {
                file << "  (empty segment — no products assigned)\n";
            }
            file << "\n";
        }
    }

    emit finished(threadId);
}
