#include "userdialog.h"
#include "translationmanager.h"

static QString T(const std::string& key) {
    return QString::fromStdString(TranslationManager::instance().tr(key));
}

UserDialog::UserDialog(QWidget* parent)
    : QDialog(parent), editMode_(false), userId_(0) {
    setupUI();
    updateLanguage();
    setFixedSize(400, 260);
}

UserDialog::UserDialog(const User& user, QWidget* parent)
    : QDialog(parent), editMode_(true), userId_(user.getId()) {
    setupUI();

    usernameEdit_->setText(QString::fromStdString(user.getUsername()));
    // Username is the identity the salt is derived from — keep it fixed on edit.
    usernameEdit_->setReadOnly(true);
    fullNameEdit_->setText(QString::fromStdString(user.getFullName()));
    roleCombo_->setCurrentIndex(static_cast<int>(user.getRole()));

    updateLanguage();
    setFixedSize(400, 260);
}

void UserDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    formLayout_ = new QFormLayout();

    usernameEdit_ = new QLineEdit();
    formLayout_->addRow("Username:", usernameEdit_);

    fullNameEdit_ = new QLineEdit();
    formLayout_->addRow("Full Name:", fullNameEdit_);

    passwordEdit_ = new QLineEdit();
    passwordEdit_->setEchoMode(QLineEdit::Password);
    formLayout_->addRow("Password:", passwordEdit_);

    roleCombo_ = new QComboBox();
    // Order matches the UserRole enum: Admin=0, Manager=1, Clerk=2
    roleCombo_->addItem("Admin");
    roleCombo_->addItem("Manager");
    roleCombo_->addItem("Clerk");
    formLayout_->addRow("Role:", roleCombo_);

    mainLayout->addLayout(formLayout_);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    saveButton_ = new QPushButton();
    cancelButton_ = new QPushButton();
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton_);
    buttonLayout->addWidget(cancelButton_);
    mainLayout->addLayout(buttonLayout);

    connect(saveButton_, &QPushButton::clicked, this, &UserDialog::onSaveClicked);
    connect(cancelButton_, &QPushButton::clicked, this, &QDialog::reject);
}

void UserDialog::updateLanguage() {
    setWindowTitle(editMode_ ? T("user_edit_title") : T("user_add_title"));
    usernameEdit_->setPlaceholderText(T("user_ph_username"));
    fullNameEdit_->setPlaceholderText(T("user_ph_fullname"));
    passwordEdit_->setPlaceholderText(
        editMode_ ? T("user_ph_password_edit") : T("user_ph_password"));
    saveButton_->setText(T("user_btn_save"));
    cancelButton_->setText(T("user_btn_cancel"));

    if (auto* label = qobject_cast<QLabel*>(formLayout_->labelForField(usernameEdit_)))
        label->setText(T("user_username"));
    if (auto* label = qobject_cast<QLabel*>(formLayout_->labelForField(fullNameEdit_)))
        label->setText(T("user_fullname"));
    if (auto* label = qobject_cast<QLabel*>(formLayout_->labelForField(passwordEdit_)))
        label->setText(T("user_password"));
    if (auto* label = qobject_cast<QLabel*>(formLayout_->labelForField(roleCombo_)))
        label->setText(T("user_role"));
}

void UserDialog::onSaveClicked() {
    if (usernameEdit_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, T("user_err_title"), T("user_err_username"));
        usernameEdit_->setFocus();
        return;
    }
    if (fullNameEdit_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, T("user_err_title"), T("user_err_fullname"));
        fullNameEdit_->setFocus();
        return;
    }
    if (!editMode_ && passwordEdit_->text().isEmpty()) {
        QMessageBox::warning(this, T("user_err_title"), T("user_err_password"));
        passwordEdit_->setFocus();
        return;
    }

    accept();
}

User UserDialog::getUser() const {
    return User(userId_,
                usernameEdit_->text().trimmed().toStdString(),
                fullNameEdit_->text().trimmed().toStdString(),
                "",  // password carried separately via getPassword()
                static_cast<UserRole>(roleCombo_->currentIndex()));
}

std::string UserDialog::getPassword() const {
    return passwordEdit_->text().toStdString();
}

bool UserDialog::isEditMode() const {
    return editMode_;
}
