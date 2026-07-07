#include "reportmanager.h"
#include <QPrinter>
#include <QTextDocument>
#include <QDateTime>
#include <QPageSize>
#include <QPageLayout>
#include <QFile>



static QString money(double v) {
    return QString::number(v, 'f', 2);
}

static QString cell(const QString& content, const QString& align = "left",
                    const QString& extraStyle = "") {
    return "<td align=\"" + align + "\"" +
           (extraStyle.isEmpty() ? "" : " style=\"" + extraStyle + "\"") +
           ">" + content + "</td>";
}



QString ReportManager::generateInventoryReport(const std::vector<Product>& products,
                                               const std::vector<Category>& categories) {
    QString body;
    double grandTotal = 0.0;

    for (const Category& cat : categories) {

        // svi produkti u toj kategoriji
        std::vector<const Product*> catProducts;
        for (const Product& p : products) {
            if (p.getCategory().getId() == cat.getId())
                catProducts.push_back(&p);
        }
        if (catProducts.empty()) continue;

        double subtotal = 0.0;


        body += "<table width=\"100%\" border=\"1\" cellspacing=\"0\" cellpadding=\"5\" "
                "style=\"border-collapse:collapse; margin-bottom:18px;\">";


        body += "<tr style=\"background-color:#1a3a5c; color:#ffffff;\">"
                "<td colspan=\"4\" style=\"padding:7px 8px;\">"
                "<b>CATEGORY: " + QString::fromStdString(cat.getName()) +
                "</b> &nbsp;<span style=\"font-weight:normal; font-size:9pt;\">"
                "(" + QString::fromStdString(cat.getDescription()) + ")"
                "</span></td></tr>";


        body += "<tr style=\"background-color:#d8e8f8;\">"
                "<th align=\"left\"  style=\"padding:4px 8px;\">Product</th>"
                "<th align=\"right\" style=\"padding:4px 8px;\">Qty</th>"
                "<th align=\"right\" style=\"padding:4px 8px;\">Unit Price (&euro;)</th>"
                "<th align=\"right\" style=\"padding:4px 8px;\">Total Value (&euro;)</th>"
                "</tr>";


        for (int i = 0; i < static_cast<int>(catProducts.size()); i++) {
            const Product* p = catProducts[i];
            double tv = p->getTotalValue();
            subtotal += tv;

            QString rowBg = (i % 2 == 0) ? "#ffffff" : "#f4f8fd";
            body += "<tr style=\"background-color:" + rowBg + ";\">";
            body += cell(QString::fromStdString(p->getName()));
            body += cell(QString::number(p->getQuantity()), "right");
            body += cell(money(p->getPrice()),    "right");
            body += cell(money(tv),               "right");
            body += "</tr>";
        }


        body += "<tr style=\"background-color:#e4eff9;\">"
                "<td colspan=\"3\" align=\"right\" style=\"padding:5px 8px;\"><b>Subtotal:</b></td>"
                "<td align=\"right\" style=\"padding:5px 8px;\"><b>" + money(subtotal) + "</b></td>"
                "</tr>";

        body += "</table>";
        grandTotal += subtotal;
    }


    body += "<table width=\"100%\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\">"
            "<tr><td style=\"background-color:#1a3a5c; color:#ffffff; "
            "padding:10px 12px; text-align:right; font-size:13pt;\">"
            "<b>GRAND TOTAL: &euro;" + money(grandTotal) + "</b>"
            "</td></tr></table>";


    QString datetime = QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm");
    int count = static_cast<int>(products.size());

    return
        "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"/>"
        "<style>"
        "body { font-family: Arial, Helvetica, sans-serif; font-size: 11pt; margin: 0; }"
        "h1   { color: #1a3a5c; border-bottom: 2px solid #1a3a5c;"
        "       padding-bottom: 5px; margin-bottom: 4px; }"
        ".meta { color: #666; font-size: 9pt; margin-bottom: 20px; }"
        "</style></head><body>"
        "<h1>Warehouse Inventory Report</h1>"
        "<p class=\"meta\">Generated: " + datetime +
        " &nbsp;|&nbsp; Total products: " + QString::number(count) + "</p>" +
        body +
        "</body></html>";
}

// pdf export

bool ReportManager::exportToPDF(const QString& html, const QString& filePath) {
    QPrinter printer(QPrinter::ScreenResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Portrait);
    printer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);

    QTextDocument doc;
    doc.setHtml(html);
    doc.setPageSize(printer.pageLayout().paintRectPixels(printer.resolution()).size());
    doc.print(&printer);

    return QFile::exists(filePath);
}
