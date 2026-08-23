#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include <QObject>
#include <QTcpSocket>

class ClientHandler : public QObject {
    Q_OBJECT
public:
    explicit ClientHandler(qintptr socketDescriptor, QObject* parent = nullptr);

private slots:
    void onReadyRead();
    void onDisconnected();

private:
    QTcpSocket* socket_;


    QByteArray  buffer_;


    bool       receivingFile_ = false;
    QString    fileName_;
    qint64     fileSize_      = 0;
    QByteArray fileBuffer_;

    void handleCommand(const QByteArray& line);
    void handleBarcode(const QString& barcode);
    void processFileData();
};

#endif // CLIENTHANDLER_H
