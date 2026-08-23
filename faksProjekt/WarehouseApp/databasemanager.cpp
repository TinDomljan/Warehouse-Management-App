#include "databasemanager.h"
#include "cryptomanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QRandomGenerator>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager inst;
    return inst;
}

bool DatabaseManager::open(const QString& path) {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", CONNECTION);
    db.setDatabaseName(path);
    if (!db.open())
        return false;

    QSqlQuery q(db);
    q.exec("PRAGMA foreign_keys = ON");
    createTables();
    seedDefaultData();
    migratePasswords();
    return true;
}

bool DatabaseManager::isOpen() const {
    return QSqlDatabase::database(CONNECTION).isOpen();
}

QString DatabaseManager::lastError() const {
    return QSqlDatabase::database(CONNECTION).lastError().text();
}

//shema tablice

void DatabaseManager::createTables() {
    QSqlQuery q(QSqlDatabase::database(CONNECTION));

    q.exec("CREATE TABLE IF NOT EXISTS categories ("
           "id          INTEGER PRIMARY KEY AUTOINCREMENT,"
           "name        TEXT NOT NULL,"
           "description TEXT"
           ")");

    q.exec("CREATE TABLE IF NOT EXISTS suppliers ("
           "id             INTEGER PRIMARY KEY AUTOINCREMENT,"
           "company_name   TEXT NOT NULL,"
           "contact_person TEXT,"
           "email          TEXT,"
           "phone          TEXT,"
           "address        TEXT"
           ")");

    q.exec("CREATE TABLE IF NOT EXISTS products ("
           "id          INTEGER PRIMARY KEY AUTOINCREMENT,"
           "name        TEXT NOT NULL,"
           "price       REAL NOT NULL,"
           "quantity    INTEGER NOT NULL,"
           "category_id INTEGER REFERENCES categories(id),"
           "supplier_id INTEGER REFERENCES suppliers(id)"
           ")");


    q.exec("CREATE TABLE IF NOT EXISTS users ("
           "id        INTEGER PRIMARY KEY AUTOINCREMENT,"
           "username  TEXT NOT NULL UNIQUE,"
           "full_name TEXT,"
           "password  TEXT NOT NULL,"
           "role      INTEGER NOT NULL DEFAULT 2"
           ")");
}

void DatabaseManager::seedDefaultData() {
    QSqlQuery q(QSqlDatabase::database(CONNECTION));

    q.exec("SELECT COUNT(*) FROM categories"); //provjera da ne seedamo opet
    if (q.next() && q.value(0).toInt() > 0)
        return;


    q.exec("INSERT INTO categories (name, description) VALUES "
           "('Electronics',    'Electronic devices and components')");
    q.exec("INSERT INTO categories (name, description) VALUES "
           "('Office Supplies','Pens, paper, folders')");
    q.exec("INSERT INTO categories (name, description) VALUES "
           "('Tools',          'Hand and power tools')");


    q.exec("INSERT INTO suppliers (company_name, contact_person, email, phone, address) VALUES "
           "('Acme Electronics','Ivan Horvat','ivan@acme.hr','+385-1-234-5678','Ilica 42, Zagreb')");
    q.exec("INSERT INTO suppliers (company_name, contact_person, email, phone, address) VALUES "
           "('Office Pro','Ana Anic','ana@officepro.hr','+385-1-987-6543','Vukovarska 58, Zagreb')");


    q.exec("INSERT INTO products (name, price, quantity, category_id, supplier_id) VALUES "
           "('Laptop HP ProBook',   899.99,  25, 1, 1)");
    q.exec("INSERT INTO products (name, price, quantity, category_id, supplier_id) VALUES "
           "('Wireless Mouse',       29.99, 150, 1, 1)");
    q.exec("INSERT INTO products (name, price, quantity, category_id, supplier_id) VALUES "
           "('A4 Paper (500 sheets)', 4.99, 300, 2, 2)");
    q.exec("INSERT INTO products (name, price, quantity, category_id, supplier_id) VALUES "
           "('Cordless Drill',      149.99,  12, 3, 1)");


    QSqlDatabase db = QSqlDatabase::database(CONNECTION);
    db.transaction();
    QSqlQuery ins(db);
    ins.prepare("INSERT INTO products (name, price, quantity, category_id, supplier_id) "
                "VALUES (:name, :price, :qty, :cat, :sup)");
    QRandomGenerator* rng = QRandomGenerator::global();
    for (int i = 5; i <= 1000; ++i) {
        const double price = 10.0 + rng->bounded(49001) / 100.0;
        const int    qty   = 1 + rng->bounded(100);
        const int    cat   = 1 + rng->bounded(3);
        const int    sup   = 1 + rng->bounded(2);
        ins.bindValue(":name",  "Product_" + QString::number(i));
        ins.bindValue(":price", price);
        ins.bindValue(":qty",   qty);
        ins.bindValue(":cat",   cat);
        ins.bindValue(":sup",   sup);
        ins.exec();
    }
    db.commit();


    seedDefaultUsers();
}


void DatabaseManager::seedDefaultUsers() {
    QSqlQuery q(QSqlDatabase::database(CONNECTION));

    struct DefaultUser { const char* username; const char* fullName;
                         const char* password; int role; };
    const DefaultUser defaults[] = {
        {"admin",   "Administrator", "admin123",   0},
        {"manager", "Manager User",  "manager123", 1},
        {"clerk",   "Clerk User",    "clerk123",   2},
    };

    for (const DefaultUser& d : defaults) {
        const QString username = QString::fromUtf8(d.username);

        const QString salt = CryptoManager::deriveSalt(username);

        const QString hash = CryptoManager::hashWithSaltAndSpecificPepper(
            QString::fromUtf8(d.password), salt, CryptoManager::PEPPER);

        q.prepare("INSERT OR IGNORE INTO users (username, full_name, password, role) "
                  "VALUES (:uname, :full, :pwd, :role)");
        q.bindValue(":uname", username);
        q.bindValue(":full",  QString::fromUtf8(d.fullName));
        q.bindValue(":pwd",   hash);
        q.bindValue(":role",  d.role);
        q.exec();
    }
}


bool DatabaseManager::addCategory(const Category& category) {
    QSqlQuery q(QSqlDatabase::database(CONNECTION));
    q.prepare("INSERT INTO categories (name, description) VALUES (:name, :desc)");
    q.bindValue(":name", QString::fromStdString(category.getName()));
    q.bindValue(":desc", QString::fromStdString(category.getDescription()));
    return q.exec();
}

bool DatabaseManager::updateCategory(const Category& category) {
    QSqlQuery q(QSqlDatabase::database(CONNECTION));
    q.prepare("UPDATE categories SET name = :name, description = :desc WHERE id = :id");
    q.bindValue(":name", QString::fromStdString(category.getName()));
    q.bindValue(":desc", QString::fromStdString(category.getDescription()));
    q.bindValue(":id",   category.getId());
    return q.exec();
}

bool DatabaseManager::deleteCategory(int id) {
    QSqlQuery q(QSqlDatabase::database(CONNECTION));
    q.prepare("DELETE FROM categories WHERE id = :id");
    q.bindValue(":id", id);
    return q.exec();
}

std::vector<Category> DatabaseManager::getAllCategories() {
    std::vector<Category> result;
    QSqlQuery q(QSqlDatabase::database(CONNECTION));
    q.exec("SELECT id, name, description FROM categories ORDER BY id");
    while (q.next()) {
        result.emplace_back(q.value(0).toInt(),
                            q.value(1).toString().toStdString(),
                            q.value(2).toString().toStdString());
    }
    return result;
}

Category DatabaseManager::getCategoryById(int id) {
    QSqlQuery q(QSqlDatabase::database(CONNECTION));
    q.prepare("SELECT id, name, description FROM categories WHERE id = :id");
    q.bindValue(":id", id);
    if (q.exec() && q.next()) {
        return Category(q.value(0).toInt(),
                        q.value(1).toString().toStdString(),
                        q.value(2).toString().toStdString());
    }
    return Category();
}



bool DatabaseManager::addSupplier(const Supplier& supplier) {
    QSqlQuery q(QSqlDatabase::database(CONNECTION));
    q.prepare("INSERT INTO suppliers (company_name, contact_person, email, phone, address) "
              "VALUES (:company, :contact, :email, :phone, :address)");
    q.bindValue(":company", QString::fromStdString(supplier.getCompanyName()));
    q.bindValue(":contact", QString::fromStdString(supplier.getContactPerson()));
    q.bindValue(":email",   QString::fromStdString(supplier.getEmail()));
    q.bindValue(":phone",   QString::fromStdString(supplier.getPhone()));
    q.bindValue(":address", QString::fromStdString(supplier.getAddress()));
    return q.exec();
}

bool DatabaseManager::updateSupplier(const Supplier& supplier) {
    QSqlQuery q(QSqlDatabase::database(CONNECTION));
    q.prepare("UPDATE suppliers SET company_name = :company, contact_person = :contact, "
              "email = :email, phone = :phone, address = :address WHERE id = :id");
    q.bindValue(":company", QString::fromStdString(supplier.getCompanyName()));
    q.bindValue(":contact", QString::fromStdString(supplier.getContactPerson()));
    q.bindValue(":email",   QString::fromStdString(supplier.getEmail()));
    q.bindValue(":phone",   QString::fromStdString(supplier.getPhone()));
    q.bindValue(":address", QString::fromStdString(supplier.getAddress()));
    q.bindValue(":id",      supplier.getId());
    return q.exec();
}

bool DatabaseManager::deleteSupplier(int id) {
    QSqlQuery q(QSqlDatabase::database(CONNECTION));
    q.prepare("DELETE FROM suppliers WHERE id = :id");
    q.bindValue(":id", id);
    return q.exec();
}

std::vector<Supplier> DatabaseManager::getAllSuppliers() {
    std::vector<Supplier> result;
    QSqlQuery q(QSqlDatabase::database(CONNECTION));
    q.exec("SELECT id, company_name, contact_person, email, phone, address "
           "FROM suppliers ORDER BY id");
    while (q.next()) {
        result.emplace_back(q.value(0).toInt(),
                            q.value(1).toString().toStdString(),
                            q.value(2).toString().toStdString(),
                            q.value(3).toString().toStdString(),
                            q.value(4).toString().toStdString(),
                            q.value(5).toString().toStdString());
    }
    return result;
}

Supplier DatabaseManager::getSupplierById(int id) {
    QSqlQuery q(QSqlDatabase::database(CONNECTION));
    q.prepare("SELECT id, company_name, contact_person, email, phone, address "
              "FROM suppliers WHERE id = :id");
    q.bindValue(":id", id);
    if (q.exec() && q.next()) {
        return Supplier(q.value(0).toInt(),
                        q.value(1).toString().toStdString(),
                        q.value(2).toString().toStdString(),
                        q.value(3).toString().toStdString(),
                        q.value(4).toString().toStdString(),
                        q.value(5).toString().toStdString());
    }
    return Supplier();
}



bool DatabaseManager::addProduct(const Product& product) {
    QSqlQuery q(QSqlDatabase::database(CONNECTION));
    q.prepare("INSERT INTO products (name, price, quantity, category_id, supplier_id) "
              "VALUES (:name, :price, :qty, :catId, :supId)");
    q.bindValue(":name",  QString::fromStdString(product.getName()));
    q.bindValue(":price", product.getPrice());
    q.bindValue(":qty",   product.getQuantity());
    q.bindValue(":catId", product.getCategory().getId());
    q.bindValue(":supId", product.getSupplier().getId());
    return q.exec();
}

bool DatabaseManager::updateProduct(const Product& product) {
    QSqlQuery q(QSqlDatabase::database(CONNECTION));
    q.prepare("UPDATE products SET name = :name, price = :price, quantity = :qty, "
              "category_id = :catId, supplier_id = :supId WHERE id = :id");
    q.bindValue(":name",  QString::fromStdString(product.getName()));
    q.bindValue(":price", product.getPrice());
    q.bindValue(":qty",   product.getQuantity());
    q.bindValue(":catId", product.getCategory().getId());
    q.bindValue(":supId", product.getSupplier().getId());
    q.bindValue(":id",    product.getId());
    return q.exec();
}

bool DatabaseManager::deleteProduct(int id) {
    QSqlQuery q(QSqlDatabase::database(CONNECTION));
    q.prepare("DELETE FROM products WHERE id = :id");
    q.bindValue(":id", id);
    return q.exec();
}

std::vector<Product> DatabaseManager::getAllProducts() {
    std::vector<Product> result;
    QSqlQuery q(QSqlDatabase::database(CONNECTION));
    q.exec(
        "SELECT p.id, p.name, p.price, p.quantity, "
        "       (p.price * p.quantity) AS total_value, "
        "       c.id, c.name, c.description, "
        "       s.id, s.company_name, s.contact_person, s.email, s.phone, s.address "
        "FROM products p "
        "LEFT JOIN categories c ON p.category_id = c.id "
        "LEFT JOIN suppliers  s ON p.supplier_id  = s.id "
        "ORDER BY p.id"
    );
    while (q.next()) {
        Category cat(q.value(5).toInt(),
                     q.value(6).toString().toStdString(),
                     q.value(7).toString().toStdString());
        Supplier sup(q.value(8).toInt(),
                     q.value(9).toString().toStdString(),
                     q.value(10).toString().toStdString(),
                     q.value(11).toString().toStdString(),
                     q.value(12).toString().toStdString(),
                     q.value(13).toString().toStdString());
        result.emplace_back(q.value(0).toInt(),
                            q.value(1).toString().toStdString(),
                            q.value(2).toDouble(),
                            q.value(3).toInt(),
                            cat, sup);
    }
    return result;
}

Product DatabaseManager::getProductById(int id) {
    QSqlQuery q(QSqlDatabase::database(CONNECTION));
    q.prepare(
        "SELECT p.id, p.name, p.price, p.quantity, "
        "       (p.price * p.quantity) AS total_value, "
        "       c.id, c.name, c.description, "
        "       s.id, s.company_name, s.contact_person, s.email, s.phone, s.address "
        "FROM products p "
        "LEFT JOIN categories c ON p.category_id = c.id "
        "LEFT JOIN suppliers  s ON p.supplier_id  = s.id "
        "WHERE p.id = :id"
    );
    q.bindValue(":id", id);
    if (q.exec() && q.next()) {
        Category cat(q.value(5).toInt(),
                     q.value(6).toString().toStdString(),
                     q.value(7).toString().toStdString());
        Supplier sup(q.value(8).toInt(),
                     q.value(9).toString().toStdString(),
                     q.value(10).toString().toStdString(),
                     q.value(11).toString().toStdString(),
                     q.value(12).toString().toStdString(),
                     q.value(13).toString().toStdString());
        return Product(q.value(0).toInt(),
                       q.value(1).toString().toStdString(),
                       q.value(2).toDouble(),
                       q.value(3).toInt(),
                       cat, sup);
    }
    return Product();
}

std::vector<Product> DatabaseManager::getProductsFiltered(const QString& search,
                                                           const QString& orderByClause) {
    std::vector<Product> result;
    QSqlQuery q(QSqlDatabase::database(CONNECTION));


    QString sql =
        "SELECT p.id, p.name, p.price, p.quantity, "
        "       (p.price * p.quantity) AS total_value, "
        "       c.id, c.name, c.description, "
        "       s.id, s.company_name, s.contact_person, s.email, s.phone, s.address "
        "FROM products p "
        "LEFT JOIN categories c ON p.category_id = c.id "
        "LEFT JOIN suppliers  s ON p.supplier_id  = s.id ";

    //dodajemo na sql upit za filtriranje i trazenje
    if (!search.trimmed().isEmpty()) {
        sql += "WHERE p.name LIKE :search "
               "   OR c.name LIKE :search "
               "   OR s.company_name LIKE :search ";
    }

    sql += "ORDER BY " + (orderByClause.isEmpty() ? QString("p.id ASC") : orderByClause);

    //parametriziramo i izvrsujemo
    q.prepare(sql);
    if (!search.trimmed().isEmpty())
        q.bindValue(":search", "%" + search.trimmed() + "%"); //postotci su wildcard

    if (!q.exec())
        return result;

    while (q.next()) {
        Category cat(q.value(5).toInt(),
                     q.value(6).toString().toStdString(),
                     q.value(7).toString().toStdString());
        Supplier sup(q.value(8).toInt(),
                     q.value(9).toString().toStdString(),
                     q.value(10).toString().toStdString(),
                     q.value(11).toString().toStdString(),
                     q.value(12).toString().toStdString(),
                     q.value(13).toString().toStdString());
        result.emplace_back(q.value(0).toInt(),
                            q.value(1).toString().toStdString(),
                            q.value(2).toDouble(),
                            q.value(3).toInt(),
                            cat, sup);
    }
    return result;
}



bool DatabaseManager::addUser(const User& user, const std::string& password) {
    const QString username = QString::fromStdString(user.getUsername());

    const QString salt = CryptoManager::deriveSalt(username);
    const QString hash = CryptoManager::hashWithSaltAndSpecificPepper(
        QString::fromStdString(password), salt, CryptoManager::PEPPER);
    QSqlQuery q(QSqlDatabase::database(CONNECTION));
    q.prepare("INSERT INTO users (username, full_name, password, role) "
              "VALUES (:uname, :fullname, :pwd, :role)");
    q.bindValue(":uname",    username);
    q.bindValue(":fullname", QString::fromStdString(user.getFullName()));
    q.bindValue(":pwd",      hash);
    q.bindValue(":role",     static_cast<int>(user.getRole()));
    return q.exec();
}

bool DatabaseManager::updateUser(const User& user, const std::string& password) {
    const QString username = QString::fromStdString(user.getUsername());

    const QString salt = CryptoManager::deriveSalt(username);
    const QString hash = CryptoManager::hashWithSaltAndSpecificPepper(
        QString::fromStdString(password), salt, CryptoManager::PEPPER);
    QSqlQuery q(QSqlDatabase::database(CONNECTION));
    q.prepare("UPDATE users SET full_name = :fullname, password = :pwd, role = :role "
              "WHERE id = :id");
    q.bindValue(":fullname", QString::fromStdString(user.getFullName()));
    q.bindValue(":pwd",      hash);
    q.bindValue(":role",     static_cast<int>(user.getRole()));
    q.bindValue(":id",       user.getId());
    return q.exec();
}

bool DatabaseManager::updateUserProfile(const User& user) {
    QSqlQuery q(QSqlDatabase::database(CONNECTION));
    q.prepare("UPDATE users SET full_name = :fullname, role = :role "
              "WHERE id = :id");
    q.bindValue(":fullname", QString::fromStdString(user.getFullName()));
    q.bindValue(":role",     static_cast<int>(user.getRole()));
    q.bindValue(":id",       user.getId());
    return q.exec();
}

bool DatabaseManager::deleteUser(int id) {
    QSqlQuery q(QSqlDatabase::database(CONNECTION));
    q.prepare("DELETE FROM users WHERE id = :id");
    q.bindValue(":id", id);
    return q.exec();
}

std::vector<User> DatabaseManager::getAllUsers() {
    std::vector<User> result;
    QSqlQuery q(QSqlDatabase::database(CONNECTION));
    q.exec("SELECT id, username, full_name, password, role FROM users ORDER BY id");
    while (q.next()) {
        result.emplace_back(q.value(0).toInt(),
                            q.value(1).toString().toStdString(),
                            q.value(2).toString().toStdString(),
                            q.value(3).toString().toStdString(),
                            static_cast<UserRole>(q.value(4).toInt()));
    }
    return result;
}

User DatabaseManager::getUserByUsername(const std::string& username) {
    QSqlQuery q(QSqlDatabase::database(CONNECTION));
    q.prepare("SELECT id, username, full_name, password, role FROM users WHERE username = :uname");
    q.bindValue(":uname", QString::fromStdString(username));
    if (q.exec() && q.next()) {
        return User(q.value(0).toInt(),
                    q.value(1).toString().toStdString(),
                    q.value(2).toString().toStdString(),
                    q.value(3).toString().toStdString(),
                    static_cast<UserRole>(q.value(4).toInt()));
    }
    return User();
}

void DatabaseManager::migratePasswords() {
    QSqlDatabase db = QSqlDatabase::database(CONNECTION);
    QSqlQuery q(db);


    bool hasSaltColumn = false;
    QSqlQuery info(db);
    info.exec("PRAGMA table_info(users)");
    while (info.next()) {
        if (info.value(1).toString() == "salt") {
            hasSaltColumn = true;
            break;
        }
    }

    if (!hasSaltColumn)
        return;


    q.exec("DROP TABLE users");
    q.exec("CREATE TABLE IF NOT EXISTS users ("
           "id        INTEGER PRIMARY KEY AUTOINCREMENT,"
           "username  TEXT NOT NULL UNIQUE,"
           "full_name TEXT,"
           "password  TEXT NOT NULL,"
           "role      INTEGER NOT NULL DEFAULT 2"
           ")");
    seedDefaultUsers();
}
