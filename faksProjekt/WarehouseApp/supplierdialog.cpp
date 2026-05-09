#include "supplierdialog.h"

SupplierDialog::SupplierDialog(int id, const QString& title, QWidget* parent)
    : QDialog(parent), id_(id) {
    setupUI();
    setWindowTitle(title);
    setFixedSize(420, 280);
}

void SupplierDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QFormLayout* formLayout = new QFormLayout();

    companyEdit_ = new QLineEdit();
    companyEdit_->setPlaceholderText("Enter company name...");
    formLayout->addRow("Company:", companyEdit_);

    contactEdit_ = new QLineEdit();
    contactEdit_->setPlaceholderText("Enter contact person...");
    formLayout->addRow("Contact:", contactEdit_);

    emailEdit_ = new QLineEdit();
    emailEdit_->setPlaceholderText("Enter email...");
    formLayout->addRow("Email:", emailEdit_);

    phoneEdit_ = new QLineEdit();
    phoneEdit_->setPlaceholderText("Enter phone...");
    formLayout->addRow("Phone:", phoneEdit_);

    addressEdit_ = new QLineEdit();
    addressEdit_->setPlaceholderText("Enter address...");
    formLayout->addRow("Address:", addressEdit_);

    mainLayout->addLayout(formLayout);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    saveButton_ = new QPushButton("Save");
    cancelButton_ = new QPushButton("Cancel");
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton_);
    buttonLayout->addWidget(cancelButton_);
    mainLayout->addLayout(buttonLayout);

    connect(saveButton_, &QPushButton::clicked, this, &SupplierDialog::onSaveClicked);
    connect(cancelButton_, &QPushButton::clicked, this, &QDialog::reject);
}

// Pre-fill fields for Edit mode
void SupplierDialog::setSupplier(const Supplier& supplier) {
    companyEdit_->setText(QString::fromStdString(supplier.getCompanyName()));
    contactEdit_->setText(QString::fromStdString(supplier.getContactPerson()));
    emailEdit_->setText(QString::fromStdString(supplier.getEmail()));
    phoneEdit_->setText(QString::fromStdString(supplier.getPhone()));
    addressEdit_->setText(QString::fromStdString(supplier.getAddress()));
}

void SupplierDialog::onSaveClicked() {
    // Validate — at minimum, company name is required
    if (companyEdit_->text().isEmpty()) {
        QMessageBox::warning(this, "Validation Error",
                             "Please enter a company name.");
        companyEdit_->setFocus();
        return;
    }

    accept();
}

// Build a Supplier object from the form fields
Supplier SupplierDialog::getSupplier() const {
    return Supplier(
        id_,
        companyEdit_->text().toStdString(),
        contactEdit_->text().toStdString(),
        emailEdit_->text().toStdString(),
        phoneEdit_->text().toStdString(),
        addressEdit_->text().toStdString()
        );
}