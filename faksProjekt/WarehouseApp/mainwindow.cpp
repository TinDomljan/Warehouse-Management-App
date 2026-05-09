#include "mainwindow.h"
#include "productdialog.h"
#include "settingsdialog.h"
#include "settingsmanager.h"
#include "translationmanager.h"
#include "xmlmanager.h"
#include "databasemanager.h"
#include "reportmanager.h"
#include <QColor>
#include <QCloseEvent>
#include <QTabWidget>
#include <QInputDialog>
#include <QFileDialog>
#include "supplierdialog.h"

static QString T(const std::string& key) {
    return QString::fromStdString(TranslationManager::instance().tr(key));
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    SettingsManager& sm = SettingsManager::instance();
    currentFontSize_ = sm.loadFontSize();
    currentTextColor_ = sm.loadTextColor();
    currentBgColor_ = sm.loadBgColor();
    currentLanguage_ = sm.loadLanguage();

    TranslationManager::instance().setLanguage(currentLanguage_.toStdString());

    xmlManager_ = new XmlManager("suppliers.xml");
    jsonManager_ = new JsonManager("activity_log.json");
    binaryManager_ = new BinaryManager("stock_snapshot.bin");

    std::vector<LogEntry> previousEntries = jsonManager_->loadEntries();
    for (int i = 0; i < previousEntries.size(); i++) {
        activityLog_.addEntry(previousEntries[i].username,
                              previousEntries[i].action,
                              previousEntries[i].target);
    }

    setupUI();
    setupMenuBar();
    loadSampleData();
    refreshProductTable();
    refreshSupplierTable();
    refreshLogTable();
    updateLanguage();

    QPoint pos = sm.loadWindowPosition();
    QSize size = sm.loadWindowSize();
    move(pos);
    resize(size);
    if (sm.loadIsMaximized()) {
        showMaximized();
    }

    setWindowTitle(T("main_title"));
}

void MainWindow::setLoggedInUser(const User& user) {
    currentUser_ = user;
    userInfoLabel_->setText(T("main_logged_in") +
                            QString::fromStdString(user.getFullName() + " (" + user.getRoleAsString() + ")"));
    statusBar()->showMessage(T("main_welcome") + QString::fromStdString(user.getFullName() + "!"));
    activityLog_.addEntry(user.getUsername(), "LOGIN", "System");
}

// ─── UI Setup ────────────────────────────────────────────────────────────────

void MainWindow::setupUI() {
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    userInfoLabel_ = new QLabel();
    QFont infoFont = userInfoLabel_->font();
    infoFont.setPointSize(11);
    infoFont.setBold(true);
    userInfoLabel_->setFont(infoFont);
    mainLayout->addWidget(userInfoLabel_);

    QTabWidget* tabWidget = new QTabWidget();
    setupProductsTab(tabWidget);
    setupSnapshotTab(tabWidget);
    setupSuppliersTab(tabWidget);
    setupLogTab(tabWidget);
    mainLayout->addWidget(tabWidget);

    QHBoxLayout* bottomLayout = new QHBoxLayout();
    generateReportBtn_ = new QPushButton();
    settingsBtn_       = new QPushButton();
    aboutBtn_          = new QPushButton();
    logoutBtn_         = new QPushButton();
    bottomLayout->addStretch();
    bottomLayout->addWidget(generateReportBtn_);
    bottomLayout->addWidget(settingsBtn_);
    bottomLayout->addWidget(aboutBtn_);
    bottomLayout->addWidget(logoutBtn_);
    mainLayout->addLayout(bottomLayout);

    connect(generateReportBtn_, &QPushButton::clicked, this, &MainWindow::onGenerateReport);
    connect(settingsBtn_,       &QPushButton::clicked, this, &MainWindow::onOpenSettings);
    connect(aboutBtn_,          &QPushButton::clicked, this, &MainWindow::onOpenAbout);
    connect(logoutBtn_,         &QPushButton::clicked, this, &MainWindow::onLogout);
}

void MainWindow::setupProductsTab(QTabWidget* tabs) {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    // ── Filter / sort bar ──────────────────────────────────────────────
    QHBoxLayout* filterLayout = new QHBoxLayout();

    searchEdit_ = new QLineEdit();
    searchEdit_->setPlaceholderText("Search by name, category or supplier...");
    searchEdit_->setClearButtonEnabled(true);

    sortCombo_ = new QComboBox();
    sortCombo_->addItem("Default (ID)",     QString("p.id ASC"));
    sortCombo_->addItem("Name A → Z",  QString("p.name ASC"));
    sortCombo_->addItem("Name Z → A",  QString("p.name DESC"));
    sortCombo_->addItem("Price ↑",     QString("p.price ASC"));
    sortCombo_->addItem("Price ↓",     QString("p.price DESC"));
    sortCombo_->addItem("Quantity ↑",  QString("p.quantity ASC"));
    sortCombo_->addItem("Quantity ↓",  QString("p.quantity DESC"));
    sortCombo_->addItem("Value ↑",     QString("(p.price * p.quantity) ASC"));
    sortCombo_->addItem("Value ↓",     QString("(p.price * p.quantity) DESC"));
    sortCombo_->addItem("Category",         QString("c.name ASC"));
    sortCombo_->addItem("Supplier",         QString("s.company_name ASC"));

    filterLayout->addWidget(new QLabel("Search:"));
    filterLayout->addWidget(searchEdit_, 1);
    filterLayout->addWidget(new QLabel("Sort:"));
    filterLayout->addWidget(sortCombo_);
    layout->addLayout(filterLayout);

    // ── Table ─────────────────────────────────────────────────────────
    productTable_ = new QTableWidget(0, 7);
    productTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    productTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    productTable_->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(productTable_);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    addProductBtn_    = new QPushButton();
    removeProductBtn_ = new QPushButton();
    btnLayout->addWidget(addProductBtn_);
    btnLayout->addWidget(removeProductBtn_);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    connect(addProductBtn_,    &QPushButton::clicked, this, &MainWindow::onAddProduct);
    connect(removeProductBtn_, &QPushButton::clicked, this, &MainWindow::onRemoveProduct);
    connect(searchEdit_, &QLineEdit::textChanged,         this, [this]{ refreshProductTable(); });
    connect(sortCombo_,  &QComboBox::currentIndexChanged, this, [this]{ refreshProductTable(); });

    tabs->addTab(tab, "Products");
}

void MainWindow::setupSnapshotTab(QTabWidget* tabs) {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    QLabel* info = new QLabel("Save or load a binary snapshot of current inventory.\n"
                              "Snapshots capture product quantities and prices at a specific moment.");
    layout->addWidget(info);

    snapshotTable_ = new QTableWidget(0, 5);
    snapshotTable_->setHorizontalHeaderLabels({"ID", "Product", "Quantity", "Price", "Value"});
    snapshotTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    snapshotTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    snapshotTable_->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(snapshotTable_);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* saveSnapshotBtn = new QPushButton("Save Snapshot");
    QPushButton* loadSnapshotBtn = new QPushButton("Load Snapshot");
    btnLayout->addWidget(saveSnapshotBtn);
    btnLayout->addWidget(loadSnapshotBtn);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    connect(saveSnapshotBtn, &QPushButton::clicked, this, &MainWindow::onSaveSnapshot);
    connect(loadSnapshotBtn, &QPushButton::clicked, this, &MainWindow::onLoadSnapshot);

    tabs->addTab(tab, "Snapshots");
}

void MainWindow::setupSuppliersTab(QTabWidget* tabs) {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    supplierTable_ = new QTableWidget(0, 6);
    supplierTable_->setHorizontalHeaderLabels(
        {"ID", "Company", "Contact", "Email", "Phone", "Address"});
    supplierTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    supplierTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    supplierTable_->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(supplierTable_);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* addSupplierBtn    = new QPushButton("Add Supplier");
    QPushButton* editSupplierBtn   = new QPushButton("Edit Supplier");
    QPushButton* deleteSupplierBtn = new QPushButton("Delete Supplier");
    btnLayout->addWidget(addSupplierBtn);
    btnLayout->addWidget(editSupplierBtn);
    btnLayout->addWidget(deleteSupplierBtn);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    connect(addSupplierBtn,    &QPushButton::clicked, this, &MainWindow::onAddSupplier);
    connect(editSupplierBtn,   &QPushButton::clicked, this, &MainWindow::onEditSupplier);
    connect(deleteSupplierBtn, &QPushButton::clicked, this, &MainWindow::onDeleteSupplier);

    tabs->addTab(tab, "Suppliers");
}

void MainWindow::setupLogTab(QTabWidget* tabs) {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    logTable_ = new QTableWidget(0, 5);
    logTable_->setHorizontalHeaderLabels({"ID", "Time", "User", "Action", "Target"});
    logTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    logTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    logTable_->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(logTable_);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* clearLogBtn = new QPushButton("Clear Log");
    btnLayout->addWidget(clearLogBtn);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    connect(clearLogBtn, &QPushButton::clicked, this, &MainWindow::onClearLog);

    tabs->addTab(tab, "Activity Log");
}

void MainWindow::setupMenuBar() {
    menuBar()->clear();
    QMenu* fileMenu = menuBar()->addMenu(T("main_menu_file"));
    fileMenu->addAction(T("main_btn_add"),    this, &MainWindow::onAddProduct);
    fileMenu->addAction(T("main_btn_remove"), this, &MainWindow::onRemoveProduct);
    fileMenu->addSeparator();
    fileMenu->addAction(T("main_btn_logout"), this, &MainWindow::onLogout);

    QMenu* toolsMenu = menuBar()->addMenu(T("main_menu_tools"));
    toolsMenu->addAction(T("main_btn_settings"), this, &MainWindow::onOpenSettings);

    QMenu* helpMenu = menuBar()->addMenu(T("main_menu_help"));
    helpMenu->addAction(T("main_btn_about"), this, &MainWindow::onOpenAbout);
}

void MainWindow::updateLanguage() {
    setWindowTitle(T("main_title"));
    addProductBtn_->setText(T("main_btn_add"));
    removeProductBtn_->setText(T("main_btn_remove"));
    generateReportBtn_->setText("Generate Report");
    settingsBtn_->setText(T("main_btn_settings"));
    aboutBtn_->setText(T("main_btn_about"));
    logoutBtn_->setText(T("main_btn_logout"));

    productTable_->setHorizontalHeaderLabels({
        T("main_col_id"), T("main_col_name"), T("main_col_price"),
        T("main_col_qty"), T("main_col_value"), T("main_col_category"),
        T("main_col_supplier")
    });

    if (currentUser_.getId() != 0) {
        userInfoLabel_->setText(T("main_logged_in") +
                                QString::fromStdString(currentUser_.getFullName() + " (" +
                                                       currentUser_.getRoleAsString() + ")"));
    }

    setupMenuBar();
}

// ─── Data ────────────────────────────────────────────────────────────────────

void MainWindow::loadSampleData() {
    categories_ = DatabaseManager::instance().getAllCategories();
    suppliers_  = DatabaseManager::instance().getAllSuppliers();
    products_   = DatabaseManager::instance().getAllProducts();
}

// ─── Table refresh ───────────────────────────────────────────────────────────

void MainWindow::applyTableStyle(QTableWidget* table) {
    for (int row = 0; row < table->rowCount(); row++) {
        for (int col = 0; col < table->columnCount(); col++) {
            QTableWidgetItem* item = table->item(row, col);
            if (item) {
                item->setForeground(currentTextColor_);
                item->setBackground(currentBgColor_);
                QFont f = item->font();
                f.setPointSize(currentFontSize_);
                item->setFont(f);
            }
        }
    }
}

void MainWindow::refreshProductTable() {
    QString search  = searchEdit_->text().trimmed();
    QString orderBy = sortCombo_->currentData().toString();

    std::vector<Product> rows =
        DatabaseManager::instance().getProductsFiltered(search, orderBy);

    productTable_->setRowCount(0);
    for (int i = 0; i < rows.size(); i++) {
        int row = productTable_->rowCount();
        productTable_->insertRow(row);

        productTable_->setItem(row, 0, new QTableWidgetItem(
                                           QString::number(rows[i].getId())));
        productTable_->setItem(row, 1, new QTableWidgetItem(
                                           QString::fromStdString(rows[i].getName())));
        productTable_->setItem(row, 2, new QTableWidgetItem(
                                           QString::number(rows[i].getPrice(), 'f', 2)));
        productTable_->setItem(row, 3, new QTableWidgetItem(
                                           QString::number(rows[i].getQuantity())));
        productTable_->setItem(row, 4, new QTableWidgetItem(
                                           QString::number(rows[i].getTotalValue(), 'f', 2)));
        productTable_->setItem(row, 5, new QTableWidgetItem(
                                           QString::fromStdString(rows[i].getCategory().getName())));
        productTable_->setItem(row, 6, new QTableWidgetItem(
                                           QString::fromStdString(rows[i].getSupplier().getCompanyName())));
    }

    applyTableStyle(productTable_);
    productTable_->resizeColumnsToContents();
}

void MainWindow::refreshSupplierTable() {
    supplierTable_->setRowCount(0);

    for (int i = 0; i < suppliers_.size(); i++) {
        int row = supplierTable_->rowCount();
        supplierTable_->insertRow(row);

        supplierTable_->setItem(row, 0, new QTableWidgetItem(
                                            QString::number(suppliers_[i].getId())));
        supplierTable_->setItem(row, 1, new QTableWidgetItem(
                                            QString::fromStdString(suppliers_[i].getCompanyName())));
        supplierTable_->setItem(row, 2, new QTableWidgetItem(
                                            QString::fromStdString(suppliers_[i].getContactPerson())));
        supplierTable_->setItem(row, 3, new QTableWidgetItem(
                                            QString::fromStdString(suppliers_[i].getEmail())));
        supplierTable_->setItem(row, 4, new QTableWidgetItem(
                                            QString::fromStdString(suppliers_[i].getPhone())));
        supplierTable_->setItem(row, 5, new QTableWidgetItem(
                                            QString::fromStdString(suppliers_[i].getAddress())));
    }

    applyTableStyle(supplierTable_);
    supplierTable_->resizeColumnsToContents();
}

void MainWindow::refreshLogTable() {
    logTable_->setRowCount(0);

    std::vector<LogEntry> entries = activityLog_.getAllEntries();

    for (int i = 0; i < entries.size(); i++) {
        int row = logTable_->rowCount();
        logTable_->insertRow(row);

        char timeBuffer[26];
        ctime_s(timeBuffer, sizeof(timeBuffer), &entries[i].timestamp);
        QString timeStr = QString(timeBuffer).trimmed();

        logTable_->setItem(row, 0, new QTableWidgetItem(
                                       QString::number(entries[i].id)));
        logTable_->setItem(row, 1, new QTableWidgetItem(timeStr));
        logTable_->setItem(row, 2, new QTableWidgetItem(
                                       QString::fromStdString(entries[i].username)));
        logTable_->setItem(row, 3, new QTableWidgetItem(
                                       QString::fromStdString(entries[i].action)));
        logTable_->setItem(row, 4, new QTableWidgetItem(
                                       QString::fromStdString(entries[i].target)));
    }

    logTable_->resizeColumnsToContents();
}

// ─── Slot handlers ───────────────────────────────────────────────────────────

void MainWindow::onAddSupplier() {
    int nextId = suppliers_.empty() ? 1 : suppliers_.back().getId() + 1;

    SupplierDialog dlg(nextId, "Add Supplier", this);
    if (dlg.exec() == QDialog::Accepted) {
        DatabaseManager::instance().addSupplier(dlg.getSupplier());
        suppliers_ = DatabaseManager::instance().getAllSuppliers();
        refreshSupplierTable();

        activityLog_.addEntry(currentUser_.getUsername(), "ADD_SUPPLIER",
                              dlg.getSupplier().getCompanyName());
        statusBar()->showMessage("Supplier added", 3000);
    }
}

void MainWindow::onEditSupplier() {
    int currentRow = supplierTable_->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Edit Supplier", "Please select a supplier to edit.");
        return;
    }

    SupplierDialog dlg(suppliers_[currentRow].getId(), "Edit Supplier", this);
    dlg.setSupplier(suppliers_[currentRow]);
    if (dlg.exec() == QDialog::Accepted) {
        DatabaseManager::instance().updateSupplier(dlg.getSupplier());
        suppliers_ = DatabaseManager::instance().getAllSuppliers();
        refreshSupplierTable();

        activityLog_.addEntry(currentUser_.getUsername(), "EDIT_SUPPLIER",
                              dlg.getSupplier().getCompanyName());
        statusBar()->showMessage("Supplier updated", 3000);
    }
}

void MainWindow::onDeleteSupplier() {
    int currentRow = supplierTable_->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Delete Supplier", "Please select a supplier to delete.");
        return;
    }

    QString companyName = supplierTable_->item(currentRow, 1)->text();
    int id = supplierTable_->item(currentRow, 0)->text().toInt();

    int reply = QMessageBox::question(this, "Confirm Delete",
                                      "Are you sure you want to delete supplier " + companyName + "?",
                                      QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        DatabaseManager::instance().deleteSupplier(id);
        suppliers_ = DatabaseManager::instance().getAllSuppliers();
        refreshSupplierTable();

        activityLog_.addEntry(currentUser_.getUsername(), "DELETE_SUPPLIER",
                              companyName.toStdString());
        statusBar()->showMessage("Supplier deleted: " + companyName, 3000);
    }
}

void MainWindow::onAddProduct() {
    int nextId = products_.empty() ? 1 : products_.back().getId() + 1;

    ProductDialog dlg(categories_, suppliers_, nextId, this);
    if (dlg.exec() == QDialog::Accepted) {
        Product newProduct = dlg.getProduct();
        DatabaseManager::instance().addProduct(newProduct);
        products_ = DatabaseManager::instance().getAllProducts();
        refreshProductTable();

        activityLog_.addEntry(currentUser_.getUsername(), "ADD_PRODUCT", newProduct.getName());
        statusBar()->showMessage(T("main_product_added") +
                                     QString::fromStdString(newProduct.getName()), 3000);
    }
}

void MainWindow::onRemoveProduct() {
    int currentRow = productTable_->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, T("main_btn_remove"), T("main_remove_select"));
        return;
    }

    int     productId   = productTable_->item(currentRow, 0)->text().toInt();
    QString productName = productTable_->item(currentRow, 1)->text();

    int reply = QMessageBox::question(this, T("main_confirm_delete"),
                                      T("main_remove_confirm") + productName + "?",
                                      QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        activityLog_.addEntry(currentUser_.getUsername(), "DELETE", productName.toStdString());
        DatabaseManager::instance().deleteProduct(productId);
        products_ = DatabaseManager::instance().getAllProducts();
        refreshProductTable();
        statusBar()->showMessage(T("main_product_removed") + productName, 3000);
    }
}

void MainWindow::onViewLog() {
    refreshLogTable();
}

void MainWindow::onClearLog() {
    int reply = QMessageBox::question(this, "Clear Log",
                                      "Are you sure you want to clear the entire activity log?",
                                      QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        jsonManager_->clearLog();
        activityLog_ = ActivityLog();
        refreshLogTable();
        statusBar()->showMessage("Activity log cleared", 3000);
    }
}

void MainWindow::onSaveSnapshot() {
    if (products_.empty()) {
        QMessageBox::warning(this, "Save Snapshot", "No products to snapshot.");
        return;
    }

    std::vector<SnapshotRecord> records;
    for (int i = 0; i < products_.size(); i++) {
        SnapshotRecord rec;
        rec.productId = products_[i].getId();

        std::string name = products_[i].getName();
        std::memset(rec.productName, 0, sizeof(rec.productName));
        std::strncpy(rec.productName, name.c_str(), sizeof(rec.productName) - 1);

        rec.quantity   = products_[i].getQuantity();
        rec.price      = products_[i].getPrice();
        rec.totalValue = products_[i].getTotalValue();
        records.push_back(rec);
    }

    if (binaryManager_->saveSnapshot(records)) {
        activityLog_.addEntry(currentUser_.getUsername(), "SAVE_SNAPSHOT",
                              "Products: " + std::to_string(records.size()));
        statusBar()->showMessage(
            QString("Snapshot saved: %1 products").arg(records.size()), 3000);
    } else {
        QMessageBox::warning(this, "Save Snapshot", "Failed to save snapshot.");
    }
}

void MainWindow::onLoadSnapshot() {
    if (!binaryManager_->snapshotExists()) {
        QMessageBox::information(this, "Load Snapshot",
                                 "No snapshot file found. Save a snapshot first.");
        return;
    }

    SnapshotHeader header;
    std::vector<SnapshotRecord> records;

    if (!binaryManager_->loadSnapshot(header, records)) {
        QMessageBox::warning(this, "Load Snapshot",
                             "Failed to load snapshot. File may be corrupted.");
        return;
    }

    char timeBuffer[26];
    ctime_s(timeBuffer, sizeof(timeBuffer), &header.timestamp);
    QString timeStr = QString(timeBuffer).trimmed();

    snapshotTable_->setRowCount(0);
    for (int i = 0; i < records.size(); i++) {
        int row = snapshotTable_->rowCount();
        snapshotTable_->insertRow(row);

        snapshotTable_->setItem(row, 0, new QTableWidgetItem(
                                            QString::number(records[i].productId)));
        snapshotTable_->setItem(row, 1, new QTableWidgetItem(
                                            QString(records[i].productName)));
        snapshotTable_->setItem(row, 2, new QTableWidgetItem(
                                            QString::number(records[i].quantity)));
        snapshotTable_->setItem(row, 3, new QTableWidgetItem(
                                            QString::number(records[i].price, 'f', 2)));
        snapshotTable_->setItem(row, 4, new QTableWidgetItem(
                                            QString::number(records[i].totalValue, 'f', 2)));
    }
    snapshotTable_->resizeColumnsToContents();

    activityLog_.addEntry(currentUser_.getUsername(), "LOAD_SNAPSHOT",
                          "Records: " + std::to_string(records.size()));
    statusBar()->showMessage(
        QString("Snapshot loaded: %1 products from %2").arg(records.size()).arg(timeStr), 5000);
}

void MainWindow::onOpenSettings() {
    SettingsDialog dlg(currentFontSize_, currentTextColor_, currentBgColor_,
                       currentLanguage_, this);
    if (dlg.exec() == QDialog::Accepted) {
        currentFontSize_ = dlg.getFontSize();
        currentTextColor_ = dlg.getTextColor();
        currentBgColor_   = dlg.getBgColor();
        QString newLang   = dlg.getLanguage();

        if (newLang != currentLanguage_) {
            currentLanguage_ = newLang;
            TranslationManager::instance().setLanguage(newLang.toStdString());
            updateLanguage();
        }

        QFont font = productTable_->font();
        font.setPointSize(currentFontSize_);
        productTable_->setFont(font);

        refreshProductTable();
        refreshSupplierTable();

        QFont headerFont = productTable_->horizontalHeader()->font();
        headerFont.setPointSize(currentFontSize_);
        productTable_->horizontalHeader()->setFont(headerFont);
        supplierTable_->horizontalHeader()->setFont(headerFont);

        userInfoLabel_->setStyleSheet(
            QString("color: %1; font-weight: bold; font-size: %2pt;")
                .arg(currentTextColor_.name())
                .arg(currentFontSize_));

        activityLog_.addEntry(currentUser_.getUsername(), "SETTINGS_CHANGED",
                              "Font: " + std::to_string(currentFontSize_) + "pt");
        statusBar()->showMessage(T("main_settings_applied"), 3000);

        SettingsManager::instance().saveVisualSettings(
            currentFontSize_, currentTextColor_, currentBgColor_, currentLanguage_);
    }
}

void MainWindow::onGenerateReport() {
    QString filePath = QFileDialog::getSaveFileName(
        this, "Save Inventory Report",
        "inventory_report.pdf",
        "PDF Files (*.pdf)");

    if (filePath.isEmpty())
        return;

    QString html = ReportManager::generateInventoryReport(products_, categories_);

    if (ReportManager::exportToPDF(html, filePath)) {
        activityLog_.addEntry(currentUser_.getUsername(), "GENERATE_REPORT", filePath.toStdString());
        statusBar()->showMessage("Report saved: " + filePath, 5000);
        QMessageBox::information(this, "Report Generated",
                                 "Inventory report saved to:\n" + filePath);
    } else {
        QMessageBox::warning(this, "Report Error",
                             "Failed to write PDF to:\n" + filePath);
    }
}

void MainWindow::onOpenAbout() {
    QMessageBox::about(this, T("about_title"), T("about_text"));
}

void MainWindow::onLogout() {
    activityLog_.addEntry(currentUser_.getUsername(), "LOGOUT", "System");

    int reply = QMessageBox::question(this, T("main_btn_logout"),
                                      T("main_logout_confirm"),
                                      QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        close();
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    SettingsManager& sm = SettingsManager::instance();
    sm.saveVisualSettings(currentFontSize_, currentTextColor_,
                          currentBgColor_, currentLanguage_);
    sm.saveWindowSettings(pos(), size(), isMaximized());

    jsonManager_->saveLog(activityLog_);
    event->accept();
}
