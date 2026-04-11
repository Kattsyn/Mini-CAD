#include "axisindicator.h"

#include <QFont>
#include <QPainter>
#include <QPen>
#include <QVector3D>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────

AxisIndicator::AxisIndicator(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(80, 80);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
}

void AxisIndicator::updateView(const QMatrix4x4 &viewMatrix)
{
    m_view = viewMatrix;
    update();
}

void AxisIndicator::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Semi-transparent dark circle background
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 100));
    p.drawEllipse(rect().adjusted(2, 2, -2, -2));

    const float cx = width()  * 0.5f;
    const float cy = height() * 0.5f;
    const float len = cx - 10.0f;

    struct Axis { QVector3D dir; QColor color; char label; };
    const Axis axes[3] = {
        { {1, 0, 0}, QColor(220,  60,  60), 'X' },
        { {0, 1, 0}, QColor( 60, 200,  60), 'Y' },
        { {0, 0, 1}, QColor( 60, 120, 220), 'Z' },
    };

    // Camera-space Z for each axis (used for depth sort and dimming)
    float depths[3];
    for (int i = 0; i < 3; ++i)
        depths[i] = m_view.mapVector(axes[i].dir).z();

    // Sort back-to-front so nearer axes are drawn on top
    int order[3] = {0, 1, 2};
    std::sort(order, order + 3, [&](int a, int b){ return depths[a] < depths[b]; });

    p.setFont(QFont("sans-serif", 8, QFont::Bold));

    for (int idx = 0; idx < 3; ++idx) {
        const int i = order[idx];
        const QVector3D cs = m_view.mapVector(axes[i].dir);

        // Project to 2D: use camera-space X and Y (flip Y for screen)
        const float sx = cs.x() * len;
        const float sy = -cs.y() * len;

        // Axes pointing away from the viewer are dimmed
        QColor col = axes[i].color;
        col.setAlphaF(depths[i] < 0.0f ? 1.0f : 0.35f);

        p.setPen(QPen(col, 2, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(cx, cy), QPointF(cx + sx, cy + sy));

        // Arrowhead (small filled circle at the tip)
        p.setBrush(col);
        p.setPen(Qt::NoPen);
        p.drawEllipse(QPointF(cx + sx, cy + sy), 3.5f, 3.5f);

        // Label just beyond the tip
        p.setPen(col);
        p.drawText(QPointF(cx + sx * 1.3f - 4.0f, cy + sy * 1.3f + 4.0f),
                   QString(QChar(axes[i].label)));
    }
}
