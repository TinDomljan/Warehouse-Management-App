#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QColorDialog>
#include <QColor>
#include <QString>

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    SettingsDialog(int currentFontSize, const QColor& currentColor,
                   const QColor& currentBgColor,
                   const QString& currentLanguage, QWidget* parent = nullptr);

    int getFontSize() const;
    QColor getTextColor() const;
    QColor getBgColor() const;
    QString getLanguage() const;
    void updateLanguage();

private slots:
    void onChooseColor();
    void onChooseBgColor();
    void onApplyClicked();

private:
    QSpinBox* fontSizeSpinBox_;
    QPushButton* colorButton_;
    QComboBox* languageCombo_;
    QPushButton* applyButton_;
    QPushButton* cancelButton_;
    QLabel* previewLabel_;
    void updateBgColorButton();
    QPushButton* bgColorButton_;
    QColor selectedBgColor_;
    QLabel* titleLabel_;
    QLabel* userLabel_;
    QLabel* passLabel_;
    QFormLayout* formLayout_;
    QColor selectedColor_;

    void setupUI();
    void updateColorButton();
    void updatePreview();
};

#endif // SETTINGSDIALOG_H