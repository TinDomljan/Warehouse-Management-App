#include "warehouseserver.h"
#include "clienthandler.h"
#include "udpprotocol.h"
#include "databasemanager.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <cstring>

// konstruktor

WarehouseServer::WarehouseServer(QObject* parent) : QTcpServer(parent) {
    udpSocket_ = new QUdpSocket(this);

    if (!udpSocket_->bind(QHostAddress::Any, UDP_PORT)) {
        qWarning() << "[Server] Failed to bind UDP socket on port" << UDP_PORT;
    } else { //nakon sto se pozove signal readyread on poziva slot onudpdatagramreceived
        connect(udpSocket_, &QUdpSocket::readyRead,
                this, &WarehouseServer::onUdpDatagramReceived);
        qInfo() << "[Server] UDP socket listening on port" << UDP_PORT;
    }
}

// tcp

void WarehouseServer::incomingConnection(qintptr socketDescriptor) {
    qInfo() << "[Server] New TCP connection, descriptor:" << socketDescriptor;
    new ClientHandler(socketDescriptor, this);
}

// udp

void WarehouseServer::onUdpDatagramReceived() {
    while (udpSocket_->hasPendingDatagrams()) { //petlja zbog vise datagrama
        QByteArray  datagram(udpSocket_->pendingDatagramSize(), Qt::Uninitialized); //priprema buffera, koliko je velik datagram
        //izlazni parametri
        QHostAddress sender;
        quint16      senderPort = 0;

        udpSocket_->readDatagram(datagram.data(), datagram.size(),
                                 &sender, &senderPort);

        if (datagram.trimmed() == "STATUS") {
            handleStatusQuery(sender, senderPort);
        } else if (datagram.startsWith("LOG:")) {
            handleLogPacket(datagram.mid(4), sender, senderPort);
        } else {
            qWarning() << "[UDP] Unknown datagram from"
                       << sender.toString() << ":" << senderPort;
        }
    }
}

void WarehouseServer::handleStatusQuery(const QHostAddress& sender, quint16 port) {
    std::vector<Product> products = DatabaseManager::instance().getAllProducts();

    int    total    = static_cast<int>(products.size());
    int    lowStock = 0;
    double value    = 0.0;
    for (const Product& p : products) {
        value += p.getTotalValue();
        if (p.isLowStock(10)) ++lowStock; //sve ispod praga 10
    }

    QJsonObject json;
    json["totalProducts"] = total;
    json["lowStockCount"] = lowStock;
    json["totalValue"]    = qRound(value * 100.0) / 100.0;

    QByteArray response = QJsonDocument(json).toJson(QJsonDocument::Compact);
    udpSocket_->writeDatagram(response, sender, port);

    qInfo() << "[UDP] STATUS -> products:" << total
            << "lowStock:" << lowStock << "value:" << value;
}



void WarehouseServer::handleLogPacket(const QByteArray& payload,
                                      const QHostAddress& sender, quint16 port) {
    constexpr int SZ = static_cast<int>(sizeof(LogPacket)); //136

    if (payload.isEmpty() || payload.size() % SZ != 0) { //ako nije dijelivo sa 136
        qWarning() << "[UDP] Invalid LOG payload size:" << payload.size()
                   << "(expected multiple of" << SZ << ")";
        return;
    }

    const int count = payload.size() / SZ; // 2720/136 = 20, zapisi

    QFile logFile("server_log.txt");
    if (!logFile.open(QIODevice::Append | QIODevice::Text)) {
        qWarning() << "[UDP] Cannot open server_log.txt for writing";
        return;
    }

    QTextStream out(&logFile);
    out << "=== UDP log from " << sender.toString() << ":" << port
        << "  at " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
        << "  (" << count << " entries) ===\n";

    //skacemo po 136, payload.consData pointer na bajt broj 0,
    for (int i = 0; i < count; ++i) {
        const auto* pkt = reinterpret_cast<const LogPacket*>(
            payload.constData() + i * SZ);

        //sigurno kopiranje stringova
        char username[33] = {};
        char action[33]   = {};
        char target[65]   = {};
        std::memcpy(username, pkt->username, 32);
        std::memcpy(action,   pkt->action,   32);
        std::memcpy(target,   pkt->target,   64);

        QDateTime ts = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(pkt->timestamp));
        out << "  [" << ts.toString("yyyy-MM-dd hh:mm:ss") << "] "
            << username << " | " << action << " | " << target << "\n";
    }
    out << "\n";

    qInfo() << "[UDP] Saved" << count << "log entries to server_log.txt";
}
