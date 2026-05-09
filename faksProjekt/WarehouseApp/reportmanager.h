#pragma once

#include <QString>
#include <vector>
#include "Product.h"
#include "Category.h"

class ReportManager {
public:
    // Builds an HTML inventory report grouped by category.
    static QString generateInventoryReport(const std::vector<Product>& products,
                                           const std::vector<Category>& categories);

    // Renders the HTML to a PDF file via QPrinter + QTextDocument.
    // Returns true if the file was written successfully.
    static bool exportToPDF(const QString& html, const QString& filePath);
};
