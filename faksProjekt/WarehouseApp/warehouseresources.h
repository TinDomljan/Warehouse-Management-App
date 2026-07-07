#pragma once

#include <QPixmap>
#include <QString>


#if defined(_WIN32) || defined(_WIN64)
#  ifdef WAREHOUSERESOURCES_EXPORTS
#    define WAREHOUSERESOURCES_API __declspec(dllexport)
#  else
#    define WAREHOUSERESOURCES_API __declspec(dllimport)
#  endif
#else
#  define WAREHOUSERESOURCES_API __attribute__((visibility("default")))
#endif


WAREHOUSERESOURCES_API QPixmap loadWarehouseIcon(const QString& name);
