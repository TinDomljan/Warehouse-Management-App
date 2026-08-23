#include <QApplication>
#include <QMessageBox>
#include "logindialog.h"
#include "mainwindow.h"
#include "databasemanager.h"
#include "translationmanager.h"
#include "settingsmanager.h"


int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    if (!DatabaseManager::instance().open("warehouse.db")) {
        QMessageBox::critical(nullptr, "Database Error",
                              "Failed to open database:\n" +
                              DatabaseManager::instance().lastError());
        return 1;
    }

    TranslationManager::instance().setLanguage(
        SettingsManager::instance().loadLanguage().toStdString());

    LoginDialog loginDialog;
    if (loginDialog.exec() == QDialog::Accepted) {
        User user = loginDialog.getLoggedInUser();

        MainWindow mainWindow;
        mainWindow.setLoggedInUser(user);
        mainWindow.show();

        return app.exec();
    }

    return 0;
}