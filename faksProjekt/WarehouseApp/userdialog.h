#ifndef USERDIALOG_H
#define USERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <string>

#include "User.h"

class UserDialog : public QDialog {
    Q_OBJECT

public:
    // Add mode — empty fields, password required.
    explicit UserDialog(QWidget* parent = nullptr);
    // Edit mode — fields pre-filled; empty password means keep the existing hash.
    explicit UserDialog(const User& user, QWidget* parent = nullptr);

    // Username, full name and role from the form (id preserved in edit mode).
    // The returned User carries an empty password — the plaintext is fetched
    // separately via getPassword() and hashed by DatabaseManager.
    User getUser() const;
    std::string getPassword() const;
    bool isEditMode() const;

    void updateLanguage();

private slots:
    void onSaveClicked();

private:
    QLineEdit*   usernameEdit_;
    QLineEdit*   fullNameEdit_;
    QLineEdit*   passwordEdit_;
    QComboBox*   roleCombo_;
    QPushButton* saveButton_;
    QPushButton* cancelButton_;
    QFormLayout* formLayout_;

    bool editMode_;
    int  userId_;

    void setupUI();
};

#endif // USERDIALOG_H
