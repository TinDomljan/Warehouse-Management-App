#include "reportpreviewdialog.h"
#include "reportmanager.h"

ReportPreviewDialog::ReportPreviewDialog(const QString& html, QWidget* parent)
    : QDialog(parent), html_(html) {
    setupUI();
    browser_->setHtml(html_);
    setWindowTitle("Inventory Report Preview");
    resize(900, 700);
}

void ReportPreviewDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    browser_ = new QTextBrowser();
    browser_->setReadOnly(true);
    mainLayout->addWidget(browser_);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    saveBtn_  = new QPushButton("Save as PDF");
    closeBtn_ = new QPushButton("Close");
    btnLayout->addStretch();
    btnLayout->addWidget(saveBtn_);
    btnLayout->addWidget(closeBtn_);
    mainLayout->addLayout(btnLayout);

    connect(saveBtn_,  &QPushButton::clicked, this, &ReportPreviewDialog::onSavePdf);
    connect(closeBtn_, &QPushButton::clicked, this, &QDialog::accept);
}

void ReportPreviewDialog::onSavePdf() {
    QString path = QFileDialog::getSaveFileName(
        this, "Save Inventory Report",
        "inventory_report.pdf",
        "PDF Files (*.pdf)");

    if (path.isEmpty())
        return;

    if (ReportManager::exportToPDF(html_, path)) {
        lastSavedPath_ = path;
        QMessageBox::information(this, "Report Generated",
                                 "Inventory report saved to:\n" + path);
    } else {
        QMessageBox::warning(this, "Report Error",
                             "Failed to write PDF to:\n" + path);
    }
}

QString ReportPreviewDialog::lastSavedPath() const {
    return lastSavedPath_;
}
