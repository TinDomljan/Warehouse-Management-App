#include <QCoreApplication>
#include <QDebug>
#include "warehouseserver.h"
#include "databasemanager.h"

static constexpr quint16 SERVER_PORT = 23117;

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);


    QString dbPath = QCoreApplication::applicationDirPath() + "/warehouse.db";
    if (!DatabaseManager::instance().open(dbPath)) {
        qCritical() << "[Server] Failed to open database at" << dbPath
                    << ":" << DatabaseManager::instance().lastError();
        return 1;
    }
    qInfo() << "[Server] Database opened:" << dbPath;

    WarehouseServer server; //vec se ovdje binda
    if (!server.listen(QHostAddress::Any, SERVER_PORT)) {
        qCritical() << "[Server] Failed to listen on port" << SERVER_PORT;
        return 1;
    }

    qInfo() << "[Server] WarehouseServer listening on port" << SERVER_PORT;
    qInfo() << "[Server] Protocol:";
    qInfo() << "[Server]   <barcode>\\n          -> JSON product info";
    qInfo() << "[Server]   FILE:<name>:<size>\\n  -> receive binary file";

    return app.exec();
}
