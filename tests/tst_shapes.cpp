/**
 * @file tst_shapes.cpp
 * @brief Unit tests for shape serialisation/deserialisation.
 *
 * Covers toJson() / fromJson() round-trips for LineItem, CircleItem,
 * and RectItem including pen colour and width.
 */

#include <QApplication>
#include <QtTest>

#include "shapes.h"

class TestShapes : public QObject
{
    Q_OBJECT

private slots:
    void lineSerialisesType();
    void lineRoundTrip();
    void linePenRoundTrip();

    void circleRoundTrip();
    void circlePenRoundTrip();

    void rectRoundTrip();
    void rectPenRoundTrip();

    void penHelpers();
};

// ─────────────────────────────────────────────────────────────────────────────
// LineItem
// ─────────────────────────────────────────────────────────────────────────────

void TestShapes::lineSerialisesType()
{
    LineItem item(QLineF(0, 0, 10, 10), QPen(Qt::black, 1));
    QCOMPARE(item.toJson()["type"].toString(), QString("line"));
}

void TestShapes::lineRoundTrip()
{
    const QLineF original(10.0, 20.0, 100.0, 200.0);
    LineItem src(original, QPen(Qt::black, 1));
    const QJsonObject json = src.toJson();

    LineItem *dst = LineItem::fromJson(json);
    QVERIFY(dst != nullptr);
    QCOMPARE(dst->line().x1(), original.x1());
    QCOMPARE(dst->line().y1(), original.y1());
    QCOMPARE(dst->line().x2(), original.x2());
    QCOMPARE(dst->line().y2(), original.y2());
    delete dst;
}

void TestShapes::linePenRoundTrip()
{
    LineItem src(QLineF(0, 0, 1, 1), QPen(Qt::red, 3));
    LineItem *dst = LineItem::fromJson(src.toJson());
    QVERIFY(dst != nullptr);
    QCOMPARE(dst->pen().color(), QColor(Qt::red));
    QCOMPARE(dst->pen().width(), 3);
    delete dst;
}

// ─────────────────────────────────────────────────────────────────────────────
// CircleItem
// ─────────────────────────────────────────────────────────────────────────────

void TestShapes::circleRoundTrip()
{
    const QRectF original(50.0, 60.0, 100.0, 80.0);
    CircleItem src(original, QPen(Qt::black, 1));
    CircleItem *dst = CircleItem::fromJson(src.toJson());
    QVERIFY(dst != nullptr);
    QCOMPARE(dst->rect(), original);
    delete dst;
}

void TestShapes::circlePenRoundTrip()
{
    CircleItem src(QRectF(0, 0, 50, 50), QPen(Qt::blue, 2));
    CircleItem *dst = CircleItem::fromJson(src.toJson());
    QVERIFY(dst != nullptr);
    QCOMPARE(dst->pen().color(), QColor(Qt::blue));
    QCOMPARE(dst->pen().width(), 2);
    delete dst;
}

// ─────────────────────────────────────────────────────────────────────────────
// RectItem
// ─────────────────────────────────────────────────────────────────────────────

void TestShapes::rectRoundTrip()
{
    const QRectF original(0.0, 0.0, 200.0, 150.0);
    RectItem src(original, QPen(Qt::black, 1));
    RectItem *dst = RectItem::fromJson(src.toJson());
    QVERIFY(dst != nullptr);
    QCOMPARE(dst->rect(), original);
    delete dst;
}

void TestShapes::rectPenRoundTrip()
{
    RectItem src(QRectF(0, 0, 10, 10), QPen(Qt::green, 5));
    RectItem *dst = RectItem::fromJson(src.toJson());
    QVERIFY(dst != nullptr);
    QCOMPARE(dst->pen().color(), QColor(Qt::green));
    QCOMPARE(dst->pen().width(), 5);
    delete dst;
}

// ─────────────────────────────────────────────────────────────────────────────
// Pen helpers
// ─────────────────────────────────────────────────────────────────────────────

void TestShapes::penHelpers()
{
    QPen original(QColor("#ff8800"), 4);
    const QJsonObject json = penToJson(original);
    const QPen restored   = penFromJson(json);

    QCOMPARE(restored.color().name(), original.color().name());
    QCOMPARE(restored.widthF(), original.widthF());
}

// ─────────────────────────────────────────────────────────────────────────────

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    TestShapes tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "tst_shapes.moc"
