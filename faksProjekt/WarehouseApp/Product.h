#pragma once
#include <string>
#include "Category.h"
#include "Supplier.h"

class Product {
public:
    Product();
    Product(int id, std::string name, double price, int quantity,
        Category category, Supplier supplier);

    // Getters
    int getId() const;
    std::string getName() const;
    double getPrice() const;
    int getQuantity() const;
    Category getCategory() const;
    Supplier getSupplier() const;

    // Setters
    void setName(const std::string& name);
    void setPrice(double price);
    void setQuantity(int quantity);
    void setCategory(const Category& category);
    void setSupplier(const Supplier& supplier);

    // Business logic methods
    double getTotalValue() const;
    bool isLowStock(int threshold) const;
    void addStock(int amount);
    void removeStock(int amount);

    // Utility
    std::string toString() const;

private:
    int id_;
    std::string name_;
    double price_;
    int quantity_;
    Category category_;
    Supplier supplier_;
};