#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QMessageBox>
#include "User.h"


class LoginDialog : public QDialog
{

    Q_OBJECT

public:
    LoginDialog(QWidget* parent = nullptr);

    User getLoggedInUser() const;
    void updateLanguage();

private slots:
    void onLoginClicked();

private:
    QLineEdit* usernameEdit_;
    QLineEdit* passwordEdit_;
    QPushButton* loginButton_;
    User loggedInUser_;
    QLabel* titleLabel_;
    QLabel* userLabel_;
    QLabel* passLabel_;
    void setupUI();
};

#endif // LOGINDIALOG_H

