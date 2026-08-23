#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QString>
#include <QColor>
#include <QPoint>
#include <QSize>
#include <QSettings>



class SettingsManager {
public:

    static SettingsManager& instance();


    void saveVisualSettings(int fontSize, const QColor& textColor,
                            const QColor& bgColor, const QString& language);
    int loadFontSize();
    QColor loadTextColor();
    QColor loadBgColor();
    QString loadLanguage();


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

    SettingsManager();


    QSettings* iniSettings_;
    QSettings* regSettings_;

    static const int DEFAULT_FONT_SIZE = 10;
    static const bool DEFAULT_MAXIMIZED = false;
};

#endif