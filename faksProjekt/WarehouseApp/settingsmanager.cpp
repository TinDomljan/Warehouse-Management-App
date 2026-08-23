#include "settingsmanager.h"

// Singleton instanca
SettingsManager& SettingsManager::instance() {
    static SettingsManager inst;
    return inst;
}

SettingsManager::SettingsManager() {

    iniSettings_ = new QSettings("settings.ini", QSettings::IniFormat);
    regSettings_ = new QSettings("WarehouseApp", "WarehouseApp");
}


void SettingsManager::saveVisualSettings(int fontSize, const QColor& textColor,
                                         const QColor& bgColor, const QString& language) {

    // kreiramo section [Visual]
    iniSettings_->beginGroup("Visual");
    iniSettings_->setValue("FontSize", fontSize);


    iniSettings_->setValue("TextColor", textColor.name());
    iniSettings_->setValue("BgColor", bgColor.name());
    iniSettings_->endGroup();


    iniSettings_->beginGroup("Language");
    iniSettings_->setValue("Current", language);
    iniSettings_->endGroup();


    iniSettings_->sync();
}

int SettingsManager::loadFontSize() {

    return iniSettings_->value("Visual/FontSize", DEFAULT_FONT_SIZE).toInt();
}

QColor SettingsManager::loadTextColor() {

    QString colorStr = iniSettings_->value("Visual/TextColor", "#000000").toString();
    return QColor(colorStr);
}

QColor SettingsManager::loadBgColor() {

    QString colorStr = iniSettings_->value("Visual/BgColor", "#ffffff").toString();
    return QColor(colorStr);
}

QString SettingsManager::loadLanguage() {
    return iniSettings_->value("Language/Current", "ENG").toString();
}



void SettingsManager::saveWindowSettings(const QPoint& position, const QSize& size,
                                         bool isMaximized) {

    regSettings_->beginGroup("Window");
    regSettings_->setValue("X", position.x());
    regSettings_->setValue("Y", position.y());
    regSettings_->setValue("Width", size.width());
    regSettings_->setValue("Height", size.height());
    regSettings_->setValue("Maximized", isMaximized);
    regSettings_->endGroup();

    regSettings_->sync();
}

void SettingsManager::saveLastUsername(const QString& username) {
    //zadnji username
    regSettings_->setValue("LastUsername", username);
    regSettings_->sync();
}

void SettingsManager::saveLastSortColumn(const QString& columnName) {
    regSettings_->setValue("LastSortColumn", columnName);
    regSettings_->sync();
}

QPoint SettingsManager::loadWindowPosition() {
    int x = regSettings_->value("Window/X", 100).toInt();
    int y = regSettings_->value("Window/Y", 100).toInt();
    return QPoint(x, y);
}

QSize SettingsManager::loadWindowSize() {
    int w = regSettings_->value("Window/Width", 900).toInt();
    int h = regSettings_->value("Window/Height", 600).toInt();
    return QSize(w, h);
}

bool SettingsManager::loadIsMaximized() {
    return regSettings_->value("Window/Maximized", DEFAULT_MAXIMIZED).toBool();
}

QString SettingsManager::loadLastUsername() {
    return regSettings_->value("LastUsername", "").toString();
}

QString SettingsManager::loadLastSortColumn() {
    return regSettings_->value("LastSortColumn", "Name").toString();
}