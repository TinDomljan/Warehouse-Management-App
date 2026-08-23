#pragma once

#include <QString>

namespace WarehouseUtils {

// Returns value formatted as '€1,234.56' (EN-US thousands separator, 2 d.p.)
QString formatCurrency(double value);

// Returns true if code is purely numeric and 8–13 digits long (EAN-8 to EAN-13).
bool validateBarcode(const QString& code);

class StockCalculator {
public:
    // Inventory turnover rate = COGS / average inventory value.
    // Returns 0 if avgInventory is zero to avoid division by zero.
    static double calculateTurnoverRate(double costOfGoodsSold, double avgInventory);

    // Reorder point = daily sales * lead-time days.
    static int calculateReorderPoint(int dailySales, int leadTimeDays);
};

} // namespace WarehouseUtils
