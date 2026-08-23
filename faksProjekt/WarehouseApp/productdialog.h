#ifndef PRODUCTDIALOG_H
#define PRODUCTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <vector>

#include "Product.h"
#include "Category.h"
#include "Supplier.h"

class ProductDialog : public QDialog {
    Q_OBJECT

public:
    ProductDialog(const std::vector<Category>& categories,
                  const std::vector<Supplier>& suppliers,
                  int nextId,
                  QWidget* parent = nullptr);

    Product getProduct() const;
    void updateLanguage();


private slots:
    void onSaveClicked();


private:
    QLineEdit* nameEdit_;
    QDoubleSpinBox* priceSpinBox_;
    QSpinBox* quantitySpinBox_;
    QComboBox* categoryCombo_;
    QComboBox* supplierCombo_;
    QPushButton* saveButton_;
    QPushButton* cancelButton_;
    QLabel* titleLabel_;
    QLabel* userLabel_;
    QLabel* passLabel_;
    QFormLayout* formLayout_;
    std::vector<Category> categories_;
    std::vector<Supplier> suppliers_;
    Product product_;
    int nextId_;

    void setupUI();
};

#endif // PRODUCTDIALOG_H