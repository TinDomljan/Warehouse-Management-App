#include "logindialog.h"
#include "translationmanager.h"
#include "settingsmanager.h"
#include "databasemanager.h"

static QString T(const std::string& key) {
    return QString::fromStdString(TranslationManager::instance().tr(key));
}

LoginDialog::LoginDialog(QWidget* parent) : QDialog(parent) {
    setupUI();
    updateLanguage();
    setFixedSize(350, 200);

    // Pre-fill username from last session =====
    QString lastUser = SettingsManager::instance().loadLastUsername();
    if (!lastUser.isEmpty()) {
        usernameEdit_->setText(lastUser);
        passwordEdit_->setFocus();
    }
    //
}

void LoginDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    titleLabel_ = new QLabel();
    titleLabel_->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel_->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel_->setFont(titleFont);
    mainLayout->addWidget(titleLabel_);
    userLabel_ = new QLabel();
    mainLayout->addWidget(userLabel_);
    usernameEdit_ = new QLineEdit();
    mainLayout->addWidget(usernameEdit_);
    passLabel_ = new QLabel();
    mainLayout->addWidget(passLabel_);
    passwordEdit_ = new QLineEdit();
    passwordEdit_->setEchoMode(QLineEdit::Password);
    mainLayout->addWidget(passwordEdit_);
    loginButton_ = new QPushButton();
    mainLayout->addWidget(loginButton_);
    connect(loginButton_, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(passwordEdit_, &QLineEdit::returnPressed, this, &LoginDialog::onLoginClicked);
}

void LoginDialog::updateLanguage() {
    setWindowTitle(T("login_title"));
    titleLabel_->setText(T("login_header"));
    userLabel_->setText(T("login_username"));
    passLabel_->setText(T("login_password"));
    usernameEdit_->setPlaceholderText(T("login_ph_user"));
    passwordEdit_->setPlaceholderText(T("login_ph_pass"));
    loginButton_->setText(T("login_btn"));
}

void LoginDialog::onLoginClicked() {
    std::string username = usernameEdit_->text().toStdString();
    std::string password = passwordEdit_->text().toStdString();
    if (username.empty() || password.empty()) {
        QMessageBox::warning(this, T("login_err_title"), T("login_err_empty"));
        return;
    }
    std::vector<User> users = DatabaseManager::instance().getAllUsers();
    for (int i = 0; i < users.size(); i++) {
        if (users[i].getUsername() == username && users[i].checkPassword(password)) {
            users[i].login(password);
            loggedInUser_ = users[i];
            SettingsManager::instance().saveLastUsername(QString::fromStdString(username));
            accept();
            return;
        }
    }
    QMessageBox::warning(this, T("login_failed_title"), T("login_err_invalid"));
    passwordEdit_->clear();
    passwordEdit_->setFocus();
}

User LoginDialog::getLoggedInUser() const {
    return loggedInUser_;
}