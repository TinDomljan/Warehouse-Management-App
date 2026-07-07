#include "clienthandler.h"
#include "databasemanager.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDebug>

ClientHandler::ClientHandler(qintptr socketDescriptor, QObject* parent)
    : QObject(parent)
{
    socket_ = new QTcpSocket(this);
    socket_->setSocketDescriptor(socketDescriptor);

    connect(socket_, &QTcpSocket::readyRead,    this, &ClientHandler::onReadyRead);
    connect(socket_, &QTcpSocket::disconnected, this, &ClientHandler::onDisconnected);

    qInfo() << "[Server] Client connected from"
            << socket_->peerAddress().toString()
            << "port" << socket_->peerPort();
}

// Kad dode data

void ClientHandler::onReadyRead() {
    if (receivingFile_) {

        fileBuffer_ += socket_->readAll();
        processFileData();
        return;
    }

    buffer_ += socket_->readAll();


    while (true) {
        int nl = buffer_.indexOf('\n');
        if (nl == -1) break;                        // wait for more data

        QByteArray line = buffer_.left(nl).trimmed();
        buffer_ = buffer_.mid(nl + 1);

        if (line.isEmpty()) continue;

        handleCommand(line);

        if (receivingFile_) {

            fileBuffer_ = buffer_;
            buffer_.clear();
            processFileData();
            break;
        }
    }
}


void ClientHandler::handleCommand(const QByteArray& line) {
    QString cmd = QString::fromUtf8(line);

    if (cmd.startsWith("FILE:")) {

        QString rest      = cmd.mid(5);
        int     lastColon = rest.lastIndexOf(':');

        if (lastColon == -1) {
            socket_->write("ERROR:Invalid FILE format\n");
            return;
        }

        fileName_ = rest.left(lastColon);
        fileSize_ = rest.mid(lastColon + 1).toLongLong();
        fileBuffer_.clear();
        receivingFile_ = true;

        qInfo() << "[Server] Receiving file:" << fileName_
                << "expected bytes:" << fileSize_;
    } else {
        handleBarcode(cmd);
    }
}



void ClientHandler::handleBarcode(const QString& barcode) {
    std::vector<Product> results =
        DatabaseManager::instance().getProductsFiltered(barcode, "p.id ASC");

    QJsonObject json;
    if (results.empty()) {
        json["found"]   = false;
        json["barcode"] = barcode;
        json["message"] = QString("No product matching \"%1\"").arg(barcode);
    } else {
        const Product& p = results[0];
        json["found"]    = true;
        json["id"]       = p.getId();
        json["name"]     = QString::fromStdString(p.getName());
        json["price"]    = p.getPrice();
        json["quantity"] = p.getQuantity();
        json["value"]    = p.getTotalValue();
        json["category"] = QString::fromStdString(p.getCategory().getName());
        json["supplier"] = QString::fromStdString(p.getSupplier().getCompanyName());
    }

    QByteArray response = QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    socket_->write(response);

    qInfo() << "[Server] Query:" << barcode
            << "-> found:" << !results.empty();
}



void ClientHandler::processFileData() {
    if (fileBuffer_.size() < fileSize_)
        return;

    QFile file(fileName_);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(fileBuffer_.left(static_cast<qsizetype>(fileSize_)));
        file.close();
        socket_->write("OK:File received\n");
        qInfo() << "[Server] Saved file:" << fileName_
                << "bytes:" << fileSize_;
    } else {
        socket_->write("ERROR:Cannot save file\n");
        qWarning() << "[Server] Failed to save file:" << fileName_;
    }

    receivingFile_ = false;
    fileBuffer_.clear();
}



void ClientHandler::onDisconnected() {
    qInfo() << "[Server] Client disconnected";
    deleteLater();
}
