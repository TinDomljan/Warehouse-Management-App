#include "productdialog.h"
#include "translationmanager.h"

static QString T(const std::string& key) {
    return QString::fromStdString(TranslationManager::instance().tr(key));
}

ProductDialog::ProductDialog(const std::vector<Category>& categories,
                             const std::vector<Supplier>& suppliers,
                             int nextId,
                             QWidget* parent)
    : QDialog(parent), categories_(categories), suppliers_(suppliers), nextId_(nextId) {
    setupUI();
    updateLanguage();
    setFixedSize(400, 300);
}

void ProductDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    formLayout_ = new QFormLayout();

    nameEdit_ = new QLineEdit();
    formLayout_->addRow("", nameEdit_);

    priceSpinBox_ = new QDoubleSpinBox();
    priceSpinBox_->setRange(0.01, 99999.99);
    priceSpinBox_->setDecimals(2);
    priceSpinBox_->setPrefix("€ ");
    priceSpinBox_->setValue(0.01);
    formLayout_->addRow("", priceSpinBox_);

    quantitySpinBox_ = new QSpinBox();
    quantitySpinBox_->setRange(0, 99999);
    quantitySpinBox_->setValue(1);
    formLayout_->addRow("", quantitySpinBox_);

    categoryCombo_ = new QComboBox();
    for (int i = 0; i < categories_.size(); i++) {
        categoryCombo_->addItem(QString::fromStdString(categories_[i].getName()));
    }
    formLayout_->addRow("", categoryCombo_);

    supplierCombo_ = new QComboBox();
    for (int i = 0; i < suppliers_.size(); i++) {
        supplierCombo_->addItem(QString::fromStdString(suppliers_[i].getCompanyName()));
    }
    formLayout_->addRow("", supplierCombo_);

    mainLayout->addLayout(formLayout_);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    saveButton_ = new QPushButton();
    cancelButton_ = new QPushButton();
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton_);
    buttonLayout->addWidget(cancelButton_);
    mainLayout->addLayout(buttonLayout);

    connect(saveButton_, &QPushButton::clicked, this, &ProductDialog::onSaveClicked);
    connect(cancelButton_, &QPushButton::clicked, this, &QDialog::reject);
}

void ProductDialog::updateLanguage() {
    setWindowTitle(T("product_title"));
    nameEdit_->setPlaceholderText(T("product_ph_name"));
    saveButton_->setText(T("product_btn_save"));
    cancelButton_->setText(T("product_btn_cancel"));

    // Update form labels
    formLayout_->labelForField(nameEdit_)->setProperty("text", T("product_name"));
    if (auto* label = qobject_cast<QLabel*>(formLayout_->labelForField(nameEdit_)))
        label->setText(T("product_name"));
    if (auto* label = qobject_cast<QLabel*>(formLayout_->labelForField(priceSpinBox_)))
        label->setText(T("product_price"));
    if (auto* label = qobject_cast<QLabel*>(formLayout_->labelForField(quantitySpinBox_)))
        label->setText(T("product_qty"));
    if (auto* label = qobject_cast<QLabel*>(formLayout_->labelForField(categoryCombo_)))
        label->setText(T("product_category"));
    if (auto* label = qobject_cast<QLabel*>(formLayout_->labelForField(supplierCombo_)))
        label->setText(T("product_supplier"));
}

void ProductDialog::onSaveClicked() {
    std::string name = nameEdit_->text().toStdString();

    if (name.empty()) {
        QMessageBox::warning(this, T("product_err_title"), T("product_err_name"));
        nameEdit_->setFocus();
        return;
    }

    double price = priceSpinBox_->value();
    int quantity = quantitySpinBox_->value();
    int categoryIndex = categoryCombo_->currentIndex();
    int supplierIndex = supplierCombo_->currentIndex();

    product_ = Product(nextId_, name, price, quantity,
                       categories_[categoryIndex],
                       suppliers_[supplierIndex]);

    accept();
}

Product ProductDialog::getProduct() const {
    return product_;
}
