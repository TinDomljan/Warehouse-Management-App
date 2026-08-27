#include "warehouseutils.h"
#include <QLocale>

namespace WarehouseUtils {

QString formatCurrency(double value) {

    static const QLocale locale(QLocale::English, QLocale::UnitedStates);
    return u'€' + locale.toString(value, 'f', 2);
}

bool validateBarcode(const QString& code) {
    if (code.length() < 8 || code.length() > 13)
        return false;
    for (const QChar ch : code) {
        if (!ch.isDigit())
            return false;
    }
    return true;
}

double StockCalculator::calculateTurnoverRate(double costOfGoodsSold, double avgInventory) {
    if (avgInventory == 0.0)
        return 0.0;
    return costOfGoodsSold / avgInventory;
}

int StockCalculator::calculateReorderPoint(int dailySales, int leadTimeDays) {
    return dailySales * leadTimeDays;
}

int TransferProgress::percent(qint64 received, qint64 total) {
    if (total <= 0)
        return 0;
    int pct = static_cast<int>(received * 100 / total);
    if (pct > 100)
        pct = 100;
    return pct;
}

QString TransferProgress::formatBytes(qint64 bytes) {
    if (bytes < 1024)
        return QString("%1 B").arg(bytes);
    if (bytes < 1024 * 1024)
        return QString("%1 KB").arg(bytes / 1024.0, 0, 'f', 2);
    return QString("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 2);
}

QString TransferProgress::progressText(qint64 received, qint64 total) {
    return QString("%1% — %2 / %3")
        .arg(percent(received, total))
        .arg(formatBytes(received))
        .arg(formatBytes(total));
}

}
