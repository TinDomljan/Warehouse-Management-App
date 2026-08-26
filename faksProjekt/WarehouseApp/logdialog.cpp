#include "logdialog.h"
#include "translationmanager.h"

#include <QDateTime>


static QString T(const std::string& key) {
    return QString::fromStdString(TranslationManager::instance().tr(key));
}



LogFormDialog::LogFormDialog(int id, const QString& title, QWidget* parent)
    : QDialog(parent), id_(id) {
    setupUI();
    setWindowTitle(title);
    resize(420, 220);
}

void LogFormDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QFormLayout* formLayout = new QFormLayout();

    usernameEdit_ = new QLineEdit();
    usernameEdit_->setPlaceholderText(T("log_ph_user"));
    formLayout->addRow(T("log_user"), usernameEdit_);

    actionEdit_ = new QLineEdit();
    actionEdit_->setPlaceholderText(T("log_ph_action"));
    formLayout->addRow(T("log_action"), actionEdit_);

    targetEdit_ = new QLineEdit();
    targetEdit_->setPlaceholderText(T("log_ph_target"));
    formLayout->addRow(T("log_target"), targetEdit_);

    timestampEdit_ = new QDateTimeEdit();
    timestampEdit_->setCalendarPopup(true);
    timestampEdit_->setDisplayFormat("yyyy-MM-dd hh:mm:ss");
    timestampEdit_->setDateTime(QDateTime::currentDateTime());
    formLayout->addRow(T("log_time"), timestampEdit_);

    mainLayout->addLayout(formLayout);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    saveButton_   = new QPushButton(T("log_btn_save"));
    cancelButton_ = new QPushButton(T("log_btn_cancel"));
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton_);
    buttonLayout->addWidget(cancelButton_);
    mainLayout->addLayout(buttonLayout);

    connect(saveButton_,   &QPushButton::clicked, this, &LogFormDialog::onSaveClicked);
    connect(cancelButton_, &QPushButton::clicked, this, &QDialog::reject);
}

void LogFormDialog::setEntry(const LogEntry& entry) {
    usernameEdit_->setText(QString::fromStdString(entry.username));
    actionEdit_->setText(QString::fromStdString(entry.action));
    targetEdit_->setText(QString::fromStdString(entry.target));
    timestampEdit_->setDateTime(
        QDateTime::fromSecsSinceEpoch(static_cast<qint64>(entry.timestamp)));
}

LogEntry LogFormDialog::getEntry() const {
    LogEntry entry;
    entry.id        = id_;
    entry.username  = usernameEdit_->text().toStdString();
    entry.action    = actionEdit_->text().toStdString();
    entry.target    = targetEdit_->text().toStdString();
    entry.timestamp = static_cast<time_t>(timestampEdit_->dateTime().toSecsSinceEpoch());
    return entry;
}

void LogFormDialog::onSaveClicked() {
    if (usernameEdit_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, T("log_err_title"), T("log_err_user"));
        usernameEdit_->setFocus();
        return;
    }
    accept();
}

// ============================================================
// LogDialog — table list with Add / Edit / Delete / Clear All
// ============================================================

LogDialog::LogDialog(QWidget* parent)
    : QDialog(parent) {
    jsonManager_ = new JsonManager("activity_log.json");
    setupUI();
    setWindowTitle(T("log_title"));
    resize(750, 450);
    refreshTable();
}

void LogDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    table_ = new QTableWidget(0, 5);
    table_->setHorizontalHeaderLabels(
        {T("log_col_id"), T("log_col_time"), T("log_col_user"),
         T("log_col_action"), T("log_col_target")});
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->verticalHeader()->setVisible(false);
    mainLayout->addWidget(table_);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    addBtn_    = new QPushButton(T("log_btn_add"));
    editBtn_   = new QPushButton(T("log_btn_edit"));
    deleteBtn_ = new QPushButton(T("log_btn_delete"));
    clearBtn_  = new QPushButton(T("log_btn_clear"));
    closeBtn_  = new QPushButton(T("log_btn_close"));
    btnLayout->addStretch();
    btnLayout->addWidget(addBtn_);
    btnLayout->addWidget(editBtn_);
    btnLayout->addWidget(deleteBtn_);
    btnLayout->addWidget(clearBtn_);
    btnLayout->addWidget(closeBtn_);
    mainLayout->addLayout(btnLayout);

    connect(addBtn_,    &QPushButton::clicked, this, &LogDialog::onAdd);
    connect(editBtn_,   &QPushButton::clicked, this, &LogDialog::onEdit);
    connect(deleteBtn_, &QPushButton::clicked, this, &LogDialog::onDelete);
    connect(clearBtn_,  &QPushButton::clicked, this, &LogDialog::onClearAll);
    connect(closeBtn_,  &QPushButton::clicked, this, &QDialog::accept);
}

void LogDialog::refreshTable() {
    entries_ = jsonManager_->loadEntries();

    table_->setRowCount(0);
    for (int i = 0; i < (int)entries_.size(); i++) {
        int row = table_->rowCount();
        table_->insertRow(row);

        char timeBuffer[26];
        ctime_s(timeBuffer, sizeof(timeBuffer), &entries_[i].timestamp);
        QString timeStr = QString(timeBuffer).trimmed();

        table_->setItem(row, 0, new QTableWidgetItem(QString::number(entries_[i].id)));
        table_->setItem(row, 1, new QTableWidgetItem(timeStr));
        table_->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(entries_[i].username)));
        table_->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(entries_[i].action)));
        table_->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(entries_[i].target)));
    }

    table_->resizeColumnsToContents();
    table_->horizontalHeader()->setStretchLastSection(true);
}

int LogDialog::getNextId() {
    int maxId = 0;
    for (int i = 0; i < (int)entries_.size(); i++) {
        if (entries_[i].id > maxId)
            maxId = entries_[i].id;
    }
    return maxId + 1;
}

void LogDialog::onAdd() {
    int nextId = getNextId();
    LogFormDialog dlg(nextId, T("log_add_title"), this);
    if (dlg.exec() == QDialog::Accepted) {
        jsonManager_->addEntry(dlg.getEntry());
        refreshTable();
    }
}

void LogDialog::onEdit() {
    int currentRow = table_->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, T("log_edit_title"), T("log_select_edit"));
        return;
    }

    LogFormDialog dlg(entries_[currentRow].id, T("log_edit_title"), this);
    dlg.setEntry(entries_[currentRow]);
    if (dlg.exec() == QDialog::Accepted) {
        jsonManager_->updateEntry(dlg.getEntry());
        refreshTable();
    }
}

void LogDialog::onDelete() {
    int currentRow = table_->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, T("log_delete_title"), T("log_select_delete"));
        return;
    }

    QString user = QString::fromStdString(entries_[currentRow].username);
    auto reply = QMessageBox::question(this, T("log_delete_title"),
        T("log_delete_confirm") + "\"" + user + "\"?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        jsonManager_->deleteEntry(entries_[currentRow].id);
        refreshTable();
    }
}

void LogDialog::onClearAll() {
    int reply = QMessageBox::question(this, T("log_clear_title"),
                                      T("log_clear_confirm"),
                                      QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        jsonManager_->clearLog();
        refreshTable();
    }
}
