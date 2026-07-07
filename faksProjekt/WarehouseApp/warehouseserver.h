#ifndef WAREHOUSESERVER_H
#define WAREHOUSESERVER_H

#include <QTcpServer>
#include <QUdpSocket>
#include <QHostAddress>

class WarehouseServer : public QTcpServer {
    Q_OBJECT
public:
    explicit WarehouseServer(QObject* parent = nullptr);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onUdpDatagramReceived();

private:
    QUdpSocket* udpSocket_;

    void handleStatusQuery(const QHostAddress& sender, quint16 port);
    void handleLogPacket(const QByteArray& payload,
                         const QHostAddress& sender, quint16 port);
};

#endif // WAREHOUSESERVER_H
