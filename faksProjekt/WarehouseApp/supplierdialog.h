#ifndef SUPPLIERDIALOG_H
#define SUPPLIERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include "Supplier.h"



class SupplierDialog : public QDialog {
    Q_OBJECT

public:

    SupplierDialog(int id, const QString& title, QWidget* parent = nullptr);


    void setSupplier(const Supplier& supplier);


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