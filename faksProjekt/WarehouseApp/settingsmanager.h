#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QString>
#include <QColor>
#include <QPoint>
#include <QSize>
#include <QSettings>

// SettingsManager handles two separate storage backends:
// 1. INI file — stores visual/portable settings (font, colors, language)
// 2. Windows Registry — stores machine-specific settings (window position, user prefs)
//
// This separation is required by the project rubric (Feature #4):
// "svaki od njih treba demonstrirati nad zasebnim (različitim) nizom podataka"

class SettingsManager {
public:
    // Singleton — only one instance exists, accessible from anywhere
    static SettingsManager& instance();

    // ========================================
    // INI FILE OPERATIONS
    // Stores: font size, text color, bg color, language
    // File location: settings.ini (next to the .exe)
    // ========================================
    void saveVisualSettings(int fontSize, const QColor& textColor,
                            const QColor& bgColor, const QString& language);
    int loadFontSize();
    QColor loadTextColor();
    QColor loadBgColor();
    QString loadLanguage();

    // ========================================
    // WINDOWS REGISTRY OPERATIONS
    // Stores: window position, window size, maximized state,
    //         last username, last sort column
    // Registry path: HKEY_CURRENT_USER\Software\WarehouseApp
    // ========================================
    void saveWindowSettings(const QPoint& position, const QSize& size,
                            bool isMaximized);
    void saveLastUsername(const QString& username);
    void saveLastSortColumn(const QString& columnName);

    QPoint loadWindowPosition();
    QSize loadWindowSize();
    bool loadIsMaximized();
    QString loadLastUsername();
    QString loadLastSortColumn();

private:
    // Private constructor — nobody can create instances directly
    // Access only through instance()
    SettingsManager();

    // Two separate QSettings objects for two storage backends
    // iniSettings_ writes to a text file (settings.ini)
    // regSettings_ writes to Windows Registry (HKEY_CURRENT_USER\Software\WarehouseApp)
    QSettings* iniSettings_;
    QSettings* regSettings_;

    // Default values — returned when no saved setting exists
    static const int DEFAULT_FONT_SIZE = 10;
    static const bool DEFAULT_MAXIMIZED = false;
};

#endif // SETTINGSMANAGER_H