#ifndef ORDERXMLMANAGER_H
#define ORDERXMLMANAGER_H

#include <QString>
#include <vector>
#include "Order.h"

class OrderXmlManager {
public:
    OrderXmlManager(const QString& filePath);

    std::vector<Order> loadOrders();
    void addOrder(const Order& order);
    void updateOrder(const Order& order);
    void deleteOrder(int id);
    int getNextId();

private:
    QString filePath_;

    void ensureFileExists();
};

#endif // ORDERXMLMANAGER_H
