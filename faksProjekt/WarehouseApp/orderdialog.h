#ifndef ORDERDIALOG_H
#define ORDERDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <vector>
#include "Order.h"
#include "orderxmlmanager.h"


class OrderFormDialog : public QDialog {
    Q_OBJECT
public:
    OrderFormDialog(int id, const QString& title, QWidget* parent = nullptr);

    void setOrder(const Order& order);
    Order getOrder() const;

private slots:
    void onSaveClicked();
    void onAddItem();
    void onRemoveItem();

private:
    QLineEdit*    customerEdit_;
    QDateEdit*    dateEdit_;
    QComboBox*    statusCombo_;
    QTableWidget* itemsTable_;
    QPushButton*  addItemBtn_;
    QPushButton*  removeItemBtn_;
    QPushButton*  saveButton_;
    QPushButton*  cancelButton_;
    int           id_;

    void setupUI();
};

// Main dialog: QTableWidget with Add / Edit / Delete buttons
class OrderDialog : public QDialog {
    Q_OBJECT
public:
    explicit OrderDialog(QWidget* parent = nullptr);

private slots:
    void onAdd();
    void onEdit();
    void onDelete();

private:
    QTableWidget*      table_;
    QPushButton*       addBtn_;
    QPushButton*       editBtn_;
    QPushButton*       deleteBtn_;
    OrderXmlManager*   xmlManager_;
    std::vector<Order> orders_;

    void setupUI();
    void refreshTable();
};

#endif // ORDERDIALOG_H
