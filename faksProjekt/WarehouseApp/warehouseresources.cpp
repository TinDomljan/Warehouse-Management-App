#include "warehouseresources.h"

static void initWarehouseResources() {
    Q_INIT_RESOURCE(warehouse_icons);
}
Q_CONSTRUCTOR_FUNCTION(initWarehouseResources)

QPixmap loadWarehouseIcon(const QString& name) {
    return QPixmap(QStringLiteral(":/icons/") + name + QStringLiteral(".png"));
}
