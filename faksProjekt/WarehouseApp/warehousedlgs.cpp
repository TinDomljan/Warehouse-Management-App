#include "warehousedlgs.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QPixmap>
#include <QPainter>
#include <QFont>
#include <QWidget>


static QPixmap makeWarehouseIcon(int size) {
    QPixmap px(size, size);
    px.fill(Qt::transparent);
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);

    // zid
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#2980b9"));
    p.drawRect(size * 0.08, size * 0.40, size * 0.84, size * 0.52);

    // krov
    p.setBrush(QColor("#1a5276"));
    QPolygonF roof;
    roof << QPointF(size * 0.50, size * 0.06)
         << QPointF(size * 0.02, size * 0.43)
         << QPointF(size * 0.98, size * 0.43);
    p.drawPolygon(roof);

    //vrata
    p.setBrush(QColor("#aed6f1"));
    p.drawRect(size * 0.28, size * 0.54, size * 0.44, size * 0.22);
    p.setPen(QPen(QColor("#7fb3d3"), 1));
    for (int i = 1; i <= 3; ++i)
        p.drawLine(QPointF(size * 0.28, size * (0.54 + i * 0.055)),
                   QPointF(size * 0.72, size * (0.54 + i * 0.055)));

    // kvaka
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#1a5276"));
    p.drawEllipse(QPointF(size * 0.50, size * 0.65), size * 0.035, size * 0.035);

    return px;
}



void ShowAboutDialog(QWidget* parent) {
    QDialog dlg(parent);
    dlg.setWindowTitle("About WarehouseApp");
    dlg.setFixedWidth(360);
    dlg.setWindowFlags(dlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QVBoxLayout* layout = new QVBoxLayout(&dlg);
    layout->setSpacing(10);
    layout->setContentsMargins(24, 20, 24, 20);


    QLabel* iconLabel = new QLabel;
    iconLabel->setPixmap(makeWarehouseIcon(72));
    iconLabel->setAlignment(Qt::AlignHCenter);
    layout->addWidget(iconLabel);


    QLabel* nameLabel = new QLabel("WarehouseApp");
    {
        QFont f = nameLabel->font();
        f.setPointSize(16);
        f.setBold(true);
        nameLabel->setFont(f);
    }
    nameLabel->setAlignment(Qt::AlignHCenter);
    layout->addWidget(nameLabel);


    QLabel* versionLabel = new QLabel("Version 1.0.0");
    versionLabel->setAlignment(Qt::AlignHCenter);
    versionLabel->setStyleSheet("color: gray;");
    layout->addWidget(versionLabel);


    QFrame* sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setFrameShadow(QFrame::Sunken);
    layout->addWidget(sep);


    QGridLayout* grid = new QGridLayout;
    grid->setColumnStretch(1, 1);
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(6);

    auto addRow = [&](int row, const QString& key, const QString& val) {
        QLabel* k = new QLabel(key + ":");
        k->setStyleSheet("font-weight: bold; color: #555;");
        QLabel* v = new QLabel(val);
        v->setWordWrap(true);
        grid->addWidget(k, row, 0);
        grid->addWidget(v, row, 1);
    };

    addRow(0, "Author",      "Tin Domljan");
    addRow(1, "Built with",  "Qt 6  ·  OpenSSL  ·  SQLite");
    addRow(2, "Description", "Desktop inventory management system\n"
                             "with crypto, networking & reporting.");

    layout->addLayout(grid);
    layout->addSpacing(6);


    QPushButton* closeBtn = new QPushButton("Close");
    closeBtn->setFixedWidth(90);
    QHBoxLayout* btnRow = new QHBoxLayout;
    btnRow->addStretch();
    btnRow->addWidget(closeBtn);
    layout->addLayout(btnRow);

    QObject::connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);

    dlg.exec();
}

void ShowQuickCalcDialog(QWidget* parent) {
    QDialog dlg(parent);
    dlg.setWindowTitle("Quick Price Calculator");
    dlg.setFixedWidth(320);
    dlg.setWindowFlags(dlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QVBoxLayout* layout = new QVBoxLayout(&dlg);
    layout->setSpacing(10);
    layout->setContentsMargins(20, 16, 20, 16);

    QLabel* title = new QLabel("Price Calculator");
    {
        QFont f = title->font();
        f.setPointSize(13);
        f.setBold(true);
        title->setFont(f);
    }
    layout->addWidget(title);

    QFrame* sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setFrameShadow(QFrame::Sunken);
    layout->addWidget(sep);


    QGridLayout* grid = new QGridLayout;
    grid->setColumnStretch(1, 1);
    grid->setVerticalSpacing(8);
    grid->setHorizontalSpacing(10);

    QLabel* purchaseLabel = new QLabel("Purchase price (€):");
    QDoubleSpinBox* purchaseSpin = new QDoubleSpinBox;
    purchaseSpin->setRange(0.0, 999'999.99);
    purchaseSpin->setDecimals(2);
    purchaseSpin->setSingleStep(1.0);
    purchaseSpin->setValue(100.0);
    purchaseSpin->setAlignment(Qt::AlignRight);

    QLabel* markupLabel = new QLabel("Markup (%):");
    QDoubleSpinBox* markupSpin = new QDoubleSpinBox;
    markupSpin->setRange(0.0, 10'000.0);
    markupSpin->setDecimals(1);
    markupSpin->setSingleStep(5.0);
    markupSpin->setValue(30.0);
    markupSpin->setAlignment(Qt::AlignRight);

    grid->addWidget(purchaseLabel, 0, 0);
    grid->addWidget(purchaseSpin,  0, 1);
    grid->addWidget(markupLabel,   1, 0);
    grid->addWidget(markupSpin,    1, 1);
    layout->addLayout(grid);


    QFrame* resultFrame = new QFrame;
    resultFrame->setFrameShape(QFrame::StyledPanel);
    resultFrame->setStyleSheet(
        "QFrame { background: #eaf4fb; border: 1px solid #aed6f1; border-radius: 4px; }");

    QGridLayout* resGrid = new QGridLayout(resultFrame);
    resGrid->setContentsMargins(12, 10, 12, 10);
    resGrid->setVerticalSpacing(6);
    resGrid->setColumnStretch(1, 1);

    auto makeResultLabel = [](const QString& text) -> QLabel* {
        QLabel* l = new QLabel(text);
        l->setStyleSheet("font-weight: bold; font-size: 13px; color: #1a5276;");
        l->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        return l;
    };

    QLabel* sellLbl    = new QLabel("Selling price:");
    QLabel* marginLbl  = new QLabel("Gross margin:");
    QLabel* profitLbl  = new QLabel("Gross profit:");
    QLabel* sellVal    = makeResultLabel("€130.00");
    QLabel* marginVal  = makeResultLabel("23.1 %");
    QLabel* profitVal  = makeResultLabel("€30.00");

    resGrid->addWidget(sellLbl,   0, 0);
    resGrid->addWidget(sellVal,   0, 1);
    resGrid->addWidget(marginLbl, 1, 0);
    resGrid->addWidget(marginVal, 1, 1);
    resGrid->addWidget(profitLbl, 2, 0);
    resGrid->addWidget(profitVal, 2, 1);

    layout->addWidget(resultFrame);


    auto recalc = [&]() {
        const double purchase = purchaseSpin->value();
        const double markup   = markupSpin->value();
        const double selling  = purchase * (1.0 + markup / 100.0);
        const double margin   = (markup / (100.0 + markup)) * 100.0;
        const double profit   = selling - purchase;

        sellVal->setText(QString("€%1").arg(selling, 0, 'f', 2));
        marginVal->setText(QString("%1 %").arg(margin, 0, 'f', 1));
        profitVal->setText(QString("€%1").arg(profit, 0, 'f', 2));
    };


    recalc();

    QObject::connect(purchaseSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                     [recalc](double) { recalc(); });
    QObject::connect(markupSpin,   QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                     [recalc](double) { recalc(); });


    QPushButton* closeBtn = new QPushButton("Close");
    closeBtn->setFixedWidth(90);
    QHBoxLayout* btnRow = new QHBoxLayout;
    btnRow->addStretch();
    btnRow->addWidget(closeBtn);
    layout->addLayout(btnRow);

    QObject::connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);

    dlg.exec();
}
