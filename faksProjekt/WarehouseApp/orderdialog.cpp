#include "orderdialog.h"
#include "translationmanager.h"

#include <QDate>
#include <QLabel>


static QString T(const std::string& key) {
    return QString::fromStdString(TranslationManager::instance().tr(key));
}



OrderFormDialog::OrderFormDialog(int id, const QString& title, QWidget* parent)
    : QDialog(parent), id_(id) {
    setupUI();
    setWindowTitle(title);
    resize(520, 420);
}

void OrderFormDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QFormLayout* formLayout = new QFormLayout();

    customerEdit_ = new QLineEdit();
    customerEdit_->setPlaceholderText(T("order_ph_customer"));
    formLayout->addRow(T("order_customer"), customerEdit_);

    dateEdit_ = new QDateEdit();
    dateEdit_->setCalendarPopup(true);
    dateEdit_->setDisplayFormat("yyyy-MM-dd");
    dateEdit_->setDate(QDate::currentDate());
    formLayout->addRow(T("order_date"), dateEdit_);

    statusCombo_ = new QComboBox();
    statusCombo_->addItems({"PENDING", "SHIPPED", "DELIVERED", "CANCELLED"});
    formLayout->addRow(T("order_status"), statusCombo_);

    mainLayout->addLayout(formLayout);


    mainLayout->addWidget(new QLabel(T("order_items")));

    itemsTable_ = new QTableWidget(0, 3);
    itemsTable_->setHorizontalHeaderLabels(
        {T("order_item_col_pid"), T("order_item_col_pname"), T("order_item_col_qty")});
    itemsTable_->horizontalHeader()->setStretchLastSection(true);
    itemsTable_->verticalHeader()->setVisible(false);
    mainLayout->addWidget(itemsTable_);

    QHBoxLayout* itemBtnLayout = new QHBoxLayout();
    addItemBtn_    = new QPushButton(T("order_item_add"));
    removeItemBtn_ = new QPushButton(T("order_item_remove"));
    itemBtnLayout->addWidget(addItemBtn_);
    itemBtnLayout->addWidget(removeItemBtn_);
    itemBtnLayout->addStretch();
    mainLayout->addLayout(itemBtnLayout);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    saveButton_   = new QPushButton(T("order_btn_save"));
    cancelButton_ = new QPushButton(T("order_btn_cancel"));
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton_);
    buttonLayout->addWidget(cancelButton_);
    mainLayout->addLayout(buttonLayout);

    connect(addItemBtn_,    &QPushButton::clicked, this, &OrderFormDialog::onAddItem);
    connect(removeItemBtn_, &QPushButton::clicked, this, &OrderFormDialog::onRemoveItem);
    connect(saveButton_,    &QPushButton::clicked, this, &OrderFormDialog::onSaveClicked);
    connect(cancelButton_,  &QPushButton::clicked, this, &QDialog::reject);
}

void OrderFormDialog::onAddItem() {
    int row = itemsTable_->rowCount();
    itemsTable_->insertRow(row);
    itemsTable_->setItem(row, 0, new QTableWidgetItem("0"));
    itemsTable_->setItem(row, 1, new QTableWidgetItem(""));
    itemsTable_->setItem(row, 2, new QTableWidgetItem("1"));
}

void OrderFormDialog::onRemoveItem() {
    int row = itemsTable_->currentRow();
    if (row >= 0)
        itemsTable_->removeRow(row);
}

void OrderFormDialog::setOrder(const Order& order) {
    customerEdit_->setText(QString::fromStdString(order.getCustomerName()));

    QDate date = QDate::fromString(QString::fromStdString(order.getOrderDate()), "yyyy-MM-dd");
    if (date.isValid())
        dateEdit_->setDate(date);

    int idx = statusCombo_->findText(QString::fromStdString(order.getStatus()));
    if (idx >= 0)
        statusCombo_->setCurrentIndex(idx);

    itemsTable_->setRowCount(0);
    for (const auto& item : order.getItems()) {
        int row = itemsTable_->rowCount();
        itemsTable_->insertRow(row);
        itemsTable_->setItem(row, 0, new QTableWidgetItem(QString::number(item.productId)));
        itemsTable_->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(item.productName)));
        itemsTable_->setItem(row, 2, new QTableWidgetItem(QString::number(item.quantity)));
    }
}

Order OrderFormDialog::getOrder() const {
    std::vector<OrderItem> items;
    for (int row = 0; row < itemsTable_->rowCount(); row++) {
        QTableWidgetItem* pidItem  = itemsTable_->item(row, 0);
        QTableWidgetItem* nameItem = itemsTable_->item(row, 1);
        QTableWidgetItem* qtyItem  = itemsTable_->item(row, 2);

        int productId       = pidItem  ? pidItem->text().toInt() : 0;
        std::string pName   = nameItem ? nameItem->text().toStdString() : "";
        int quantity        = qtyItem  ? qtyItem->text().toInt() : 0;

        items.push_back(OrderItem(productId, pName, quantity));
    }

    return Order(
        id_,
        customerEdit_->text().toStdString(),
        dateEdit_->date().toString("yyyy-MM-dd").toStdString(),
        statusCombo_->currentText().toStdString(),
        items
    );
}

void OrderFormDialog::onSaveClicked() {
    if (customerEdit_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, T("order_err_title"), T("order_err_customer"));
        customerEdit_->setFocus();
        return;
    }

    // Validate every item has quantity > 0
    for (int row = 0; row < itemsTable_->rowCount(); row++) {
        QTableWidgetItem* qtyItem = itemsTable_->item(row, 2);
        int quantity = qtyItem ? qtyItem->text().toInt() : 0;
        if (quantity <= 0) {
            QMessageBox::warning(this, T("order_err_title"), T("order_err_items"));
            return;
        }
    }

    accept();
}

// ============================================================
// OrderDialog — table list with Add / Edit / Delete
// ============================================================

OrderDialog::OrderDialog(QWidget* parent)
    : QDialog(parent) {
    xmlManager_ = new OrderXmlManager("orders.xml");
    setupUI();
    setWindowTitle(T("order_title"));
    resize(750, 450);
    refreshTable();
}

void OrderDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    table_ = new QTableWidget(0, 5);
    table_->setHorizontalHeaderLabels(
        {T("order_col_id"), T("order_col_customer"), T("order_col_date"),
         T("order_col_status"), T("order_col_items")});
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->verticalHeader()->setVisible(false);
    mainLayout->addWidget(table_);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    addBtn_    = new QPushButton(T("order_btn_add"));
    editBtn_   = new QPushButton(T("order_btn_edit"));
    deleteBtn_ = new QPushButton(T("order_btn_delete"));
    btnLayout->addStretch();
    btnLayout->addWidget(addBtn_);
    btnLayout->addWidget(editBtn_);
    btnLayout->addWidget(deleteBtn_);
    mainLayout->addLayout(btnLayout);

    connect(addBtn_,    &QPushButton::clicked, this, &OrderDialog::onAdd);
    connect(editBtn_,   &QPushButton::clicked, this, &OrderDialog::onEdit);
    connect(deleteBtn_, &QPushButton::clicked, this, &OrderDialog::onDelete);
}

void OrderDialog::refreshTable() {
    orders_ = xmlManager_->loadOrders();

    table_->setRowCount(0);
    for (int i = 0; i < (int)orders_.size(); i++) {
        int row = table_->rowCount();
        table_->insertRow(row);
        table_->setItem(row, 0, new QTableWidgetItem(QString::number(orders_[i].getId())));
        table_->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(orders_[i].getCustomerName())));
        table_->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(orders_[i].getOrderDate())));
        table_->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(orders_[i].getStatus())));
        table_->setItem(row, 4, new QTableWidgetItem(QString::number((int)orders_[i].getItems().size())));
    }

    table_->resizeColumnsToContents();
    table_->horizontalHeader()->setStretchLastSection(true);
}

void OrderDialog::onAdd() {
    int nextId = xmlManager_->getNextId();
    OrderFormDialog dlg(nextId, T("order_add_title"), this);
    if (dlg.exec() == QDialog::Accepted) {
        xmlManager_->addOrder(dlg.getOrder());
        refreshTable();
    }
}

void OrderDialog::onEdit() {
    int currentRow = table_->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, T("order_edit_title"), T("order_select_edit"));
        return;
    }

    OrderFormDialog dlg(orders_[currentRow].getId(), T("order_edit_title"), this);
    dlg.setOrder(orders_[currentRow]);
    if (dlg.exec() == QDialog::Accepted) {
        xmlManager_->updateOrder(dlg.getOrder());
        refreshTable();
    }
}

void OrderDialog::onDelete() {
    int currentRow = table_->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, T("order_delete_title"), T("order_select_delete"));
        return;
    }

    QString customer = QString::fromStdString(orders_[currentRow].getCustomerName());
    auto reply = QMessageBox::question(this, T("order_delete_title"),
        T("order_confirm_delete") + "\"" + customer + "\"?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        xmlManager_->deleteOrder(orders_[currentRow].getId());
        refreshTable();
    }
}
