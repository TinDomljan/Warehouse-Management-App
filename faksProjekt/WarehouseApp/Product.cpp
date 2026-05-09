#include "Product.h"

Product::Product() {
    id_ = 0;
    name_ = "";
    price_ = 0.0;
    quantity_ = 0;
    // category_ and supplier_ automatically use their default constructors
}

Product::Product(int id, std::string name, double price, int quantity,
    Category category, Supplier supplier) {
    id_ = id;
    name_ = name;
    price_ = price;
    quantity_ = quantity;
    category_ = category;
    supplier_ = supplier;
}

// Getters
int Product::getId() const {
    return id_;
}

std::string Product::getName() const {
    return name_;
}

double Product::getPrice() const {
    return price_;
}

int Product::getQuantity() const {
    return quantity_;
}

Category Product::getCategory() const {
    return category_;
}

Supplier Product::getSupplier() const {
    return supplier_;
}

// Setters
void Product::setName(const std::string& name) {
    name_ = name;
}

void Product::setPrice(double price) {
    price_ = price;
}

void Product::setQuantity(int quantity) {
    quantity_ = quantity;
}

void Product::setCategory(const Category& category) {
    category_ = category;
}

void Product::setSupplier(const Supplier& supplier) {
    supplier_ = supplier;
}

// Business logic
double Product::getTotalValue() const {
    return price_ * quantity_;
}

bool Product::isLowStock(int threshold) const {
    return quantity_ < threshold;
}

void Product::addStock(int amount) {
    if (amount > 0) {
        quantity_ += amount;
    }
}

void Product::removeStock(int amount) {
    if (amount > 0 && amount <= quantity_) {
        quantity_ -= amount;
    }
}

// Utility
std::string Product::toString() const {
    return "[" + std::to_string(id_) + "] " + name_ +
        " | Price: " + std::to_string(price_) +
        " | Qty: " + std::to_string(quantity_) +
        " | Value: " + std::to_string(getTotalValue()) +
        " | Category: " + category_.getName() +
        " | Supplier: " + supplier_.getCompanyName();
}