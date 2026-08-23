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

} //WarehouseUtils namespace
