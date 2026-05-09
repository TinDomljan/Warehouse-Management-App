#include "settingsmanager.h"

// Singleton instance — created once, lives forever
SettingsManager& SettingsManager::instance() {
    static SettingsManager inst;
    return inst;
}

// Constructor creates two QSettings objects with different backends
SettingsManager::SettingsManager() {
    // INI backend — creates/reads a text file called "settings.ini"
    // QSettings::IniFormat tells Qt to use INI file format
    // The file will appear next to the .exe, professor can open it in Notepad
    iniSettings_ = new QSettings("settings.ini", QSettings::IniFormat);

    // Registry backend — writes to Windows Registry
    // QSettings::NativeFormat on Windows = Registry
    // Creates keys under: HKEY_CURRENT_USER\Software\WarehouseApp\WarehouseApp
    // Professor can verify with regedit.exe
    regSettings_ = new QSettings("WarehouseApp", "WarehouseApp");
}

// ============================================================
// INI FILE — Visual/portable settings
// These are settings that affect how the app looks
// Stored in sections: [Visual] and [Language]
// ============================================================

void SettingsManager::saveVisualSettings(int fontSize, const QColor& textColor,
                                         const QColor& bgColor, const QString& language) {
    // beginGroup creates a section in the INI file: [Visual]
    // Equivalent to professor's TIniFile section parameter
    iniSettings_->beginGroup("Visual");
    iniSettings_->setValue("FontSize", fontSize);

    // Colors stored as hex strings (#RRGGBB) — human readable in the INI file
    iniSettings_->setValue("TextColor", textColor.name());
    iniSettings_->setValue("BgColor", bgColor.name());
    iniSettings_->endGroup();

    // Separate section for language: [Language]
    iniSettings_->beginGroup("Language");
    iniSettings_->setValue("Current", language);
    iniSettings_->endGroup();

    // Force write to disk immediately (don't wait for app close)
    iniSettings_->sync();
}

int SettingsManager::loadFontSize() {
    // value() takes key path and default value
    // "Visual/FontSize" means section "Visual", key "FontSize"
    // .toInt() converts the stored value to integer
    // If key doesn't exist, returns DEFAULT_FONT_SIZE (10)
    return iniSettings_->value("Visual/FontSize", DEFAULT_FONT_SIZE).toInt();
}

QColor SettingsManager::loadTextColor() {
    // Load hex string, convert to QColor
    // Default: black (#000000)
    QString colorStr = iniSettings_->value("Visual/TextColor", "#000000").toString();
    return QColor(colorStr);
}

QColor SettingsManager::loadBgColor() {
    // Default: white (#ffffff)
    QString colorStr = iniSettings_->value("Visual/BgColor", "#ffffff").toString();
    return QColor(colorStr);
}

QString SettingsManager::loadLanguage() {
    return iniSettings_->value("Language/Current", "ENG").toString();
}

// ============================================================
// WINDOWS REGISTRY — Machine-specific settings
// These are settings tied to this specific computer/user
// Stored under HKEY_CURRENT_USER\Software\WarehouseApp
// ============================================================

void SettingsManager::saveWindowSettings(const QPoint& position, const QSize& size,
                                         bool isMaximized) {
    // beginGroup creates a registry subkey: ...\WarehouseApp\Window
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
    // No group needed for top-level values
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