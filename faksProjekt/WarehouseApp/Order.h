#pragma once
#include <string>
#include <vector>


struct OrderItem {
    int         productId;
    std::string productName;
    int         quantity;

    OrderItem() : productId(0), productName(""), quantity(0) {}
    OrderItem(int id, std::string name, int qty)
        : productId(id), productName(name), quantity(qty) {}
};

class Order
{
public:
    Order();
    Order(int id, std::string customerName, std::string orderDate,
          std::string status, std::vector<OrderItem> items);

    // Getters
    int getId() const;
    std::string getCustomerName() const;
    std::string getOrderDate() const;
    std::string getStatus() const;
    std::vector<OrderItem> getItems() const;

    // Setters
    void setCustomerName(const std::string& customerName);
    void setOrderDate(const std::string& orderDate);
    void setStatus(const std::string& status);
    void setItems(const std::vector<OrderItem>& items);
    void addItem(const OrderItem& item);

    // Validacija
    bool isValid() const;
    static bool isValidStatus(const std::string& status);


    std::string toString() const;

private:
    int id_;
    std::string customerName_;
    std::string orderDate_;
    std::string status_;
    std::vector<OrderItem> items_;
};
