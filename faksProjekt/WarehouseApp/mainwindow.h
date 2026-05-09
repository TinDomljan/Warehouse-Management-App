#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QStatusBar>
#include <vector>
#include "settingsmanager.h"
#include "User.h"
#include "Product.h"
#include "Category.h"
#include "Supplier.h"
#include "ActivityLog.h"
#include "xmlmanager.h"
#include "jsonmanager.h"
#include "binarymanager.h"

class QTabWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);

    void setLoggedInUser(const User& user);
    void updateLanguage();
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onAddProduct();
    void onRemoveProduct();
    void onOpenSettings();
    void onOpenAbout();
    void onLogout();
    void onEditSupplier();
    void onAddSupplier();
    void onDeleteSupplier();
    void onViewLog();
    void onClearLog();
    void onSaveSnapshot();
    void onLoadSnapshot();
    void onGenerateReport();

private:
    // UI elements
    QTableWidget* productTable_;
    QTableWidget* supplierTable_;
    QTableWidget* logTable_;
    QTableWidget* snapshotTable_;
    QLabel*       userInfoLabel_;
    QPushButton*  addProductBtn_;
    QPushButton*  removeProductBtn_;
    QPushButton*  generateReportBtn_;
    QPushButton*  settingsBtn_;
    QPushButton*  aboutBtn_;
    QPushButton*  logoutBtn_;
    QLineEdit*    searchEdit_;
    QComboBox*    sortCombo_;

    // Data
    User currentUser_;
    std::vector<Product> products_;
    std::vector<Category> categories_;
    std::vector<Supplier> suppliers_;
    ActivityLog activityLog_;

    // Managers
    XmlManager* xmlManager_;
    JsonManager* jsonManager_;
    BinaryManager* binaryManager_;

    // Visual settings
    int currentFontSize_;
    QColor currentTextColor_;
    QColor currentBgColor_;
    QString currentLanguage_;

    // Setup
    void setupUI();
    void setupProductsTab(QTabWidget* tabs);
    void setupSnapshotTab(QTabWidget* tabs);
    void setupSuppliersTab(QTabWidget* tabs);
    void setupLogTab(QTabWidget* tabs);
    void setupMenuBar();
    void loadSampleData();

    // Refresh
    void refreshProductTable();
    void refreshSupplierTable();
    void refreshLogTable();
    void applyTableStyle(QTableWidget* table);
};

#endif // MAINWINDOW_H