#include "orderxmlmanager.h"

#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QTextStream>

OrderXmlManager::OrderXmlManager(const QString& filePath)
    : filePath_(filePath) {
    ensureFileExists();
}

void OrderXmlManager::ensureFileExists() {
    QFile file(filePath_);
    if (!file.exists()) {
        QDomDocument doc;

        QDomProcessingInstruction header = doc.createProcessingInstruction(
            "xml", "version=\"1.0\" encoding=\"UTF-8\"");
        doc.appendChild(header);

        QDomElement root = doc.createElement("orders");
        doc.appendChild(root);

        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            doc.save(stream, 4);
            file.close();
        }
    }
}

std::vector<Order> OrderXmlManager::loadOrders() {
    std::vector<Order> orders;

    QFile file(filePath_);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return orders;

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        return orders;
    }
    file.close();

    QDomElement root = doc.documentElement();

    QDomElement orderElem = root.firstChildElement("order");
    while (!orderElem.isNull()) {
        int id = orderElem.attribute("id").toInt();
        std::string customerName = orderElem.firstChildElement("customerName").text().toStdString();
        std::string orderDate = orderElem.firstChildElement("orderDate").text().toStdString();
        std::string status = orderElem.firstChildElement("status").text().toStdString();


        std::vector<OrderItem> items;
        QDomElement itemsElem = orderElem.firstChildElement("items");
        QDomElement itemElem = itemsElem.firstChildElement("item");
        while (!itemElem.isNull()) {
            int productId = itemElem.attribute("productId").toInt();
            std::string productName = itemElem.attribute("productName").toStdString();
            int quantity = itemElem.attribute("quantity").toInt();
            items.push_back(OrderItem(productId, productName, quantity));

            itemElem = itemElem.nextSiblingElement("item");
        }

        orders.push_back(Order(id, customerName, orderDate, status, items));

        orderElem = orderElem.nextSiblingElement("order");
    }

    return orders;
}

void OrderXmlManager::addOrder(const Order& order) {
    QFile file(filePath_);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QDomDocument doc;
    doc.setContent(&file);
    file.close();

    QDomElement root = doc.documentElement();

    QDomElement newOrder = doc.createElement("order");
    newOrder.setAttribute("id", order.getId());

    auto addTextElement = [&](const QString& tagName, const QString& value) {
        QDomElement elem = doc.createElement(tagName);
        QDomText textNode = doc.createTextNode(value);
        elem.appendChild(textNode);
        newOrder.appendChild(elem);
    };

    addTextElement("customerName", QString::fromStdString(order.getCustomerName()));
    addTextElement("orderDate", QString::fromStdString(order.getOrderDate()));
    addTextElement("status", QString::fromStdString(order.getStatus()));


    QDomElement itemsElem = doc.createElement("items");
    for (const auto& item : order.getItems()) {
        QDomElement itemElem = doc.createElement("item");
        itemElem.setAttribute("productId", item.productId);
        itemElem.setAttribute("productName", QString::fromStdString(item.productName));
        itemElem.setAttribute("quantity", item.quantity);
        itemsElem.appendChild(itemElem);
    }
    newOrder.appendChild(itemsElem);

    root.appendChild(newOrder);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        return;
    QTextStream stream(&file);
    doc.save(stream, 4);
    file.close();
}

void OrderXmlManager::updateOrder(const Order& order) {
    QFile file(filePath_);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QDomDocument doc;
    doc.setContent(&file);
    file.close();

    QDomElement root = doc.documentElement();

    QDomElement orderElem = root.firstChildElement("order");
    while (!orderElem.isNull()) {
        if (orderElem.attribute("id").toInt() == order.getId()) {
            auto updateField = [&](const QString& tagName, const QString& newValue) {
                QDomElement field = orderElem.firstChildElement(tagName);
                if (!field.isNull()) {
                    while (field.hasChildNodes())
                        field.removeChild(field.firstChild());
                    field.appendChild(doc.createTextNode(newValue));
                    }
                };

            updateField("customerName", QString::fromStdString(order.getCustomerName()));
            updateField("orderDate", QString::fromStdString(order.getOrderDate()));
            updateField("status", QString::fromStdString(order.getStatus()));


            QDomElement oldItems = orderElem.firstChildElement("items");
            QDomElement newItems = doc.createElement("items");

            for (const auto& item : order.getItems()) {
                QDomElement itemElem = doc.createElement("item");
                itemElem.setAttribute("productId", item.productId);
                itemElem.setAttribute("productName", QString::fromStdString(item.productName));
                itemElem.setAttribute("quantity", item.quantity);
                newItems.appendChild(itemElem);
            }
            if (!oldItems.isNull())
                orderElem.replaceChild(newItems, oldItems);
            else
                orderElem.appendChild(newItems);

            break;
        }
        orderElem = orderElem.nextSiblingElement("order");
    }

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        return;
    QTextStream stream(&file);
    doc.save(stream, 4);
    file.close();
}

void OrderXmlManager::deleteOrder(int id) {
    QFile file(filePath_);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QDomDocument doc;
    doc.setContent(&file);
    file.close();

    QDomElement root = doc.documentElement();

    QDomElement orderElem = root.firstChildElement("order");
    while (!orderElem.isNull()) {
        if (orderElem.attribute("id").toInt() == id) {
            root.removeChild(orderElem);
            break;
        }
        orderElem = orderElem.nextSiblingElement("order");
    }

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        return;
    QTextStream stream(&file);
    doc.save(stream, 4);
    file.close();
}

int OrderXmlManager::getNextId() {
    std::vector<Order> orders = loadOrders();
    int maxId = 0;
    for (int i = 0; i < (int)orders.size(); i++) {
        if (orders[i].getId() > maxId)
            maxId = orders[i].getId();
    }
    return maxId + 1;
}
