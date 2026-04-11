#include "propertypanel.h"

#include "shapes.h"

#include <QFormLayout>
#include <QGraphicsItem>
#include <QGraphicsLineItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsRectItem>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────

PropertyPanel::PropertyPanel(QWidget *parent)
    : QDockWidget(tr("Свойства"), parent)
{
    setMinimumWidth(200);

    m_content = new QWidget(this);
    m_content->setLayout(new QFormLayout(m_content));

    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setWidget(m_content);
    setWidget(scroll);
}

// ─────────────────────────────────────────────────────────────────────────────

void PropertyPanel::clear()
{
    // Delete the label widgets themselves (they are children of m_content);
    // just clearing the lists would leave them visible and overlapping.
    qDeleteAll(m_keyLabels);
    qDeleteAll(m_valLabels);
    m_keyLabels.clear();
    m_valLabels.clear();
    delete m_content->layout();
    m_content->setLayout(new QFormLayout(m_content));
}

void PropertyPanel::setRow(const QString &key, const QString &value)
{
    for (int i = 0; i < m_keyLabels.size(); ++i) {
        if (m_keyLabels[i]->text() == key) {
            m_valLabels[i]->setText(value);
            return;
        }
    }
    auto *kl = new QLabel(key, m_content);
    auto *vl = new QLabel(value, m_content);
    kl->setStyleSheet("font-weight: bold;");
    qobject_cast<QFormLayout*>(m_content->layout())->addRow(kl, vl);
    m_keyLabels.append(kl);
    m_valLabels.append(vl);
}

// ─────────────────────────────────────────────────────────────────────────────

static QString vec3Str(const QVector3D &v)
{
    return QString("(%1, %2, %3)")
        .arg(v.x(), 0, 'f', 2)
        .arg(v.y(), 0, 'f', 2)
        .arg(v.z(), 0, 'f', 2);
}

void PropertyPanel::showShape3D(const Shape3DParams &p)
{
    clear();
    setRow(tr("Имя"),         p.name);
    setRow(tr("Тип"),         shapeTypeName(p.type));
    setRow(tr("Цвет"),        p.color.name());
    setRow(tr("Позиция"),     vec3Str(p.position));
    setRow(tr("Поворот °"),   vec3Str(p.rotation));
    setRow(tr("Масштаб"),     vec3Str(p.scale));

    switch (p.type) {
        case ShapeType3D::Box:
            setRow(tr("Размер X"), QString::number(p.xSize, 'f', 2));
            setRow(tr("Размер Y"), QString::number(p.ySize, 'f', 2));
            setRow(tr("Размер Z"), QString::number(p.zSize, 'f', 2));
            break;
        case ShapeType3D::Sphere:
            setRow(tr("Радиус"), QString::number(p.xSize, 'f', 2));
            break;
        case ShapeType3D::Cylinder:
            setRow(tr("Радиус"),  QString::number(p.xSize, 'f', 2));
            setRow(tr("Высота"),  QString::number(p.ySize, 'f', 2));
            break;
        case ShapeType3D::Cone:
            setRow(tr("Осн. радиус"), QString::number(p.xSize, 'f', 2));
            setRow(tr("Верх. радиус"), QString::number(p.radius2, 'f', 2));
            setRow(tr("Высота"),       QString::number(p.ySize, 'f', 2));
            break;
        case ShapeType3D::Torus:
            setRow(tr("Радиус"),       QString::number(p.xSize, 'f', 2));
            setRow(tr("Мал. радиус"),  QString::number(p.radius2, 'f', 2));
            break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────

void PropertyPanel::showShape2D(QGraphicsItem *item)
{
    if (!item) { clear(); return; }

    clear();

    if (auto *li = qgraphicsitem_cast<LineItem*>(item)) {
        const QLineF line = li->line();
        const QPointF p1 = li->mapToScene(line.p1());
        const QPointF p2 = li->mapToScene(line.p2());
        const double len = QLineF(p1, p2).length();
        setRow(tr("Тип"),    tr("Отрезок"));
        setRow(tr("X1"),     QString::number(p1.x(), 'f', 1));
        setRow(tr("Y1"),     QString::number(p1.y(), 'f', 1));
        setRow(tr("X2"),     QString::number(p2.x(), 'f', 1));
        setRow(tr("Y2"),     QString::number(p2.y(), 'f', 1));
        setRow(tr("Длина"),  QString::number(len, 'f', 1));
    } else if (auto *ci = qgraphicsitem_cast<CircleItem*>(item)) {
        const QRectF r = ci->mapToScene(ci->rect()).boundingRect();
        const double rx = r.width()  / 2.0;
        const double ry = r.height() / 2.0;
        setRow(tr("Тип"),    tr("Эллипс"));
        setRow(tr("Центр X"), QString::number(r.center().x(), 'f', 1));
        setRow(tr("Центр Y"), QString::number(r.center().y(), 'f', 1));
        setRow(tr("Радиус X"), QString::number(rx, 'f', 1));
        setRow(tr("Радиус Y"), QString::number(ry, 'f', 1));
        setRow(tr("Площадь"), QString::number(M_PI * rx * ry, 'f', 1));
    } else if (auto *ri = qgraphicsitem_cast<RectItem*>(item)) {
        const QRectF r = ri->mapToScene(ri->rect()).boundingRect();
        setRow(tr("Тип"),      tr("Прямоугольник"));
        setRow(tr("X"),        QString::number(r.x(), 'f', 1));
        setRow(tr("Y"),        QString::number(r.y(), 'f', 1));
        setRow(tr("Ширина"),   QString::number(r.width(), 'f', 1));
        setRow(tr("Высота"),   QString::number(r.height(), 'f', 1));
        setRow(tr("Площадь"),  QString::number(r.width() * r.height(), 'f', 1));
    }
}
