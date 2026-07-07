#pragma once

#include <QString>
#include <QSqlDatabase>
#include <vector>
#include "Category.h"
#include "Supplier.h"
#include "Product.h"
#include "User.h"

class DatabaseManager {
public:
    static DatabaseManager& instance();

    bool open(const QString& path);
    bool isOpen() const;
    QString lastError() const;

    // Kategorije CRUD
    bool addCategory(const Category& category);
    bool updateCategory(const Category& category);
    bool deleteCategory(int id);
    std::vector<Category> getAllCategories();
    Category getCategoryById(int id);

    // Supplieri CRUD
    bool addSupplier(const Supplier& supplier);
    bool updateSupplier(const Supplier& supplier);
    bool deleteSupplier(int id);
    std::vector<Supplier> getAllSuppliers();
    Supplier getSupplierById(int id);

    // Products CRUD
    bool addProduct(const Product& product);
    bool updateProduct(const Product& product);
    bool deleteProduct(int id);
    std::vector<Product> getAllProducts();
    Product getProductById(int id);

    std::vector<Product> getProductsFiltered(const QString& search,
                                             const QString& orderByClause);

    // Users CRUD

    bool addUser(const User& user, const std::string& password);
    bool updateUser(const User& user, const std::string& password);
    bool deleteUser(int id);
    std::vector<User> getAllUsers();
    User getUserByUsername(const std::string& username);

private:
    DatabaseManager() = default;
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    void createTables();
    void seedDefaultData();
    void seedDefaultUsers();
    void migratePasswords();

    static constexpr const char* CONNECTION = "warehouse";
};
