#pragma once

#include <QString>
#include <vector>
#include "Product.h"
#include "Category.h"

class ReportManager {
public:

    static QString generateInventoryReport(const std::vector<Product>& products,
                                           const std::vector<Category>& categories);

    static bool exportToPDF(const QString& html, const QString& filePath);
};
