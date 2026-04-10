#include "shapes.h"

// ─────────────────────────────────────────────────────────────────────────────
// Pen helpers
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject penToJson(const QPen &pen)
{
    return QJsonObject{
        {"color", pen.color().name()},
        {"width", pen.widthF()}
    };
}

QPen penFromJson(const QJsonObject &obj)
{
    QPen pen;
    pen.setColor(QColor(obj["color"].toString("#000000")));
    pen.setWidthF(obj["width"].toDouble(1.0));
    return pen;
}

// ─────────────────────────────────────────────────────────────────────────────
// LineItem
// ─────────────────────────────────────────────────────────────────────────────

LineItem::LineItem(const QLineF &line, const QPen &pen)
{
    setLine(line);
    setPen(pen);
    setFlags(ItemIsSelectable | ItemIsMovable);
}

QJsonObject LineItem::toJson() const
{
    const QLineF l  = line();
    const QPointF p1 = mapToScene(l.p1());
    const QPointF p2 = mapToScene(l.p2());
    return QJsonObject{
        {"type", "line"},
        {"x1", p1.x()}, {"y1", p1.y()},
        {"x2", p2.x()}, {"y2", p2.y()},
        {"pen", penToJson(pen())}
    };
}

LineItem *LineItem::fromJson(const QJsonObject &obj)
{
    QLineF l(obj["x1"].toDouble(), obj["y1"].toDouble(),
             obj["x2"].toDouble(), obj["y2"].toDouble());
    return new LineItem(l, penFromJson(obj["pen"].toObject()));
}

// ─────────────────────────────────────────────────────────────────────────────
// CircleItem
// ─────────────────────────────────────────────────────────────────────────────

CircleItem::CircleItem(const QRectF &rect, const QPen &pen)
{
    setRect(rect);
    setPen(pen);
    setFlags(ItemIsSelectable | ItemIsMovable);
}

QJsonObject CircleItem::toJson() const
{
    const QRectF r   = rect();
    const QPointF tl = mapToScene(r.topLeft());
    return QJsonObject{
        {"type", "circle"},
        {"x", tl.x()}, {"y", tl.y()},
        {"w", r.width()}, {"h", r.height()},
        {"pen", penToJson(pen())}
    };
}

CircleItem *CircleItem::fromJson(const QJsonObject &obj)
{
    QRectF r(obj["x"].toDouble(), obj["y"].toDouble(),
             obj["w"].toDouble(), obj["h"].toDouble());
    return new CircleItem(r, penFromJson(obj["pen"].toObject()));
}

// ─────────────────────────────────────────────────────────────────────────────
// RectItem
// ─────────────────────────────────────────────────────────────────────────────

RectItem::RectItem(const QRectF &rect, const QPen &pen)
{
    setRect(rect);
    setPen(pen);
    setFlags(ItemIsSelectable | ItemIsMovable);
}

QJsonObject RectItem::toJson() const
{
    const QRectF r   = rect();
    const QPointF tl = mapToScene(r.topLeft());
    return QJsonObject{
        {"type", "rect"},
        {"x", tl.x()}, {"y", tl.y()},
        {"w", r.width()}, {"h", r.height()},
        {"pen", penToJson(pen())}
    };
}

RectItem *RectItem::fromJson(const QJsonObject &obj)
{
    QRectF r(obj["x"].toDouble(), obj["y"].toDouble(),
             obj["w"].toDouble(), obj["h"].toDouble());
    return new RectItem(r, penFromJson(obj["pen"].toObject()));
}
