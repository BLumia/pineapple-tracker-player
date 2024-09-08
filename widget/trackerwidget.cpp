#include "trackerwidget.h"
#include "util.h"

#include <algorithm>

#include <QOverload>

TrackerWidget::TrackerWidget(QWidget *parent)
    : QWidget(parent)
{
    connect(this, &TrackerWidget::patternContentChanged, this, qOverload<>(&QWidget::update));
    connect(this, &TrackerWidget::currentRowChanged, this, qOverload<>(&QWidget::update));
}

TrackerWidget::~TrackerWidget()
{

}

void TrackerWidget::paintEvent(QPaintEvent *event)
{
    static QFontMetrics fm(Util::defaultMonoFont());
    const int fontHeight = fm.height() + fm.lineSpacing();
    const int centerY = height() / 2 - fontHeight / 2;

    QPainter painter(this);
    painter.setFont(Util::defaultMonoFont());
    if (patternContent().isEmpty()) {
        painter.drawText(0, centerY, width(), fontHeight, Qt::AlignCenter, QStringLiteral("===  ===  ==="));
    } else {
        const int rowCount = patternContent().count();
        const int curRow = currentRow();
        for (int i = 0; i < rowCount; i++) {
            const int offset = (i - curRow) * fontHeight;
            QStringList curRow(patternContent().at(i));
            std::transform(curRow.cbegin(), curRow.cend(), curRow.begin(),
                           [](const QString from) -> const QString { return from.first(3); });
            painter.drawText(0, centerY + offset, width(), fontHeight,
                             Qt::AlignCenter, curRow.join(QStringLiteral("  ")));
        }
    }
    painter.drawRect(-1, centerY, width() + 2, fontHeight);
}

