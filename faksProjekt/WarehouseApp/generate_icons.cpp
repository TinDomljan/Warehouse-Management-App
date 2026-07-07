// Build-time tool: run once to produce the 3 warehouse PNG icons.
// Usage: GenerateIcons <output-directory>
// Qt6::Core + Qt6::Gui only — no display required (QImage is CPU-side).
#include <QCoreApplication>
#include <QImage>
#include <QPainter>
#include <QDir>
#include <cstdio>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    const QString outDir = (argc > 1) ? QString::fromLocal8Bit(argv[1]) : QStringLiteral(".");
    if (!QDir().mkpath(outDir)) {
        std::fprintf(stderr, "Cannot create output directory: %s\n", qPrintable(outDir));
        return 1;
    }

    struct IconSpec { const char* name; QColor fill; QColor border; };
    static const IconSpec icons[] = {
        // name       fill               border
        { "product",  QColor("#2980B9"), QColor("#1A5276") },  // blue  — Products tab
        { "supplier", QColor("#27AE60"), QColor("#1E8449") },  // green — Suppliers tab
        { "warning",  QColor("#E74C3C"), QColor("#922B21") },  // red   — Remove / danger
    };

    bool allOk = true;
    for (const auto& spec : icons) {
        QImage img(32, 32, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);

        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing);

        // Outer rounded rect
        p.setPen(QPen(spec.border, 1.5));
        p.setBrush(spec.fill);
        p.drawRoundedRect(QRectF(2.0, 2.0, 28.0, 28.0), 5.0, 5.0);

        // Small white inner highlight
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 255, 255, 60));
        p.drawRoundedRect(QRectF(5.0, 5.0, 22.0, 10.0), 3.0, 3.0);

        p.end();

        const QString path = outDir + u'/' + spec.name + ".png";
        if (!img.save(path, "PNG")) {
            std::fprintf(stderr, "[GenerateIcons] Failed to save %s\n", qPrintable(path));
            allOk = false;
        } else {
            std::printf("[GenerateIcons] Wrote %s\n", qPrintable(path));
        }
    }
    return allOk ? 0 : 1;
}
