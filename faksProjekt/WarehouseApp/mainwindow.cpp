#include "mainwindow.h"
#include "productdialog.h"
#include "settingsdialog.h"
#include "settingsmanager.h"
#include "translationmanager.h"
#include "orderxmlmanager.h"
#include "databasemanager.h"
#include "reportmanager.h"
#include "inventoryanalyzer.h"
#include <QColor>
#include <QCloseEvent>
#include <QTabWidget>
#include <QInputDialog>
#include <QFileDialog>
#include <QDialog>
#include <QTextEdit>
#include <QDateTime>
#include "supplierdialog.h"
#include "orderdialog.h"
#include <QProcess>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QUdpSocket>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QTimer>
#include <QFrame>
#include <QCheckBox>
#include "udpprotocol.h"
#include "cryptomanager.h"
#include "warehouseutils.h"
#include "warehouseexport.h"
#include "warehousedlgs.h"
#include "warehouseresources.h"
#include <QJsonArray>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <cmath>

static QString T(const std::string& key) {
    return QString::fromStdString(TranslationManager::instance().tr(key));
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    SettingsManager& sm = SettingsManager::instance();
    currentFontSize_ = sm.loadFontSize(); //citamo iz INI
    currentTextColor_ = sm.loadTextColor();

    currentBgColor_ = sm.loadBgColor();
    currentLanguage_ = sm.loadLanguage();

    TranslationManager::instance().setLanguage(currentLanguage_.toStdString());

    orderXmlManager_ = new OrderXmlManager("orders.xml");
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

    QPoint pos = sm.loadWindowPosition(); //citamo iz registry
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
    activityLog_.addEntry(user.getUsername(), "LOGIN", "System"); //entry log nakon logina

    // UI restrikcije
    applyRolePermissions();
}


void MainWindow::applyRolePermissions() {

    for (int i = 0; i < tabWidget_->count(); ++i)
        tabWidget_->setTabVisible(i, true);

    auto enable = [](QWidget* w, bool on) {
        w->setEnabled(on);
        w->setVisible(on);
    };

    enable(addProductBtn_,     true);
    enable(removeProductBtn_,  true);
    enable(analyzeInventoryBtn_, true);
    enable(exportCsvBtn_,      true);
    enable(exportHtmlBtn_,     true);
    enable(saveSnapshotBtn_,   true);
    enable(loadSnapshotBtn_,   true);
    enable(validateBackupBtn_, true);
    enable(addSupplierBtn_,    true);
    enable(editSupplierBtn_,   true);
    enable(deleteSupplierBtn_, true);
    enable(clearLogBtn_,       true);
    enable(generateReportBtn_, true);
    enable(quickCalcBtn_,      true);
    addProductAction_->setEnabled(true);
    addProductAction_->setVisible(true);
    removeProductAction_->setEnabled(true);
    removeProductAction_->setVisible(true);

    switch (currentUser_.getRole()) {
    case UserRole::Admin:

        break;

    case UserRole::Manager:

        tabWidget_->setTabVisible(4, false);
        tabWidget_->setTabVisible(5, false);
        enable(clearLogBtn_, false);
        break;

    case UserRole::Clerk:

        tabWidget_->setTabVisible(1, false);
        tabWidget_->setTabVisible(4, false);
        tabWidget_->setTabVisible(5, false);

        enable(addProductBtn_,     false);
        enable(removeProductBtn_,  false);
        enable(analyzeInventoryBtn_, false);
        enable(exportCsvBtn_,      false);
        enable(exportHtmlBtn_,     false);
        enable(addSupplierBtn_,    false);
        enable(editSupplierBtn_,   false);
        enable(deleteSupplierBtn_, false);
        enable(generateReportBtn_, false);
        enable(quickCalcBtn_,      false);
        enable(clearLogBtn_,       false);

        addProductAction_->setEnabled(false);
        addProductAction_->setVisible(false);
        removeProductAction_->setEnabled(false);
        removeProductAction_->setVisible(false);
        break;
    }
}

//ui

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

    tabWidget_ = new QTabWidget();
    setupProductsTab(tabWidget_);
    setupSnapshotTab(tabWidget_);
    setupSuppliersTab(tabWidget_);
    setupLogTab(tabWidget_);
    setupNetworkTab(tabWidget_);
    setupCryptoTab(tabWidget_);


    tabWidget_->setTabIcon(0, QIcon(loadWarehouseIcon("product")));
    tabWidget_->setTabIcon(2, QIcon(loadWarehouseIcon("supplier")));
    removeProductBtn_->setIcon(QIcon(loadWarehouseIcon("warning")));

    mainLayout->addWidget(tabWidget_);

    QHBoxLayout* bottomLayout = new QHBoxLayout();
    generateReportBtn_      = new QPushButton();
    settingsBtn_            = new QPushButton();
    aboutBtn_               = new QPushButton();
    quickCalcBtn_           = new QPushButton("Quick Calculator");
    logoutBtn_              = new QPushButton();
    bottomLayout->addStretch();
    bottomLayout->addWidget(generateReportBtn_);
    bottomLayout->addWidget(settingsBtn_);
    bottomLayout->addWidget(aboutBtn_);
    bottomLayout->addWidget(quickCalcBtn_);
    bottomLayout->addWidget(logoutBtn_);
    mainLayout->addLayout(bottomLayout);

    connect(generateReportBtn_, &QPushButton::clicked, this, &MainWindow::onGenerateReport);
    connect(settingsBtn_,       &QPushButton::clicked, this, &MainWindow::onOpenSettings);
    connect(aboutBtn_,          &QPushButton::clicked, this, &MainWindow::onOpenAbout);
    connect(quickCalcBtn_,      &QPushButton::clicked, this, &MainWindow::onQuickCalc);
    connect(logoutBtn_,         &QPushButton::clicked, this, &MainWindow::onLogout);
}

void MainWindow::setupProductsTab(QTabWidget* tabs) {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    //filter
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

    // tablica
    productTable_ = new QTableWidget(0, 7);
    productTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    productTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    productTable_->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(productTable_);


    analysisStatusLabel_ = new QLabel("Waiting for threads...");
    analysisStatusLabel_->setVisible(false);

    analysisProgressBar_ = new QProgressBar();
    analysisProgressBar_->setRange(0, 3);
    analysisProgressBar_->setValue(0);
    analysisProgressBar_->setFormat("Analyzing: %v / %m products");
    analysisProgressBar_->setTextVisible(true);
    analysisProgressBar_->setVisible(false);

    layout->addWidget(analysisStatusLabel_);
    layout->addWidget(analysisProgressBar_);

    // gumbi
    QHBoxLayout* btnLayout = new QHBoxLayout();
    addProductBtn_       = new QPushButton();
    removeProductBtn_    = new QPushButton();
    analyzeInventoryBtn_ = new QPushButton("Analyze Inventory");
    exportCsvBtn_  = new QPushButton("Export CSV");
    exportHtmlBtn_ = new QPushButton("Export HTML");
    btnLayout->addWidget(addProductBtn_);
    btnLayout->addWidget(removeProductBtn_);
    btnLayout->addWidget(analyzeInventoryBtn_);
    btnLayout->addWidget(exportCsvBtn_);
    btnLayout->addWidget(exportHtmlBtn_);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    connect(addProductBtn_,       &QPushButton::clicked, this, &MainWindow::onAddProduct);
    connect(removeProductBtn_,    &QPushButton::clicked, this, &MainWindow::onRemoveProduct);
    connect(analyzeInventoryBtn_, &QPushButton::clicked, this, &MainWindow::onAnalyzeInventory);
    connect(exportCsvBtn_,        &QPushButton::clicked, this, &MainWindow::onExportCSV);
    connect(exportHtmlBtn_,       &QPushButton::clicked, this, &MainWindow::onExportHTML);
    connect(searchEdit_, &QLineEdit::textChanged,         this, [this]{ refreshProductTable(); });
    connect(sortCombo_,  &QComboBox::currentIndexChanged, this, [this]{ refreshProductTable(); });

    tabs->addTab(tab, "Products");
}

void MainWindow::setupSnapshotTab(QTabWidget* tabs) {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    QLabel* info = new QLabel("Save or load a binary snapshot of inventory-analysis results.\n"
                              "Run 'Analyze Inventory' (Products tab) first, then save the per-thread\n"
                              "analysis segments to a custom binary file.");
    layout->addWidget(info);

    snapshotTable_ = new QTableWidget(0, 6);
    snapshotTable_->setHorizontalHeaderLabels(
        {"Thread", "Index Range", "Low Stock", "Segment Value",
         "Most Expensive", "Unit Price"});
    snapshotTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    snapshotTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    snapshotTable_->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(snapshotTable_);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    saveSnapshotBtn_     = new QPushButton("Save Snapshot");
    loadSnapshotBtn_     = new QPushButton("Load Snapshot");
    validateBackupBtn_   = new QPushButton("Validate Backup");
    btnLayout->addWidget(saveSnapshotBtn_);
    btnLayout->addWidget(loadSnapshotBtn_);
    btnLayout->addWidget(validateBackupBtn_);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    connect(saveSnapshotBtn_,   &QPushButton::clicked, this, &MainWindow::onSaveSnapshot);
    connect(loadSnapshotBtn_,   &QPushButton::clicked, this, &MainWindow::onLoadSnapshot);
    connect(validateBackupBtn_, &QPushButton::clicked, this, &MainWindow::onValidateBackup);

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
    addSupplierBtn_    = new QPushButton("Add Supplier");
    editSupplierBtn_   = new QPushButton("Edit Supplier");
    deleteSupplierBtn_ = new QPushButton("Delete Supplier");
    btnLayout->addWidget(addSupplierBtn_);
    btnLayout->addWidget(editSupplierBtn_);
    btnLayout->addWidget(deleteSupplierBtn_);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    connect(addSupplierBtn_,    &QPushButton::clicked, this, &MainWindow::onAddSupplier);
    connect(editSupplierBtn_,   &QPushButton::clicked, this, &MainWindow::onEditSupplier);
    connect(deleteSupplierBtn_, &QPushButton::clicked, this, &MainWindow::onDeleteSupplier);

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
    clearLogBtn_ = new QPushButton("Clear Log");
    btnLayout->addWidget(clearLogBtn_);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    connect(clearLogBtn_, &QPushButton::clicked, this, &MainWindow::onClearLog);

    tabs->addTab(tab, "Activity Log");
}

void MainWindow::setupMenuBar() {
    menuBar()->clear();
    QMenu* fileMenu = menuBar()->addMenu(T("main_menu_file"));
    addProductAction_    = fileMenu->addAction(T("main_btn_add"),    this, &MainWindow::onAddProduct);
    removeProductAction_ = fileMenu->addAction(T("main_btn_remove"), this, &MainWindow::onRemoveProduct);
    fileMenu->addSeparator();
    fileMenu->addAction(T("main_btn_logout"), this, &MainWindow::onLogout);

    QMenu* toolsMenu = menuBar()->addMenu(T("main_menu_tools"));
    toolsMenu->addAction(T("main_btn_settings"), this, &MainWindow::onOpenSettings);

    QMenu* dataMenu = menuBar()->addMenu(T("main_menu_data"));
    dataMenu->addAction(T("main_menu_orders"), this, &MainWindow::onOpenOrders);

    QMenu* helpMenu = menuBar()->addMenu(T("main_menu_help"));
    helpMenu->addAction(T("main_btn_about"), this, &MainWindow::onOpenAbout);


    if (currentUser_.getId() != 0)
        applyRolePermissions();
}

void MainWindow::onOpenOrders() {
    OrderDialog dlg(this);
    dlg.exec();
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



void MainWindow::loadSampleData() {
    categories_ = DatabaseManager::instance().getAllCategories();
    suppliers_  = DatabaseManager::instance().getAllSuppliers();
    products_   = DatabaseManager::instance().getAllProducts();
}



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
                                           WarehouseUtils::formatCurrency(rows[i].getPrice())));
        productTable_->setItem(row, 3, new QTableWidgetItem(
                                           QString::number(rows[i].getQuantity())));
        productTable_->setItem(row, 4, new QTableWidgetItem(
                                           WarehouseUtils::formatCurrency(rows[i].getTotalValue())));
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
    if (lastAnalysisRecords_.empty()) {
        QMessageBox::warning(this, "Save Snapshot",
                             "No analysis results to snapshot.\n"
                             "Run 'Analyze Inventory' on the Products tab first.");
        return;
    }

    if (binaryManager_->saveSnapshot(lastAnalysisRecords_)) {
        activityLog_.addEntry(currentUser_.getUsername(), "SAVE_SNAPSHOT",
                              "Analysis segments: " + std::to_string(lastAnalysisRecords_.size()));
        statusBar()->showMessage(
            QString("Snapshot saved: %1 analysis segments").arg(lastAnalysisRecords_.size()), 3000);
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
    std::vector<AnalysisSnapshotRecord> records;

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
                                            QString::number(records[i].threadId)));
        snapshotTable_->setItem(row, 1, new QTableWidgetItem(
                                            QString("%1 – %2").arg(records[i].rangeStart)
                                                              .arg(records[i].rangeEnd)));
        snapshotTable_->setItem(row, 2, new QTableWidgetItem(
                                            QString::number(records[i].lowStockCount)));
        snapshotTable_->setItem(row, 3, new QTableWidgetItem(
                                            WarehouseUtils::formatCurrency(records[i].totalValue)));
        snapshotTable_->setItem(row, 4, new QTableWidgetItem(
                                            QString(records[i].mostExpensiveProductName)));
        snapshotTable_->setItem(row, 5, new QTableWidgetItem(
                                            WarehouseUtils::formatCurrency(records[i].mostExpensiveProductPrice)));
    }
    snapshotTable_->resizeColumnsToContents();

    activityLog_.addEntry(currentUser_.getUsername(), "LOAD_SNAPSHOT",
                          "Records: " + std::to_string(records.size()));
    statusBar()->showMessage(
        QString("Snapshot loaded: %1 analysis segments from %2").arg(records.size()).arg(timeStr), 5000);
}

void MainWindow::setupNetworkTab(QTabWidget* tabs) {
    QWidget*     tab    = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    QLabel* infoLabel = new QLabel(
        "TCP connection to WarehouseServer on localhost:23117.\n"
        "Start WarehouseServer.exe before using these features.");
    infoLabel->setStyleSheet("color: gray;");
    layout->addWidget(infoLabel);


    QHBoxLayout* queryRow = new QHBoxLayout();
    queryRow->addWidget(new QLabel("Barcode / product name:"));
    barcodeEdit_ = new QLineEdit();
    barcodeEdit_->setPlaceholderText("e.g. Widget Pro");
    queryRow->addWidget(barcodeEdit_, 1);
    QPushButton* queryBtn = new QPushButton("Query Product");
    queryRow->addWidget(queryBtn);
    layout->addLayout(queryRow);

    QHBoxLayout* restRow = new QHBoxLayout();
    QPushButton* exchangeRatesBtn = new QPushButton("Fetch Exchange Rates  (REST · EUR base)");
    restRow->addWidget(exchangeRatesBtn);
    restRow->addStretch();
    layout->addLayout(restRow);


    networkResponseView_ = new QTextEdit();
    networkResponseView_->setReadOnly(true);
    networkResponseView_->setPlaceholderText("Server response will appear here...");
    networkResponseView_->setFontFamily("Courier New");
    layout->addWidget(networkResponseView_);


    QHBoxLayout* fileRow = new QHBoxLayout();
    QPushButton* sendBackupBtn = new QPushButton("Send Backup  (stock_snapshot.bin)  [TCP]");
    networkStatusLabel_ = new QLabel();
    networkStatusLabel_->setStyleSheet("color: gray;");
    fileRow->addWidget(sendBackupBtn);
    fileRow->addWidget(networkStatusLabel_, 1);
    fileRow->addStretch();
    layout->addLayout(fileRow);

    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);

    QLabel* udpLabel = new QLabel("UDP  —  port 23118");
    udpLabel->setStyleSheet("color: gray; font-style: italic;");
    layout->addWidget(udpLabel);

    QHBoxLayout* udpRow = new QHBoxLayout();
    QPushButton* statusBtn  = new QPushButton("Request Status (UDP)");
    QPushButton* logSendBtn = new QPushButton("Send Log Summary (UDP)");
    udpRow->addWidget(statusBtn);
    udpRow->addWidget(logSendBtn);
    udpRow->addStretch();
    layout->addLayout(udpRow);

    connect(queryBtn,         &QPushButton::clicked, this, &MainWindow::onQueryProduct);
    connect(exchangeRatesBtn, &QPushButton::clicked, this, &MainWindow::onFetchExchangeRates);
    connect(sendBackupBtn,    &QPushButton::clicked, this, &MainWindow::onSendBackup);
    connect(statusBtn,     &QPushButton::clicked, this, &MainWindow::onRequestStatus);
    connect(logSendBtn,    &QPushButton::clicked, this, &MainWindow::onSendLogSummary);


    QFrame* line2 = new QFrame();
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line2);

    QLabel* httpLabel = new QLabel("HTTP Download");
    httpLabel->setStyleSheet("color: gray; font-style: italic;");
    layout->addWidget(httpLabel);


    QHBoxLayout* dlRow = new QHBoxLayout();
    dlRow->addWidget(new QLabel("URL:"));
    urlEdit_ = new QLineEdit("https://raw.githubusercontent.com/datablist/sample-csv-files/main/files/customers/customers-2000.csv");
    dlRow->addWidget(urlEdit_, 1);

    speedLimitCombo_ = new QComboBox();
    speedLimitCombo_->addItem("Unlimited");
    speedLimitCombo_->addItem("100 KB/s");
    speedLimitCombo_->addItem("50 KB/s");
    dlRow->addWidget(speedLimitCombo_);

    downloadBtn_ = new QPushButton("Download");
    dlRow->addWidget(downloadBtn_);
    layout->addLayout(dlRow);


    downloadProgressBar_ = new QProgressBar();
    downloadProgressBar_->setRange(0, 100);
    downloadProgressBar_->setValue(0);
    downloadProgressBar_->setFormat("Ready");
    downloadProgressBar_->setTextVisible(true);
    layout->addWidget(downloadProgressBar_);


    downloadView_ = new QTextEdit();
    downloadView_->setReadOnly(true);
    downloadView_->setPlaceholderText("Downloaded content appears here...");
    downloadView_->setFontFamily("Courier New");
    downloadView_->setMaximumHeight(160);
    layout->addWidget(downloadView_);


    networkManager_ = new QNetworkAccessManager(this);
    speedTimer_      = new QTimer(this);
    speedTimer_->setInterval(200);
    connect(speedTimer_, &QTimer::timeout, this, &MainWindow::onReadChunk);

    connect(downloadBtn_, &QPushButton::clicked, this, &MainWindow::onDownload);

    tabs->addTab(tab, "Network");
}


static QTcpSocket* makeSocket(QWidget* parent) {
    auto* sock = new QTcpSocket(parent);
    sock->connectToHost("127.0.0.1", 23117);
    if (!sock->waitForConnected(3000)) {
        QMessageBox::warning(parent, "Network",
                             "Cannot connect to WarehouseServer on localhost:23117.\n"
                             "Make sure WarehouseServer.exe is running.");
        sock->deleteLater();
        return nullptr;
    }
    return sock;
}

void MainWindow::onFetchExchangeRates() {
    networkResponseView_->setPlainText("Fetching exchange rates from open.er-api.com...");
    statusBar()->showMessage("Fetching exchange rates...");

    //saljemo zahtjev
    QNetworkRequest request{QUrl("http://open.er-api.com/v6/latest/EUR")};
    request.setHeader(QNetworkRequest::UserAgentHeader, "WarehouseApp/1.0");

    QNetworkReply* reply = networkManager_->get(request);

    //odgovor
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            networkResponseView_->setPlainText(
                "Error: " + reply->errorString());
            statusBar()->showMessage("Exchange rate fetch failed", 3000);
            return;
        }


        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (doc.isNull() || !doc.isObject()) {
            networkResponseView_->setPlainText("Error: could not parse JSON response.");
            return;
        }

        QJsonObject root  = doc.object();
        if (root["result"].toString() != "success") {
            networkResponseView_->setPlainText(
                "API returned failure: " + root["result"].toString());
            return;
        }

        const QString    base    = root["base_code"].toString();
        const QString    updated = root["time_last_update_utc"].toString();
        const QJsonObject rates  = root["rates"].toObject();


        QString out;
        out += QString("Exchange Rates\n");
        out += QString("Base currency : %1\n").arg(base);
        out += QString("Last updated  : %1\n").arg(updated);
        out += QString("Source        : open.er-api.com\n\n");


        out += QString("%1  %2\n").arg("Code", -6).arg("Rate", 14);
        out += QString(22, u'-') + "\n";

        QStringList keys = rates.keys();
        keys.sort();
        for (const QString& code : keys) {
            const double rate = rates[code].toDouble();
            out += QString("%1  %2\n")
                       .arg(code, -6)
                       .arg(QString::number(rate, 'f', 6), 14);
        }

        out += QString(22, u'-') + "\n";
        out += QString("%1 currencies listed\n").arg(keys.size());

        networkResponseView_->setPlainText(out);

        activityLog_.addEntry(
            currentUser_.getUsername(), "REST_EXCHANGE_RATES",
            QString("base=%1 count=%2").arg(base).arg(keys.size()).toStdString());
        statusBar()->showMessage(
            QString("Exchange rates loaded — %1 currencies, base %2")
                .arg(keys.size()).arg(base), 5000);
    });
}

void MainWindow::onQueryProduct() {
    QString barcode = barcodeEdit_->text().trimmed();
    if (barcode.isEmpty()) {
        QMessageBox::warning(this, "Query Product", "Please enter a barcode or product name.");
        return;
    }

    // ako je samo broj gleda se kao barkod
    const bool isNumeric = std::all_of(barcode.cbegin(), barcode.cend(),
                                       [](QChar c){ return c.isDigit(); });
    if (isNumeric && !WarehouseUtils::validateBarcode(barcode)) {
        QMessageBox::warning(this, "Invalid Barcode",
                             QString("'%1' is not a valid barcode.\n"
                                     "Barcodes must be 8–13 digits (EAN-8 to EAN-13).\n\n"
                                     "To search by product name, include at least one letter.")
                                 .arg(barcode));
        return;
    }

    QTcpSocket* sock = makeSocket(this);
    if (!sock) return;

    sock->write((barcode + "\n").toUtf8());
    sock->waitForBytesWritten(3000);

    if (!sock->waitForReadyRead(5000)) {
        networkResponseView_->setPlainText("Timeout: no response from server.");
        sock->deleteLater();
        return;
    }

    QByteArray raw = sock->readAll();
    sock->disconnectFromHost();
    sock->deleteLater();


    QJsonDocument doc = QJsonDocument::fromJson(raw.trimmed());
    QString display   = doc.isNull()
        ? QString::fromUtf8(raw)
        : doc.toJson(QJsonDocument::Indented);

    networkResponseView_->setPlainText(display);
    activityLog_.addEntry(currentUser_.getUsername(), "NET_QUERY", barcode.toStdString());
    statusBar()->showMessage("Server response received", 3000);
}

void MainWindow::onSendBackup() {
    QFile file("stock_snapshot.bin");
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Send Backup",
                             "stock_snapshot.bin not found.\n"
                             "Save a snapshot first (Snapshots tab).");
        return;
    }
    QByteArray data     = file.readAll();
    qint64     fileSize = data.size();
    file.close();

    QTcpSocket* sock = makeSocket(this);
    if (!sock) return;

    networkStatusLabel_->setText("Sending...");
    networkStatusLabel_->setStyleSheet("color: orange;");


    QByteArray header = QString("FILE:stock_snapshot.bin:%1\n").arg(fileSize).toUtf8();
    sock->write(header);
    sock->write(data);
    sock->waitForBytesWritten(10000);

    if (!sock->waitForReadyRead(10000)) {
        networkStatusLabel_->setText("Timeout waiting for server ACK.");
        networkStatusLabel_->setStyleSheet("color: red;");
        sock->deleteLater();
        return;
    }

    QString reply = QString::fromUtf8(sock->readAll()).trimmed();
    sock->disconnectFromHost();
    sock->deleteLater();

    bool ok = reply.startsWith("OK");
    networkStatusLabel_->setText(reply);
    networkStatusLabel_->setStyleSheet(ok ? "color: green;" : "color: red;");

    if (ok) {
        activityLog_.addEntry(currentUser_.getUsername(), "NET_SEND_BACKUP",
                              "Bytes: " + std::to_string(fileSize));
        statusBar()->showMessage(
            QString("Backup sent to server — %1 bytes").arg(fileSize), 5000);
    } else {
        QMessageBox::warning(this, "Send Backup",
                             "Server reported an error:\n" + reply);
    }
}



void MainWindow::onDownload() {

    if (currentReply_) {
        speedTimer_->stop();
        currentReply_->abort();
        finalizeDownload(true);
        return;
    }

    QString urlStr = urlEdit_->text().trimmed();
    if (urlStr.isEmpty()) {
        QMessageBox::warning(this, "Download", "Please enter a URL.");
        return;
    }

    downloadBuffer_.clear();
    downloadView_->clear();
    downloadProgressBar_->setRange(0, 100);
    downloadProgressBar_->setValue(0);
    downloadProgressBar_->setFormat("Connecting...");
    downloadBtn_->setText("Cancel");

    QNetworkRequest request{QUrl(urlStr)};
    request.setHeader(QNetworkRequest::UserAgentHeader, "WarehouseApp/1.0");

    currentReply_ = networkManager_->get(request);

    const int speedIdx = speedLimitCombo_->currentIndex();
    if (speedIdx == 0) {

        readChunkSize_ = -1;
        connect(currentReply_, &QNetworkReply::readyRead,
                this, &MainWindow::onReadChunk);
        connect(currentReply_, &QNetworkReply::downloadProgress,
                this, &MainWindow::onDownloadProgress);
        connect(currentReply_, &QNetworkReply::finished,
                this, &MainWindow::onDownloadFinished);
    } else {

        readChunkSize_ = (speedIdx == 1) ? 100 * 1024 / 5 : 50 * 1024 / 5;

        currentReply_->setReadBufferSize(readChunkSize_ * 2);

        speedTimer_->start();
    }

    statusBar()->showMessage("Downloading: " + urlStr);
}

void MainWindow::onReadChunk() {
    if (!currentReply_) return;


    if (readChunkSize_ < 0) {
        const QByteArray data = currentReply_->readAll();
        if (!data.isEmpty())
            downloadBuffer_.append(data);
        return;
    }


    const QByteArray data = currentReply_->read(readChunkSize_);
    if (!data.isEmpty())
        downloadBuffer_.append(data);


    const QVariant clen = currentReply_->header(QNetworkRequest::ContentLengthHeader);
    qint64 totalBytes = clen.isValid() ? clen.toLongLong() : 0;
    if (totalBytes <= 0)
        totalBytes = downloadBuffer_.size() + currentReply_->bytesAvailable();
    if (totalBytes <= 0)
        totalBytes = 1;

    int pct = static_cast<int>(downloadBuffer_.size() * 100 / totalBytes);
    if (pct > 100) pct = 100;


    downloadProgressBar_->setRange(0, 100);
    downloadProgressBar_->setValue(pct);
    downloadProgressBar_->setFormat(
        QString("%1% — %2 / %3 KB")
            .arg(pct)
            .arg(downloadBuffer_.size() / 1024)
            .arg(totalBytes / 1024));


    if (currentReply_->isFinished() &&
        (currentReply_->bytesAvailable() == 0 ||
         currentReply_->error() != QNetworkReply::NoError)) {
        finalizeDownload();
    }
}

void MainWindow::onDownloadProgress(qint64 received, qint64 total) {
    if (total > 0) {
        int pct = static_cast<int>(received * 100 / total);
        downloadProgressBar_->setRange(0, 100);
        downloadProgressBar_->setValue(pct);
        downloadProgressBar_->setFormat(
            QString("%1% — %2 / %3 KB")
                .arg(pct)
                .arg(received / 1024)
                .arg(total / 1024));
    } else {

        downloadProgressBar_->setRange(0, 0);
        downloadProgressBar_->setFormat(
            QString("Received %1 KB...").arg(received / 1024));
    }
}

void MainWindow::onDownloadFinished() {

    finalizeDownload();
}

void MainWindow::finalizeDownload(bool cancelled) {
    speedTimer_->stop();

    if (!currentReply_)
        return;


    if (!cancelled && readChunkSize_ < 0)
        downloadBuffer_.append(currentReply_->readAll());

    downloadBtn_->setText("Download");

    const QNetworkReply::NetworkError err = currentReply_->error();
    const QString errString = currentReply_->errorString();

    const QVariant clen = currentReply_->header(QNetworkRequest::ContentLengthHeader);
    const qint64   totalExpected = clen.isValid() ? clen.toLongLong() : 0;
    currentReply_->deleteLater();
    currentReply_ = nullptr;


    if (cancelled || err == QNetworkReply::OperationCanceledError) {
        const qint64 got   = downloadBuffer_.size();
        const qint64 total = totalExpected > 0 ? totalExpected : got;
        int pct = total > 0 ? static_cast<int>(got * 100 / total) : 0;
        if (pct > 100) pct = 100;
        downloadProgressBar_->setRange(0, 100);
        downloadProgressBar_->setValue(pct);
        downloadProgressBar_->setFormat(
            QString("Cancelled at %1% — %2 KB").arg(pct).arg(got / 1024));
        statusBar()->showMessage(
            QString("Download cancelled at %1% (%2 KB)").arg(pct).arg(got / 1024), 3000);
        return;
    }

    if (err != QNetworkReply::NoError) {
        downloadProgressBar_->setRange(0, 100);
        downloadProgressBar_->setValue(0);
        downloadProgressBar_->setFormat("Error");
        statusBar()->showMessage("Download failed", 3000);
        QMessageBox::warning(this, "Download Failed", errString);
        return;
    }

    // Success
    downloadProgressBar_->setRange(0, 100);
    downloadProgressBar_->setValue(100);
    const qint64 total = downloadBuffer_.size();
    downloadProgressBar_->setFormat(
        QString("Complete — %1 KB").arg(total / 1024));

    const QString catalogPath =
        QCoreApplication::applicationDirPath() + "/catalog_update.csv";
    QFile catalogFile(catalogPath);
    if (catalogFile.open(QIODevice::WriteOnly)) {
        catalogFile.write(downloadBuffer_);
        catalogFile.close();
        statusBar()->showMessage("Catalog downloaded and saved: catalog_update.csv", 5000);
    }

    constexpr int MAX_DISPLAY = 100 * 1024;
    QString display;
    if (total > MAX_DISPLAY) {
        display = QString::fromUtf8(downloadBuffer_.left(MAX_DISPLAY))
                  + QString("\n\n... [showing first 100 KB of %1 KB total]")
                        .arg(total / 1024);
    } else {
        display = QString::fromUtf8(downloadBuffer_);
    }
    downloadView_->setPlainText(display);

    activityLog_.addEntry(currentUser_.getUsername(), "HTTP_DOWNLOAD",
                          QString("%1 (%2 KB)")
                              .arg(urlEdit_->text())
                              .arg(total / 1024).toStdString());
    statusBar()->showMessage(
        QString("Download complete — %1 KB").arg(total / 1024), 5000);
}

void MainWindow::onRequestStatus() {
    QUdpSocket sock;

    if (!sock.bind(QHostAddress::LocalHost, 0)) {
        QMessageBox::warning(this, "UDP Status", "Failed to bind local UDP socket.");
        return;
    }

    sock.writeDatagram("STATUS", QHostAddress::LocalHost, UDP_PORT); //23118

    if (!sock.waitForReadyRead(3000)) {
        networkResponseView_->setPlainText(
            "Timeout: no UDP response from WarehouseServer.\n"
            "Make sure WarehouseServer.exe is running.");
        return;
    }

    QByteArray datagram(sock.pendingDatagramSize(), Qt::Uninitialized);
    sock.readDatagram(datagram.data(), datagram.size()); //primamo datagram/JSON


    QJsonDocument doc = QJsonDocument::fromJson(datagram);
    QString display;
    if (!doc.isNull()) {
        QJsonObject obj = doc.object();
        display = QString("[UDP STATUS — localhost:%1]\n\n"
                          "Total products : %2\n"
                          "Low stock items: %3\n"
                          "Total value    : $%4")
                      .arg(UDP_PORT)
                      .arg(obj["totalProducts"].toInt())
                      .arg(obj["lowStockCount"].toInt())
                      .arg(obj["totalValue"].toDouble(), 0, 'f', 2);
    } else {
        display = "[UDP STATUS]\n" + QString::fromUtf8(datagram);
    }

    networkResponseView_->setPlainText(display);
    activityLog_.addEntry(currentUser_.getUsername(), "NET_UDP_STATUS", "localhost");
    statusBar()->showMessage("UDP status received from server", 3000);
}

void MainWindow::onSendLogSummary() {
    std::vector<LogEntry> entries = activityLog_.getAllEntries();
    if (entries.empty()) {
        QMessageBox::information(this, "Send Log Summary",
                                 "Activity log is empty — nothing to send.");
        return;
    }


    constexpr int MAX_ENTRIES = 20;
    const int startIdx = std::max(0, static_cast<int>(entries.size()) - MAX_ENTRIES);
    const int count    = static_cast<int>(entries.size()) - startIdx;

    QByteArray payload;
    payload.reserve(4 + count * static_cast<int>(sizeof(LogPacket)));
    payload.append("LOG:");

    for (int i = startIdx; i < static_cast<int>(entries.size()); ++i) {
        LogPacket pkt{};
        std::strncpy(pkt.username, entries[i].username.c_str(), 31);
        std::strncpy(pkt.action,   entries[i].action.c_str(),   31);
        std::strncpy(pkt.target,   entries[i].target.c_str(),   63);
        pkt.timestamp = static_cast<int64_t>(entries[i].timestamp);

        payload.append(reinterpret_cast<const char*>(&pkt), sizeof(LogPacket));
    }

    QUdpSocket sock;
    sock.writeDatagram(payload, QHostAddress::LocalHost, UDP_PORT);

    networkStatusLabel_->setText(
        QString("UDP log sent: %1 entries (%2 bytes)")
            .arg(count).arg(payload.size()));
    networkStatusLabel_->setStyleSheet("color: green;");

    activityLog_.addEntry(currentUser_.getUsername(), "NET_UDP_LOG",
                          "Entries: " + std::to_string(count));
    statusBar()->showMessage(
        QString("Log summary sent via UDP — %1 entries").arg(count), 3000);
}



void MainWindow::setupCryptoTab(QTabWidget* tabs) {
    QWidget*     tab    = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    // Description
    QLabel* infoLabel = new QLabel(
        "AES-256-CBC  ·  OpenSSL EVP API\n"
        "Key: SHA-256(password)   IV: 16-byte random, prepended to ciphertext");
    infoLabel->setStyleSheet("color: gray;");
    layout->addWidget(infoLabel);


    QHBoxLayout* pwRow = new QHBoxLayout();
    pwRow->addWidget(new QLabel("Password:"));
    cryptoPasswordEdit_ = new QLineEdit();
    cryptoPasswordEdit_->setEchoMode(QLineEdit::Password);
    cryptoPasswordEdit_->setPlaceholderText("Enter encryption password…");
    pwRow->addWidget(cryptoPasswordEdit_, 1);

    QCheckBox* showPwCheck = new QCheckBox("Show");
    connect(showPwCheck, &QCheckBox::toggled, this, [this](bool checked) {
        cryptoPasswordEdit_->setEchoMode(
            checked ? QLineEdit::Normal : QLineEdit::Password);
    });
    pwRow->addWidget(showPwCheck);
    layout->addLayout(pwRow);


    QFrame* sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setFrameShadow(QFrame::Sunken);
    layout->addWidget(sep);

    QHBoxLayout* encRow = new QHBoxLayout();
    QPushButton* encBtn = new QPushButton("Encrypt File");
    QLabel* encInfo = new QLabel("stock_snapshot.bin  →  stock_snapshot.bin.enc");
    encInfo->setStyleSheet("color: gray;");
    encRow->addWidget(encBtn);
    encRow->addWidget(encInfo);
    encRow->addStretch();
    layout->addLayout(encRow);

    QHBoxLayout* decRow = new QHBoxLayout();
    QPushButton* decBtn = new QPushButton("Decrypt File");
    QLabel* decInfo = new QLabel("stock_snapshot.bin.enc  →  stock_snapshot.bin.dec");
    decInfo->setStyleSheet("color: gray;");
    decRow->addWidget(decBtn);
    decRow->addWidget(decInfo);
    decRow->addStretch();
    layout->addLayout(decRow);


    QFrame* sep2 = new QFrame();
    sep2->setFrameShape(QFrame::HLine);
    sep2->setFrameShadow(QFrame::Sunken);
    layout->addWidget(sep2);

    cryptoStatusLabel_ = new QLabel("Ready.");
    cryptoStatusLabel_->setWordWrap(true);
    cryptoStatusLabel_->setTextFormat(Qt::RichText);
    cryptoStatusLabel_->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    cryptoStatusLabel_->setStyleSheet("font-family: 'Courier New'; font-size: 9pt;");
    layout->addWidget(cryptoStatusLabel_, 1);

    connect(encBtn, &QPushButton::clicked, this, &MainWindow::onEncryptFile);
    connect(decBtn, &QPushButton::clicked, this, &MainWindow::onDecryptFile);


    QFrame* rsaSep = new QFrame();
    rsaSep->setFrameShape(QFrame::HLine);
    rsaSep->setFrameShadow(QFrame::Sunken);
    layout->addWidget(rsaSep);

    QLabel* rsaTitle = new QLabel("RSA-2048-PKCS1  ·  User Data Export  ·  OpenSSL EVP API");
    rsaTitle->setStyleSheet("color: gray; font-style: italic;");
    layout->addWidget(rsaTitle);


    QHBoxLayout* rsaKeyRow = new QHBoxLayout();
    rsaKeyStatusLabel_ = new QLabel();
    QPushButton* genKeysBtn = new QPushButton("Generate Keys");
    rsaKeyRow->addWidget(rsaKeyStatusLabel_, 1);
    rsaKeyRow->addWidget(genKeysBtn);
    layout->addLayout(rsaKeyRow);

    QHBoxLayout* rsaBtnRow = new QHBoxLayout();
    QPushButton* exportUsersBtn  = new QPushButton("Export Users (RSA Encrypted)");
    QPushButton* decryptUsersBtn = new QPushButton("Decrypt Users Export");
    QLabel* exportInfo = new QLabel("users_export.bin");
    exportInfo->setStyleSheet("color: gray;");
    rsaBtnRow->addWidget(exportUsersBtn);
    rsaBtnRow->addWidget(decryptUsersBtn);
    rsaBtnRow->addWidget(exportInfo);
    rsaBtnRow->addStretch();
    layout->addLayout(rsaBtnRow);


    const bool privExists = QFile::exists("private.pem");
    const bool pubExists  = QFile::exists("public.pem");
    rsaKeyStatusLabel_->setText(
        (privExists && pubExists)
            ? "<span style='color:green;'>&#10003; private.pem / public.pem found</span>"
            : "<span style='color:gray;'>Keys not generated yet — click Generate Keys</span>");

    connect(genKeysBtn,      &QPushButton::clicked, this, &MainWindow::onGenerateRSAKeys);
    connect(exportUsersBtn,  &QPushButton::clicked, this, &MainWindow::onExportUsersRSA);
    connect(decryptUsersBtn, &QPushButton::clicked, this, &MainWindow::onDecryptUsersExport);


    QFrame* sigSep = new QFrame();
    sigSep->setFrameShape(QFrame::HLine);
    sigSep->setFrameShadow(QFrame::Sunken);
    layout->addWidget(sigSep);

    QLabel* sigTitle = new QLabel("RSA-SHA256 Digital Signature  ·  private.pem / public.pem");
    sigTitle->setStyleSheet("color: gray; font-style: italic;");
    layout->addWidget(sigTitle);

    QHBoxLayout* sigBtnRow = new QHBoxLayout();
    QPushButton* signBtn   = new QPushButton("Sign Orders");
    QPushButton* verifyBtn = new QPushButton("Verify Orders");
    QLabel* sigInfo = new QLabel("orders.xml  →  orders.xml.sig");
    sigInfo->setStyleSheet("color: gray;");
    sigBtnRow->addWidget(signBtn);
    sigBtnRow->addWidget(verifyBtn);
    sigBtnRow->addWidget(sigInfo);
    sigBtnRow->addStretch();
    layout->addLayout(sigBtnRow);

    connect(signBtn,   &QPushButton::clicked, this, &MainWindow::onSignOrders);
    connect(verifyBtn, &QPushButton::clicked, this, &MainWindow::onVerifyOrders);

    tabs->addTab(tab, "Crypto");
}

void MainWindow::onGenerateRSAKeys() {
    const bool ok = CryptoManager::generateRSAKeyPair("private.pem", "public.pem");

    if (ok) {
        rsaKeyStatusLabel_->setText(
            "<span style='color:green;'>&#10003; private.pem / public.pem generated</span>");
        cryptoStatusLabel_->setText(
            "<b>RSA key pair generated</b><br>"
            "private.pem — keep this secret<br>"
            "public.pem  — share this for encryption");
        activityLog_.addEntry(currentUser_.getUsername(),
                              "RSA_KEYGEN", "private.pem + public.pem");
        statusBar()->showMessage("RSA-2048 key pair generated", 4000);
    } else {
        rsaKeyStatusLabel_->setText(
            "<span style='color:red;'>Key generation failed — see console</span>");
    }
}

void MainWindow::onExportUsersRSA() {
    if (!QFile::exists("public.pem")) {
        cryptoStatusLabel_->setText(
            "<span style='color:red;'>public.pem not found — generate keys first.</span>");
        return;
    }

    const QByteArray encrypted = CryptoManager::encryptUsersJson("public.pem");
    if (encrypted.isEmpty()) {
        cryptoStatusLabel_->setText(
            "<span style='color:red;'>Export failed — see application log.<br>"
            "Possible cause: too many users (RSA-PKCS1 limit: ~6 users).</span>");
        return;
    }

    QFile out("users_export.bin");
    if (!out.open(QIODevice::WriteOnly)) {
        cryptoStatusLabel_->setText(
            "<span style='color:red;'>Cannot write users_export.bin.</span>");
        return;
    }
    out.write(encrypted);
    out.close();

    cryptoStatusLabel_->setText(
        QString("<span style='color:green;'>&#10003; Users exported<br>"
                "users_export.bin written (%1 B)<br>"
                "Encrypted with RSA-2048-PKCS1 (public.pem)</span>")
            .arg(encrypted.size()));
    activityLog_.addEntry(currentUser_.getUsername(), "USERS_EXPORT_RSA",
                          QString("users_export.bin %1B").arg(encrypted.size()).toStdString());
    statusBar()->showMessage("Users exported to users_export.bin", 4000);
}

void MainWindow::onDecryptUsersExport() {
    if (!QFile::exists("private.pem")) {
        cryptoStatusLabel_->setText(
            "<span style='color:red;'>private.pem not found — generate keys first.</span>");
        return;
    }
    if (!QFile::exists("users_export.bin")) {
        cryptoStatusLabel_->setText(
            "<span style='color:red;'>users_export.bin not found — export users first.</span>");
        return;
    }

    QFile in("users_export.bin");
    if (!in.open(QIODevice::ReadOnly)) {
        cryptoStatusLabel_->setText(
            "<span style='color:red;'>Cannot read users_export.bin.</span>");
        return;
    }
    const QByteArray encrypted = in.readAll();
    in.close();

    const QString json = CryptoManager::decryptUsersJson(encrypted, "private.pem");
    if (json.isEmpty()) {
        cryptoStatusLabel_->setText(
            "<span style='color:red;'>Decryption failed — wrong key or corrupted file.</span>");
        return;
    }

    cryptoStatusLabel_->setText(
        "<span style='color:green;'>&#10003; Decryption successful</span>");
    activityLog_.addEntry(currentUser_.getUsername(), "USERS_DECRYPT_RSA",
                          "users_export.bin decrypted");
    statusBar()->showMessage("users_export.bin decrypted", 4000);

    // Show the JSON in a small scrollable dialog
    QDialog* dlg = new QDialog(this);
    dlg->setWindowTitle("Decrypted Users Export");
    dlg->resize(480, 220);
    QVBoxLayout* layout = new QVBoxLayout(dlg);
    QTextEdit* view = new QTextEdit(dlg);
    view->setReadOnly(true);
    view->setFontFamily("Courier New");
    view->setPlainText(json);
    layout->addWidget(view);
    QPushButton* closeBtn = new QPushButton("Close", dlg);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    layout->addWidget(closeBtn);
    dlg->exec();
}

void MainWindow::onSignOrders() {
    if (!QFile::exists("private.pem")) {
        QMessageBox::warning(this, "Sign Orders",
                             "private.pem not found.\n"
                             "Generate RSA keys first (Generate Keys button).");
        return;
    }

    QFile src("orders.xml");
    if (!src.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Sign Orders",
                             "Cannot open orders.xml.\n"
                             "Create some orders first (Orders tab).");
        return;
    }
    const QByteArray data = src.readAll();
    src.close();

    const QByteArray sig = CryptoManager::signData(data, "private.pem");
    if (sig.isEmpty()) {
        cryptoStatusLabel_->setText(
            "<span style='color:red;'><b>Signing failed</b> — see application log.</span>");
        return;
    }

    QFile sigFile("orders.xml.sig");
    if (!sigFile.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Sign Orders", "Cannot write orders.xml.sig.");
        return;
    }
    sigFile.write(sig);
    sigFile.close();

    cryptoStatusLabel_->setText(
        QString("<b>Orders signed successfully</b><br>"
                "Data       : %1 bytes  (orders.xml)<br>"
                "Signature  : %2 bytes  (orders.xml.sig)<br>"
                "Algorithm  : RSA-SHA256  ·  private.pem<br>"
                "Sig (hex)  : <tt>%3…</tt>")
            .arg(data.size())
            .arg(sig.size())
            .arg(QString::fromLatin1(sig.left(12).toHex())));

    activityLog_.addEntry(currentUser_.getUsername(), "SIGN_ORDERS",
                          QString("data=%1B sig=%2B").arg(data.size()).arg(sig.size()).toStdString());
    statusBar()->showMessage(
        QString("Orders signed — %1 B signature written to orders.xml.sig")
            .arg(sig.size()), 5000);
}

void MainWindow::onVerifyOrders() {
    if (!QFile::exists("public.pem")) {
        QMessageBox::warning(this, "Verify Orders",
                             "public.pem not found.\n"
                             "Generate RSA keys first (Generate Keys button).");
        return;
    }

    QFile src("orders.xml");
    if (!src.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Verify Orders",
                             "Cannot open orders.xml.\n"
                             "Create some orders first.");
        return;
    }
    const QByteArray data = src.readAll();
    src.close();

    QFile sigFile("orders.xml.sig");
    if (!sigFile.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Verify Orders",
                             "Cannot open orders.xml.sig.\n"
                             "Sign the orders first (Sign Orders button).");
        return;
    }
    const QByteArray sig = sigFile.readAll();
    sigFile.close();

    const bool valid = CryptoManager::verifySignature(data, sig, "public.pem");

    if (valid) {
        cryptoStatusLabel_->setText(
            QString("<b><span style='color:green;'>&#10003; Signature VALID</span></b><br>"
                    "Data      : %1 bytes  (orders.xml)<br>"
                    "Signature : %2 bytes  (orders.xml.sig)<br>"
                    "Algorithm : RSA-SHA256  ·  public.pem<br>"
                    "The orders have not been modified since they were signed.")
                .arg(data.size())
                .arg(sig.size()));
        statusBar()->showMessage("Signature verified — orders are authentic", 5000);
    } else {
        cryptoStatusLabel_->setText(
            "<b><span style='color:red;'>&#10007; Signature INVALID</span></b><br>"
            "The orders may have been modified, or the wrong key was used.");
        QMessageBox::warning(this, "Verify Orders",
                             "Signature verification FAILED.\n\n"
                             "The orders file does not match its signature.\n"
                             "The data may have been tampered with, or the public key\n"
                             "does not match the private key used to sign.");
        statusBar()->showMessage("Signature verification FAILED", 5000);
    }

    activityLog_.addEntry(currentUser_.getUsername(), "VERIFY_ORDERS",
                          valid ? "VALID" : "INVALID");
}

void MainWindow::onEncryptFile() {
    const QString password = cryptoPasswordEdit_->text();
    if (password.isEmpty()) {
        QMessageBox::warning(this, "Encrypt", "Please enter a password.");
        return;
    }

    QFile srcFile("stock_snapshot.bin");
    if (!srcFile.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Encrypt",
                             "Cannot open stock_snapshot.bin.\n"
                             "Save a snapshot first (Snapshots tab).");
        return;
    }
    const QByteArray plaintext = srcFile.readAll();
    srcFile.close();

    const QByteArray encrypted = CryptoManager::encryptAES(plaintext, password);
    if (encrypted.isEmpty()) {
        cryptoStatusLabel_->setText(
            "<span style='color:red;'>Encryption failed — see application log.</span>");
        return;
    }

    QFile dstFile("stock_snapshot.bin.enc");
    if (!dstFile.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Encrypt", "Cannot write stock_snapshot.bin.enc.");
        return;
    }
    dstFile.write(encrypted);
    dstFile.close();


    const QString ivHex = encrypted.left(16).toHex(':');

    cryptoStatusLabel_->setText(
        QString("<b>Encrypted successfully</b><br>"
                "Plaintext : %1 bytes<br>"
                "Ciphertext: %2 bytes  (IV 16 B + padded payload)<br>"
                "IV (hex)  : <tt>%3</tt><br>"
                "Output    : stock_snapshot.bin.enc")
            .arg(plaintext.size())
            .arg(encrypted.size())
            .arg(ivHex));

    activityLog_.addEntry(currentUser_.getUsername(), "CRYPTO_ENCRYPT",
                          QString("plain=%1B cipher=%2B")
                              .arg(plaintext.size()).arg(encrypted.size()).toStdString());
    statusBar()->showMessage(
        QString("Encrypted: %1 B → %2 B  (stock_snapshot.bin.enc)")
            .arg(plaintext.size()).arg(encrypted.size()), 5000);
}

void MainWindow::onDecryptFile() {
    const QString password = cryptoPasswordEdit_->text();
    if (password.isEmpty()) {
        QMessageBox::warning(this, "Decrypt", "Please enter the decryption password.");
        return;
    }

    QFile srcFile("stock_snapshot.bin.enc");
    if (!srcFile.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Decrypt",
                             "Cannot open stock_snapshot.bin.enc.\n"
                             "Encrypt a file first.");
        return;
    }
    const QByteArray encrypted = srcFile.readAll();
    srcFile.close();

    const QByteArray plaintext = CryptoManager::decryptAES(encrypted, password);
    if (plaintext.isEmpty()) {
        cryptoStatusLabel_->setText(
            "<span style='color:red;'><b>Decryption failed.</b><br>"
            "Wrong password, corrupted file, or truncated ciphertext.</span>");
        QMessageBox::warning(this, "Decrypt Failed",
                             "Decryption produced no output.\n"
                             "Check that the password is correct and the file is not corrupted.");
        return;
    }

    QFile dstFile("stock_snapshot.bin.dec");
    if (!dstFile.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Decrypt", "Cannot write stock_snapshot.bin.dec.");
        return;
    }
    dstFile.write(plaintext);
    dstFile.close();

    cryptoStatusLabel_->setText(
        QString("<b>Decrypted successfully</b><br>"
                "Ciphertext: %1 bytes<br>"
                "Plaintext : %2 bytes<br>"
                "Output    : stock_snapshot.bin.dec")
            .arg(encrypted.size())
            .arg(plaintext.size()));

    activityLog_.addEntry(currentUser_.getUsername(), "CRYPTO_DECRYPT",
                          QString("cipher=%1B plain=%2B")
                              .arg(encrypted.size()).arg(plaintext.size()).toStdString());
    statusBar()->showMessage(
        QString("Decrypted: %1 B → %2 B  (stock_snapshot.bin.dec)")
            .arg(encrypted.size()).arg(plaintext.size()), 5000);


    const auto restore = QMessageBox::question(
        this, "Restore Backup",
        "Restore backup? This will replace the current stock_snapshot.bin "
        "with the decrypted data.",
        QMessageBox::Yes | QMessageBox::No);

    if (restore == QMessageBox::Yes) {

        if (QFile::exists("stock_snapshot.bin"))
            QFile::remove("stock_snapshot.bin");

        if (QFile::copy("stock_snapshot.bin.dec", "stock_snapshot.bin")) {
            activityLog_.addEntry(currentUser_.getUsername(), "CRYPTO_RESTORE",
                                  "stock_snapshot.bin.dec -> stock_snapshot.bin");
            statusBar()->showMessage(
                "Backup restored: stock_snapshot.bin replaced from decrypted data", 5000);
            onLoadSnapshot();
        } else {
            QMessageBox::warning(this, "Restore Backup",
                                 "Failed to copy stock_snapshot.bin.dec over stock_snapshot.bin.");
        }
    }
}

void MainWindow::onValidateBackup() {
    //lokacija exe
    QString validatorPath = QCoreApplication::applicationDirPath() + "/WarehouseValidator";
#ifdef Q_OS_WIN
    validatorPath += ".exe";
#endif

    //kao zasebni proces
    QProcess proc;
    proc.start(validatorPath, {"stock_snapshot.bin"});

    if (!proc.waitForStarted(3000)) {
        QMessageBox::critical(this, "Validate Backup",
                              "Could not launch WarehouseValidator.\n"
                              "Make sure it is built and located next to WarehouseApp.");
        return;
    }

    proc.waitForFinished(5000);

    if (proc.exitCode() == 0) {
        QMessageBox::information(this, "Validate Backup",
                                 "Backup file is valid.\n\n" +
                                 QString::fromUtf8(proc.readAllStandardOutput()).trimmed());
    } else {
        QString errMsg = QString::fromUtf8(proc.readAllStandardError()).trimmed();
        QMessageBox::warning(this, "Validate Backup",
                             "Backup file is invalid or missing.\n\n" +
                             (errMsg.isEmpty() ? "Unknown error." : errMsg));
    }

    activityLog_.addEntry(currentUser_.getUsername(), "VALIDATE_BACKUP",
                          proc.exitCode() == 0 ? "PASS" : "FAIL");
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


        //saveamo settings iz dijaloga
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

void MainWindow::onAnalyzeInventory() {
    pendingProducts_ = DatabaseManager::instance().getAllProducts();

    if (pendingProducts_.empty()) {
        QMessageBox::information(this, "Analyze Inventory", "No products to analyze.");
        return;
    }

    // resetiramo.
    analysisProcessedCount_ = 0;

    const int total = static_cast<int>(pendingProducts_.size());


    completedAnalyzers_ = 0;
    analyzeInventoryBtn_->setEnabled(false);
    analysisProgressBar_->setRange(0, 0);
    analysisProgressBar_->setFormat("");
    analysisProgressBar_->setVisible(true);
    analysisStatusLabel_->setText("Measuring single-thread baseline (1 thread)...");
    analysisStatusLabel_->setVisible(true);

    QThreadPool::globalInstance()->start(QRunnable::create([this, total]() {

        QElapsedTimer singleTimer;
        singleTimer.start();
        int    singleLowStock = 0;
        double singleValue    = 0.0;
        for (int i = 0; i < total; ++i) {
            if (pendingProducts_[i].getQuantity() < 10) ++singleLowStock;
            singleValue += pendingProducts_[i].getTotalValue();
            {
                volatile double acc = 0.0;
                for (int j = 1; j <= 800000; ++j) {
                    acc += std::sqrt(pendingProducts_[i].getPrice() * j) / (j + 1.0);
                }
            }
        }
        singleThreadTime_ = singleTimer.elapsed();


        QMetaObject::invokeMethod(this, [this]() { startParallelAnalysis(); },
                                  Qt::QueuedConnection);
    }));

}

void MainWindow::startParallelAnalysis() {
    const int total     = static_cast<int>(pendingProducts_.size());
    const int oneThird  = total / 3;
    const int twoThirds = 2 * total / 3;


    {
        std::ofstream file("analysis_result.txt", std::ios::trunc);
        file << "=== Inventory Analysis Log ===\n";
        file << "Date/time    : "
             << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString() << "\n";
        file << "Total products: " << total << "\n";
        file << "Segments     : Thread 1 = [0,"   << oneThird - 1
             << "]  Thread 2 = [" << oneThird << "," << twoThirds - 1
             << "]  Thread 3 = [" << twoThirds << "," << total - 1 << "]\n";
        file << "Mutex types  : QMutex (counter)  |  std::mutex (file)\n\n";
    }


    completedAnalyzers_ = 0;
    analysisProgressBar_->setRange(0, total);
    analysisProgressBar_->setValue(0);
    analysisProgressBar_->setFormat("Analyzing: %v / %m products");
    analysisStatusLabel_->setText("Analysis started — 3 worker threads launched...");


    pendingAnalyzers_[0] = new InventoryAnalyzer(
        &pendingProducts_, 0,         oneThird,  1,
        &analysisCounterMutex_, &analysisProcessedCount_, &analysisFileMutex_, this);
    pendingAnalyzers_[1] = new InventoryAnalyzer(
        &pendingProducts_, oneThird,  twoThirds, 2,
        &analysisCounterMutex_, &analysisProcessedCount_, &analysisFileMutex_, this);
    pendingAnalyzers_[2] = new InventoryAnalyzer(
        &pendingProducts_, twoThirds, total,     3,
        &analysisCounterMutex_, &analysisProcessedCount_, &analysisFileMutex_, this);

    multiThreadTimer_.start();

    for (InventoryAnalyzer* a : pendingAnalyzers_) {
        connect(a, &InventoryAnalyzer::finished,
                this, &MainWindow::onAnalyzerFinished);
        QThreadPool::globalInstance()->start(a);
    }
    if (!analysisProgressTimer_) {
        analysisProgressTimer_ = new QTimer(this);
        connect(analysisProgressTimer_, &QTimer::timeout,
                this, &MainWindow::onAnalysisProgressTick);
    }
    analysisProgressTimer_->start(100);

}

void MainWindow::onAnalysisProgressTick() {

    int current;
    {
        QMutexLocker locker(&analysisCounterMutex_);
        current = analysisProcessedCount_;
    }
    analysisProgressBar_->setValue(current);
    analysisStatusLabel_->setText(
        QString("Analyzing inventory: %1 / %2 products processed...")
            .arg(current).arg(analysisProgressBar_->maximum()));
}

void MainWindow::onAnalyzerFinished(int threadId) {
    Q_UNUSED(threadId);
    ++completedAnalyzers_;

    if (completedAnalyzers_ < 3)
        return;


    if (analysisProgressTimer_)
        analysisProgressTimer_->stop();

    analysisProgressBar_->setValue(analysisProgressBar_->maximum());


    const qint64 multiThreadMs = multiThreadTimer_.elapsed();


    const int finalProcessedCount = analysisProcessedCount_;

    {
        std::ofstream file("analysis_result.txt", std::ios::app);
        file << "=== Summary ===\n";
        file << "  Items counted via QMutex counter : " << finalProcessedCount << "\n";
        file << "  (should equal total products above)\n";
        file << "==============================\n";
    }


    int    totalLowStock = 0;
    double totalValue    = 0.0;
    Product mostExpensive;
    double  maxPrice = -1.0;

    for (InventoryAnalyzer* a : pendingAnalyzers_) {
        totalLowStock += a->lowStockCount;
        totalValue    += a->totalValue;
        if (a->hasResults && a->mostExpensive.getPrice() > maxPrice) {
            maxPrice      = a->mostExpensive.getPrice();
            mostExpensive = a->mostExpensive;
        }
    }

    lastAnalysisRecords_.clear();
    const qint64 snapshotTime = QDateTime::currentSecsSinceEpoch();
    //trazenje najveceg
    for (InventoryAnalyzer* a : pendingAnalyzers_) {
        AnalysisSnapshotRecord rec;
        std::memset(&rec, 0, sizeof(rec));
        rec.threadId      = a->threadId;
        rec.rangeStart    = a->rangeStart;
        rec.rangeEnd      = a->rangeEnd;
        rec.lowStockCount = a->lowStockCount;
        rec.totalValue    = a->totalValue;

        const std::string meName =
            a->hasResults ? a->mostExpensive.getName() : std::string("(none)");
        std::strncpy(rec.mostExpensiveProductName, meName.c_str(),
                     sizeof(rec.mostExpensiveProductName) - 1);
        rec.mostExpensiveProductId    = a->hasResults ? a->mostExpensive.getId() : -1;
        rec.mostExpensiveProductPrice = a->hasResults ? a->mostExpensive.getPrice() : 0.0;
        rec.timestamp = snapshotTime;

        lastAnalysisRecords_.push_back(rec);
    }

    const int total = static_cast<int>(pendingProducts_.size());


    auto segRow = [](const InventoryAnalyzer* a) -> QString {
        if (!a->hasResults)
            return QString("<tr><td align='center'>%1</td><td align='center'>%2–%3</td>"
                           "<td colspan='2' align='center'><i>empty segment</i></td></tr>")
                .arg(a->threadId).arg(a->rangeStart).arg(a->rangeEnd);
        return QString("<tr><td align='center'>%1</td><td align='center'>%2 – %3</td>"
                       "<td align='center'>%4</td><td align='center'>$%5</td></tr>")
            .arg(a->threadId)
            .arg(a->rangeStart).arg(a->rangeEnd - 1)
            .arg(a->lowStockCount)
            .arg(a->totalValue, 0, 'f', 2);
    };


    const double speedup = (multiThreadMs > 0)
        ? static_cast<double>(singleThreadTime_) / static_cast<double>(multiThreadMs)
        : 0.0;
    const QString perfHtml = QString(
        "<hr/>"
        "<h3 style=\"margin-bottom:4px;\">Performance Comparison</h3>"
        "<table width=\"100%\" cellspacing=\"4\">"
        "  <tr><td><b>Single thread:</b></td><td>%1 ms</td></tr>"
        "  <tr><td><b>3 threads:</b></td><td>%2 ms</td></tr>"
        "  <tr><td><b>Speedup:</b></td><td>%3&times; faster</td></tr>"
        "</table>")
        .arg(singleThreadTime_)
        .arg(multiThreadMs)
        .arg(speedup, 0, 'f', 2);

    const QString html = QString(R"(
<h2 style="margin-bottom:4px;">Inventory Analysis</h2>
<p style="color:gray;margin-top:0;"><i>3 segments processed in parallel via QThreadPool + QRunnable</i></p>
<hr/>
<table width="100%" cellspacing="4">
  <tr><td><b>Total products analyzed:</b></td><td>%1</td></tr>
  <tr><td><b>Low-stock items (qty &lt; 10):</b></td><td>%2</td></tr>
  <tr><td><b>Total inventory value:</b></td><td><b>$%3</b></td></tr>
</table>
<hr/>
<h3 style="margin-bottom:4px;">Most Expensive Product</h3>
<table width="100%" cellspacing="4">
  <tr><td><b>Name:</b></td><td>%4</td></tr>
  <tr><td><b>Unit price:</b></td><td>$%5</td></tr>
  <tr><td><b>Quantity:</b></td><td>%6</td></tr>
  <tr><td><b>Category:</b></td><td>%7</td></tr>
</table>
<hr/>
<h3 style="margin-bottom:4px;">Per-Thread Breakdown</h3>
<table width="100%" border="1" cellspacing="0" cellpadding="5">
  <tr style="background:#e0e0e0;">
    <th>Thread</th><th>Index range</th><th>Low stock</th><th>Segment value</th>
  </tr>
  %8%9%10
</table>
%11
)")
        .arg(total)
        .arg(totalLowStock)
        .arg(totalValue, 0, 'f', 2)
        .arg(QString::fromStdString(mostExpensive.getName()))
        .arg(mostExpensive.getPrice(), 0, 'f', 2)
        .arg(mostExpensive.getQuantity())
        .arg(QString::fromStdString(mostExpensive.getCategory().getName()))
        .arg(segRow(pendingAnalyzers_[0]))
        .arg(segRow(pendingAnalyzers_[1]))
        .arg(segRow(pendingAnalyzers_[2]))
        .arg(perfHtml);


    for (InventoryAnalyzer*& a : pendingAnalyzers_) {
        delete a;
        a = nullptr;
    }
    pendingProducts_.clear();


    analysisProgressBar_->setVisible(false);
    analysisStatusLabel_->setVisible(false);
    analyzeInventoryBtn_->setEnabled(true);

    activityLog_.addEntry(currentUser_.getUsername(), "ANALYZE_INVENTORY",
                          "Products: " + std::to_string(total));
    statusBar()->showMessage(
        QString("Analysis complete — %1 products processed, log saved to analysis_result.txt")
            .arg(total), 6000);

    QDialog dlg(this);
    dlg.setWindowTitle("Inventory Analysis Results");
    dlg.setMinimumSize(540, 520);

    QVBoxLayout* dlgLayout = new QVBoxLayout(&dlg);
    QTextEdit* view = new QTextEdit();
    view->setReadOnly(true);
    view->setHtml(html);
    dlgLayout->addWidget(view);

    QPushButton* closeBtn = new QPushButton("Close");
    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    dlgLayout->addWidget(closeBtn);

    dlg.exec();
}


static QJsonArray productsToJson(const std::vector<Product>& products) {
    QJsonArray arr;
    for (const Product& p : products) {
        QJsonObject obj;
        obj["id"]       = p.getId();
        obj["name"]     = QString::fromStdString(p.getName());
        obj["price"]    = p.getPrice();
        obj["quantity"] = p.getQuantity();
        obj["value"]    = p.getTotalValue();
        obj["category"] = QString::fromStdString(p.getCategory().getName());
        obj["supplier"] = QString::fromStdString(p.getSupplier().getCompanyName());
        arr.append(obj);
    }
    return arr;
}

void MainWindow::onExportCSV() {
    if (products_.empty()) {
        QMessageBox::information(this, "Export CSV", "No products to export.");
        return;
    }

    const QString path = QFileDialog::getSaveFileName(
        this, "Export Products as CSV", "products.csv", "CSV files (*.csv)");
    if (path.isEmpty())
        return;

    // iz dll
    const QString csv = exportToCSV(productsToJson(products_));

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Export CSV", "Cannot write to:\n" + path);
        return;
    }
    f.write(csv.toUtf8());
    f.close();

    activityLog_.addEntry(currentUser_.getUsername(), "EXPORT_CSV",
                          QString("%1 products → %2")
                              .arg(products_.size()).arg(path).toStdString());
    statusBar()->showMessage(
        QString("CSV exported: %1 products → %2").arg(products_.size()).arg(path), 5000);
    QMessageBox::information(this, "Export CSV",
                             QString("Exported %1 products.\n\nFile: %2")
                                 .arg(products_.size()).arg(path));
}

void MainWindow::onExportHTML() {
    if (products_.empty()) {
        QMessageBox::information(this, "Export HTML", "No products to export.");
        return;
    }

    const QString path = QFileDialog::getSaveFileName(
        this, "Export Products as HTML", "products.html", "HTML files (*.html)");
    if (path.isEmpty())
        return;


    DataFormatter formatter;
    const QString html = formatter.formatReport(productsToJson(products_), "html");

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Export HTML", "Cannot write to:\n" + path);
        return;
    }
    f.write(html.toUtf8());
    f.close();

    activityLog_.addEntry(currentUser_.getUsername(), "EXPORT_HTML",
                          QString("%1 products → %2")
                              .arg(products_.size()).arg(path).toStdString());
    statusBar()->showMessage(
        QString("HTML exported: %1 products → %2").arg(products_.size()).arg(path), 5000);
    QMessageBox::information(this, "Export HTML",
                             QString("Exported %1 products.\n\nFile: %2")
                                 .arg(products_.size()).arg(path));
}

void MainWindow::onOpenAbout() {
    ShowAboutDialog(this);
}

void MainWindow::onQuickCalc() {
    ShowQuickCalcDialog(this);
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

    jsonManager_->saveLog(activityLog_); //poziv u closeEvent
    event->accept();
}
