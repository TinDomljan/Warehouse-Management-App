#ifndef SUPPLIERDIALOG_H
#define SUPPLIERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include "Supplier.h"

// SupplierDialog handles both Add and Edit operations
// When editing, existing data is pre-filled via setSupplier()
// MainWindow just opens the dialog and reads the result — no QInputDialog mess

class SupplierDialog : public QDialog {
    Q_OBJECT

public:
    // mode: pass a title like "Add Supplier" or "Edit Supplier"
    SupplierDialog(int id, const QString& title, QWidget* parent = nullptr);

    // Pre-fill fields with existing data (for Edit mode)
    void setSupplier(const Supplier& supplier);

    // Read the result after dialog closes
    Supplier getSupplier() const;

private slots:
    void onSaveClicked();

private:
    QLineEdit* companyEdit_;
    QLineEdit* contactEdit_;
    QLineEdit* emailEdit_;
    QLineEdit* phoneEdit_;
    QLineEdit* addressEdit_;
    QPushButton* saveButton_;
    QPushButton* cancelButton_;

    int id_;

    void setupUI();
};

#endif // SUPPLIERDIALOG_H