#ifndef LOGDIALOG_H
#define LOGDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <vector>

#include "ActivityLog.h"
#include "jsonmanager.h"

// Single-record form — modelled on OrderFormDialog.
class LogFormDialog : public QDialog {
    Q_OBJECT
public:
    LogFormDialog(int id, const QString& title, QWidget* parent = nullptr);

    void setEntry(const LogEntry& entry);
    LogEntry getEntry() const;

private slots:
    void onSaveClicked();

private:
    QLineEdit*     usernameEdit_;
    QLineEdit*     actionEdit_;
    QLineEdit*     targetEdit_;
    QDateTimeEdit* timestampEdit_;
    QPushButton*   saveButton_;
    QPushButton*   cancelButton_;
    int            id_;

    void setupUI();
};

// List + buttons — modelled on OrderDialog. Owns its own JsonManager and
// demonstrates full CRUD over activity_log.json.
class LogDialog : public QDialog {
    Q_OBJECT
public:
    explicit LogDialog(QWidget* parent = nullptr);

private slots:
    void onAdd();
    void onEdit();
    void onDelete();
    void onClearAll();

private:
    QTableWidget*         table_;
    QPushButton*          addBtn_;
    QPushButton*          editBtn_;
    QPushButton*          deleteBtn_;
    QPushButton*          clearBtn_;
    QPushButton*          closeBtn_;
    JsonManager*          jsonManager_;
    std::vector<LogEntry> entries_;

    void setupUI();
    void refreshTable();
    int  getNextId();
};

#endif // LOGDIALOG_H
