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

ProductDialog::ProductDialog(const std::vector<Category>& categories,
                             const std::vector<Supplier>& suppliers,
                             const Product& existing,
                             QWidget* parent)
    : QDialog(parent), categories_(categories), suppliers_(suppliers),
      product_(existing), nextId_(existing.getId()) {
    editMode_ = true;
    setupUI();
    updateLanguage();
    setWindowTitle(T("pd_manage_title"));
    setFixedSize(420, 400);

    // Popuni polja postojecim vrijednostima
    nameEdit_->setText(QString::fromStdString(existing.getName()));
    priceSpinBox_->setValue(existing.getPrice());
    quantitySpinBox_->setValue(existing.getQuantity());

    for (int i = 0; i < (int)categories_.size(); ++i)
        if (categories_[i].getId() == existing.getCategory().getId())
            categoryCombo_->setCurrentIndex(i);

    for (int i = 0; i < (int)suppliers_.size(); ++i)
        if (suppliers_[i].getId() == existing.getSupplier().getId())
            supplierCombo_->setCurrentIndex(i);

    // Kolicina se NE moze rucno upisati u edit modu — mijenja se
    // iskljucivo kroz Product::addStock i Product::removeStock
    quantitySpinBox_->setReadOnly(true);
    quantitySpinBox_->setButtonSymbols(QAbstractSpinBox::NoButtons);
}

void ProductDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    formLayout_ = new QFormLayout();

    nameEdit_ = new QLineEdit();
    formLayout_->addRow("Name:", nameEdit_);

    priceSpinBox_ = new QDoubleSpinBox();
    priceSpinBox_->setRange(0.01, 99999.99);
    priceSpinBox_->setDecimals(2);
    priceSpinBox_->setPrefix("€ ");
    priceSpinBox_->setValue(0.01);
    formLayout_->addRow("Price:", priceSpinBox_);

    quantitySpinBox_ = new QSpinBox();
    quantitySpinBox_->setRange(0, 99999);
    quantitySpinBox_->setValue(1);
    formLayout_->addRow("Quantity:", quantitySpinBox_);

    categoryCombo_ = new QComboBox();
    for (int i = 0; i < (int)categories_.size(); i++) {
        categoryCombo_->addItem(QString::fromStdString(categories_[i].getName()));
    }
    formLayout_->addRow("Category:", categoryCombo_);

    supplierCombo_ = new QComboBox();
    for (int i = 0; i < (int)suppliers_.size(); i++) {
        supplierCombo_->addItem(QString::fromStdString(suppliers_[i].getCompanyName()));
    }
    formLayout_->addRow("Supplier:", supplierCombo_);

    mainLayout->addLayout(formLayout_);

    // Stock in/out — samo u edit modu, koristi Product::addStock/removeStock
    if (editMode_) {
        QHBoxLayout* stockLayout = new QHBoxLayout();
        stockLabel_ = new QLabel(T("pd_stock_change"));
        stockAmountSpin_ = new QSpinBox();
        stockAmountSpin_->setRange(1, 99999);
        stockAmountSpin_->setValue(1);
        stockInButton_  = new QPushButton(T("pd_stock_in"));
        stockOutButton_ = new QPushButton(T("pd_stock_out"));
        stockLayout->addWidget(stockLabel_);
        stockLayout->addWidget(stockAmountSpin_, 1);
        stockLayout->addWidget(stockInButton_);
        stockLayout->addWidget(stockOutButton_);
        mainLayout->addLayout(stockLayout);

        connect(stockInButton_,  &QPushButton::clicked, this, &ProductDialog::onStockIn);
        connect(stockOutButton_, &QPushButton::clicked, this, &ProductDialog::onStockOut);
    }

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    saveButton_ = new QPushButton();
    cancelButton_ = new QPushButton();

    // Delete — samo u edit modu, lijevo od stretcha
    if (editMode_) {
        deleteButton_ = new QPushButton(T("pd_delete"));
        deleteButton_->setStyleSheet("color: white; background-color: #c0392b; padding: 4px 12px;");
        buttonLayout->addWidget(deleteButton_);
        connect(deleteButton_, &QPushButton::clicked, this, &ProductDialog::onDeleteClicked);
    }

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
    int quantity = editMode_ ? product_.getQuantity() : quantitySpinBox_->value();
    int categoryIndex = categoryCombo_->currentIndex();
    int supplierIndex = supplierCombo_->currentIndex();

    if (categoryIndex < 0 || categoryIndex >= (int)categories_.size()) {
        QMessageBox::warning(this, T("product_err_title"), "No category available. Please add a category first.");
        return;
    }
    if (supplierIndex < 0 || supplierIndex >= (int)suppliers_.size()) {
        QMessageBox::warning(this, T("product_err_title"), "No supplier available. Please add a supplier first.");
        return;
    }

    product_ = Product(nextId_, name, price, quantity,
                       categories_[categoryIndex],
                       suppliers_[supplierIndex]);

    accept();
}

Product ProductDialog::getProduct() const {
    return product_;
}

void ProductDialog::onStockIn() {
    int amount = stockAmountSpin_->value();
    product_.addStock(amount);
    quantitySpinBox_->setValue(product_.getQuantity());
}

void ProductDialog::onStockOut() {
    int amount = stockAmountSpin_->value();
    int before = product_.getQuantity();
    product_.removeStock(amount);
    if (product_.getQuantity() == before) {
        QMessageBox::warning(this, "Stock Out",
            QString("Rejected: cannot remove %1 units, only %2 in stock.")
                .arg(amount).arg(before));
        return;
    }
    quantitySpinBox_->setValue(product_.getQuantity());
}

void ProductDialog::onDeleteClicked() {
    int reply = QMessageBox::warning(this, "Delete Product",
        QString("Permanently delete \"%1\"?\n\nThis cannot be undone.")
            .arg(nameEdit_->text()),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (reply != QMessageBox::Yes)
        return;
    deleteRequested_ = true;
    accept();
}

bool ProductDialog::isDeleteRequested() const {
    return deleteRequested_;
}
