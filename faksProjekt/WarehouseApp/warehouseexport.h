#pragma once

#include <QString>
#include <QJsonArray>

#if defined(_WIN32) || defined(_WIN64)
#  ifdef WAREHOUSEEXPORT_EXPORTS
#    define WAREHOUSEEXPORT_API __declspec(dllexport)
#  else
#    define WAREHOUSEEXPORT_API __declspec(dllimport)
#  endif
#else
#  define WAREHOUSEEXPORT_API __attribute__((visibility("default")))
#endif


WAREHOUSEEXPORT_API QString exportToCSV(const QJsonArray& products);

WAREHOUSEEXPORT_API QString exportToHTML(const QJsonArray& products);



class WAREHOUSEEXPORT_API DataFormatter {
public:

    QString formatReport(const QJsonArray& products, const QString& format);
};
