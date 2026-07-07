#include "Order.h"

Order::Order() {
    id_ = 0;
    customerName_ = "";
    orderDate_ = "";
    status_ = "PENDING";
}

Order::Order(int id, std::string customerName, std::string orderDate,
             std::string status, std::vector<OrderItem> items) {
    id_ = id;
    customerName_ = customerName;
    orderDate_ = orderDate;
    status_ = status;
    items_ = items;
}

int Order::getId() const {
    return id_;
}

std::string Order::getCustomerName() const {
    return customerName_;
}

std::string Order::getOrderDate() const {
    return orderDate_;
}

std::string Order::getStatus() const {
    return status_;
}

std::vector<OrderItem> Order::getItems() const {
    return items_;
}

void Order::setCustomerName(const std::string& customerName) {
    customerName_ = customerName;
}

void Order::setOrderDate(const std::string& orderDate) {
    orderDate_ = orderDate;
}

void Order::setStatus(const std::string& status) {
    status_ = status;
}

void Order::setItems(const std::vector<OrderItem>& items) {
    items_ = items;
}

void Order::addItem(const OrderItem& item) {
    items_.push_back(item);
}

bool Order::isValidStatus(const std::string& status) {
    return status == "PENDING"   || status == "SHIPPED" ||
           status == "DELIVERED" || status == "CANCELLED";
}

bool Order::isValid() const {
    if (customerName_.empty())
        return false;
    if (!isValidStatus(status_))
        return false;
    for (const auto& item : items_) {
        if (item.quantity <= 0)
            return false;
    }
    return true;
}

std::string Order::toString() const {
    return "[" + std::to_string(id_) + "] " + customerName_ +
        " (" + orderDate_ + ", " + status_ + ", " +
        std::to_string(items_.size()) + " items)";
}
