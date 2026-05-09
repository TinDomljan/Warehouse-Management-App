#include "settingsdialog.h"
#include "translationmanager.h"

static QString T(const std::string& key) {
    return QString::fromStdString(TranslationManager::instance().tr(key));
}

SettingsDialog::SettingsDialog(int currentFontSize, const QColor& currentColor,
                               const QColor& currentBgColor,
                               const QString& currentLanguage, QWidget* parent)
    : QDialog(parent), selectedColor_(currentColor), selectedBgColor_(currentBgColor) {
    setupUI();
    setFixedSize(380, 330);

    fontSizeSpinBox_->setValue(currentFontSize);
    updateColorButton();
    updateBgColorButton();

    int langIndex = languageCombo_->findText(currentLanguage);
    if (langIndex >= 0) {
        languageCombo_->setCurrentIndex(langIndex);
    }

    updateLanguage();
    updatePreview();
}

void SettingsDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    formLayout_ = new QFormLayout();

    fontSizeSpinBox_ = new QSpinBox();
    fontSizeSpinBox_->setRange(8, 24);
    fontSizeSpinBox_->setSuffix(" pt");
    formLayout_->addRow("", fontSizeSpinBox_);

    colorButton_ = new QPushButton();
    formLayout_->addRow("", colorButton_);

    bgColorButton_ = new QPushButton();
    formLayout_->addRow("", bgColorButton_);

    languageCombo_ = new QComboBox();
    languageCombo_->addItem("ENG");
    languageCombo_->addItem("HRV");
    formLayout_->addRow("", languageCombo_);

    mainLayout->addLayout(formLayout_);

    previewLabel_ = new QLabel();
    previewLabel_->setAlignment(Qt::AlignCenter);
    previewLabel_->setFrameStyle(QLabel::Box | QLabel::Sunken);
    previewLabel_->setMinimumHeight(50);
    mainLayout->addWidget(previewLabel_);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    applyButton_ = new QPushButton();
    cancelButton_ = new QPushButton();
    buttonLayout->addStretch();
    buttonLayout->addWidget(applyButton_);
    buttonLayout->addWidget(cancelButton_);
    mainLayout->addLayout(buttonLayout);

    connect(colorButton_, &QPushButton::clicked, this, &SettingsDialog::onChooseColor);
    connect(bgColorButton_, &QPushButton::clicked, this, &SettingsDialog::onChooseBgColor);
    connect(applyButton_, &QPushButton::clicked, this, &SettingsDialog::onApplyClicked);
    connect(cancelButton_, &QPushButton::clicked, this, &QDialog::reject);

    connect(fontSizeSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged),
            [this](int) { updatePreview(); });
}

void SettingsDialog::updateLanguage() {
    setWindowTitle(T("settings_title"));
    applyButton_->setText(T("settings_btn_apply"));
    cancelButton_->setText(T("settings_btn_cancel"));
    previewLabel_->setText(T("settings_preview"));

    if (auto* label = qobject_cast<QLabel*>(formLayout_->labelForField(fontSizeSpinBox_)))
        label->setText(T("settings_font"));
    if (auto* label = qobject_cast<QLabel*>(formLayout_->labelForField(colorButton_)))
        label->setText(T("settings_color"));
    if (auto* label = qobject_cast<QLabel*>(formLayout_->labelForField(bgColorButton_)))
        label->setText(T("settings_bg"));
    if (auto* label = qobject_cast<QLabel*>(formLayout_->labelForField(languageCombo_)))
        label->setText(T("settings_lang"));
}

void SettingsDialog::onChooseColor() {
    QColor color = QColorDialog::getColor(selectedColor_, this, T("settings_color_title"));
    if (color.isValid()) {
        selectedColor_ = color;
        updateColorButton();
        updatePreview();
    }
}

void SettingsDialog::onChooseBgColor() {
    QColor color = QColorDialog::getColor(selectedBgColor_, this, T("settings_bg_title"));
    if (color.isValid()) {
        selectedBgColor_ = color;
        updateBgColorButton();
        updatePreview();
    }
}

void SettingsDialog::updateColorButton() {
    colorButton_->setStyleSheet(
        QString("background-color: %1; color: %2;")
            .arg(selectedColor_.name())
            .arg(selectedColor_.lightness() > 128 ? "black" : "white"));
    colorButton_->setText(selectedColor_.name());
}

void SettingsDialog::updateBgColorButton() {
    bgColorButton_->setStyleSheet(
        QString("background-color: %1; color: %2;")
            .arg(selectedBgColor_.name())
            .arg(selectedBgColor_.lightness() > 128 ? "black" : "white"));
    bgColorButton_->setText(selectedBgColor_.name());
}

void SettingsDialog::updatePreview() {
    QFont font = previewLabel_->font();
    font.setPointSize(fontSizeSpinBox_->value());
    previewLabel_->setFont(font);
    previewLabel_->setStyleSheet(
        QString("color: %1; background-color: %2; padding: 10px;")
            .arg(selectedColor_.name())
            .arg(selectedBgColor_.name()));
}

void SettingsDialog::onApplyClicked() {
    accept();
}

int SettingsDialog::getFontSize() const {
    return fontSizeSpinBox_->value();
}

QColor SettingsDialog::getTextColor() const {
    return selectedColor_;
}

QColor SettingsDialog::getBgColor() const {
    return selectedBgColor_;
}

QString SettingsDialog::getLanguage() const {
    return languageCombo_->currentText();
}
