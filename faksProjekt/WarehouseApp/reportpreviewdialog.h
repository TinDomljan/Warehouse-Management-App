#ifndef REPORTPREVIEWDIALOG_H
#define REPORTPREVIEWDIALOG_H

#include <QDialog>
#include <QTextBrowser>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QString>

// Renders the inventory-report HTML on screen (QTextBrowser) with a
// "Save as PDF" button, so the report is previewed inside the app before
// being written to disk.
class ReportPreviewDialog : public QDialog {
    Q_OBJECT

public:
    explicit ReportPreviewDialog(const QString& html, QWidget* parent = nullptr);

    QString lastSavedPath() const;   // empty if the user never saved

private slots:
    void onSavePdf();

private:
    QTextBrowser* browser_;
    QPushButton*  saveBtn_;
    QPushButton*  closeBtn_;

    QString html_;
    QString lastSavedPath_;

    void setupUI();
};

#endif // REPORTPREVIEWDIALOG_H
