#pragma once

#include <QString>

namespace WarehouseUtils {

// Returns value formatted as '€1,234.56' (EN-US thousands separator, 2 d.p.)
QString formatCurrency(double value);

// Returns true if code is purely numeric and 8–13 digits long (EAN-8 to EAN-13).
bool validateBarcode(const QString& code);

class StockCalculator {
public:

    static double calculateTurnoverRate(double costOfGoodsSold, double avgInventory);


    static int calculateReorderPoint(int dailySales, int leadTimeDays);
};

class TransferProgress {
public:
    // 0-100; vraća 0 ako je total <= 0, ograničeno na najviše 100
    static int percent(qint64 received, qint64 total);

    // "845 B" / "12.3 KB" / "14.7 MB" — dvije decimale za KB i MB
    static QString formatBytes(qint64 bytes);

    // "42% — 6.2 MB / 14.7 MB"
    static QString progressText(qint64 received, qint64 total);
};

} // namespace WarehouseUtils
