#include "warehouseexport.h"
#include <QJsonObject>
#include <QDateTime>

// CSV helperi


static QString csvQuote(const QString& s) {
    if (s.contains(',') || s.contains('"') || s.contains('\n') || s.contains('\r'))
        return '"' + QString(s).replace('"', "\"\"") + '"';
    return s;
}



QString exportToCSV(const QJsonArray& products) {
    QString out;
    out.reserve(products.size() * 80);

    out += "ID,Name,Price,Quantity,Value,Category,Supplier\r\n";

    for (const QJsonValue& v : products) {
        const QJsonObject obj = v.toObject();
        out += QString("%1,%2,%3,%4,%5,%6,%7\r\n")
                   .arg(obj["id"].toInt())
                   .arg(csvQuote(obj["name"].toString()))
                   .arg(obj["price"].toDouble(), 0, 'f', 2)
                   .arg(obj["quantity"].toInt())
                   .arg(obj["value"].toDouble(), 0, 'f', 2)
                   .arg(csvQuote(obj["category"].toString()))
                   .arg(csvQuote(obj["supplier"].toString()));
    }

    return out;
}



QString exportToHTML(const QJsonArray& products) {
    const QString generated =
        QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    QString rows;
    rows.reserve(products.size() * 120);

    for (const QJsonValue& v : products) {
        const QJsonObject obj = v.toObject();
        rows += QString(
            "  <tr>"
            "<td>%1</td>"
            "<td>%2</td>"
            "<td style='text-align:right'>%3</td>"
            "<td style='text-align:right'>%4</td>"
            "<td style='text-align:right'>%5</td>"
            "<td>%6</td>"
            "<td>%7</td>"
            "</tr>\n")
                    .arg(obj["id"].toInt())
                    .arg(obj["name"].toString().toHtmlEscaped())
                    .arg(obj["price"].toDouble(), 0, 'f', 2)
                    .arg(obj["quantity"].toInt())
                    .arg(obj["value"].toDouble(), 0, 'f', 2)
                    .arg(obj["category"].toString().toHtmlEscaped())
                    .arg(obj["supplier"].toString().toHtmlEscaped());
    }

    return QString(R"(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <title>Product Report</title>
  <style>
    body  { font-family: Arial, sans-serif; font-size: 13px; margin: 24px; }
    h2    { color: #2c3e50; }
    p.gen { color: gray; font-size: 11px; margin-top: 0; }
    table { border-collapse: collapse; width: 100%; }
    th    { background: #2980b9; color: #fff; padding: 7px 10px; text-align: left; }
    td    { border: 1px solid #ddd; padding: 6px 10px; }
    tr:nth-child(even) td { background: #f4f8fb; }
    tfoot td { font-weight: bold; background: #eaf0f6; }
  </style>
</head>
<body>
  <h2>Product Report</h2>
  <p class="gen">Generated: %1 &mdash; %2 products</p>
  <table>
    <thead>
      <tr>
        <th>ID</th><th>Name</th><th>Price</th><th>Qty</th>
        <th>Value</th><th>Category</th><th>Supplier</th>
      </tr>
    </thead>
    <tbody>
%3
    </tbody>
  </table>
</body>
</html>
)")
        .arg(generated)
        .arg(products.size())
        .arg(rows);
}



QString DataFormatter::formatReport(const QJsonArray& products, const QString& format) {
    const QString fmt = format.toLower().trimmed();
    if (fmt == "csv")
        return exportToCSV(products);
    if (fmt == "html")
        return exportToHTML(products);
    return {};
}
