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
#include <QThreadPool>
#include <QRunnable>
#include <QProgressBar>
#include <QElapsedTimer>
#include <QMutex>
#include <QProcess>
#include <QTcpSocket>
#include <QTextEdit>
#include <array>
#include <mutex>
#include <vector>
#include "settingsmanager.h"
#include "User.h"
#include "Product.h"
#include "Category.h"
#include "Supplier.h"
#include "ActivityLog.h"
#include "orderxmlmanager.h"
#include "jsonmanager.h"
#include "binarymanager.h"
#include "inventoryanalyzer.h"

class QTabWidget;
class QNetworkAccessManager;
class QNetworkReply;
class QTimer;

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
    void onQuickCalc();
    void onLogout();
    void onEditSupplier();
    void onAddSupplier();
    void onDeleteSupplier();
    void onViewLog();
    void onClearLog();
    void onSaveSnapshot();
    void onLoadSnapshot();
    void onValidateBackup();
    void onQueryProduct();
    void onSendBackup();
    void onEncryptFile();
    void onDecryptFile();
    void onGenerateRSAKeys();
    void onExportUsersRSA();
    void onDecryptUsersExport();
    void onSignOrders();
    void onVerifyOrders();
    void onFetchExchangeRates();
    void onRequestStatus();
    void onSendLogSummary();
    void onDownload();
    void onDownloadProgress(qint64 received, qint64 total);
    void onDownloadFinished();
    void onReadChunk();
    void onGenerateReport();
    void onAnalyzeInventory();
    void onExportCSV();
    void onExportHTML();
    void onAnalyzerFinished(int threadId);
    void onAnalysisProgressTick();
    void onOpenOrders();

private:
    // UI
    QTabWidget*   tabWidget_;
    QTableWidget* productTable_;
    QTableWidget* supplierTable_;
    QTableWidget* logTable_;
    QTableWidget* snapshotTable_;
    QLabel*       userInfoLabel_;
    QPushButton*  addProductBtn_;
    QPushButton*  removeProductBtn_;
    QPushButton*  generateReportBtn_;
    QPushButton*  analyzeInventoryBtn_;
    QPushButton*  exportCsvBtn_;
    QPushButton*  exportHtmlBtn_;
    QProgressBar* analysisProgressBar_;
    QLabel*       analysisStatusLabel_;
    QPushButton*  settingsBtn_;
    QPushButton*  aboutBtn_;
    QPushButton*  quickCalcBtn_;
    QPushButton*  logoutBtn_;
    QLineEdit*    searchEdit_;
    QComboBox*    sortCombo_;


    QPushButton*  saveSnapshotBtn_;
    QPushButton*  loadSnapshotBtn_;
    QPushButton*  validateBackupBtn_;


    QPushButton*  addSupplierBtn_;
    QPushButton*  editSupplierBtn_;
    QPushButton*  deleteSupplierBtn_;


    QPushButton*  clearLogBtn_;


    QAction*      addProductAction_;
    QAction*      removeProductAction_;


    User currentUser_;
    std::vector<Product> products_;
    std::vector<Category> categories_;
    std::vector<Supplier> suppliers_;
    ActivityLog activityLog_;


    std::vector<Product>              pendingProducts_;
    std::array<InventoryAnalyzer*, 3> pendingAnalyzers_ = {nullptr, nullptr, nullptr};
    int completedAnalyzers_ = 0;


    QElapsedTimer multiThreadTimer_;
    qint64        singleThreadTime_ = 0;


    std::vector<AnalysisSnapshotRecord> lastAnalysisRecords_;


    QMutex     analysisCounterMutex_;
    int        analysisProcessedCount_ = 0;
    std::mutex analysisFileMutex_;


    QTimer*    analysisProgressTimer_ = nullptr;


    OrderXmlManager* orderXmlManager_;
    JsonManager* jsonManager_;
    BinaryManager* binaryManager_;


    int currentFontSize_;
    QColor currentTextColor_;
    QColor currentBgColor_;
    QString currentLanguage_;


    QLineEdit* barcodeEdit_;
    QTextEdit* networkResponseView_;
    QLabel*    networkStatusLabel_;


    QLineEdit* cryptoPasswordEdit_;
    QLabel*    cryptoStatusLabel_;


    QLabel*    rsaKeyStatusLabel_;


    QLineEdit*             urlEdit_;
    QComboBox*             speedLimitCombo_;
    QPushButton*           downloadBtn_;
    QProgressBar*          downloadProgressBar_;
    QTextEdit*             downloadView_;
    QNetworkAccessManager* networkManager_  = nullptr;
    QNetworkReply*         currentReply_    = nullptr;
    QTimer*                speedTimer_      = nullptr;
    QByteArray             downloadBuffer_;
    qint64                 readChunkSize_   = -1;   // -1 = unlimited


    void setupUI();
    void setupProductsTab(QTabWidget* tabs);
    void setupSnapshotTab(QTabWidget* tabs);
    void setupSuppliersTab(QTabWidget* tabs);
    void setupLogTab(QTabWidget* tabs);
    void setupNetworkTab(QTabWidget* tabs);
    void setupCryptoTab(QTabWidget* tabs);
    void setupMenuBar();
    void loadSampleData();


    void startParallelAnalysis();

    void applyRolePermissions();

    void finalizeDownload(bool cancelled = false);


    void refreshProductTable();
    void refreshSupplierTable();
    void refreshLogTable();
    void applyTableStyle(QTableWidget* table);
};

#endif