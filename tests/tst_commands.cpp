/**
 * @file tst_commands.cpp
 * @brief Unit tests for the undo/redo command stack.
 *
 * Tests AddShapeCommand, RemoveShapesCommand, MoveShapesCommand, and
 * EditShapeCommand via the QUndoStack embedded in DrawingScene.
 */

#include <QApplication>
#include <QtTest>

#include "commands.h"
#include "drawingscene.h"
#include "shapes.h"

class TestCommands : public QObject
{
    Q_OBJECT

private slots:
    void addShapeUndoRedo();
    void removeShapesUndoRedo();
    void moveShapesUndoRedo();
    void editShapeUndoRedo();
    void undoStackIndexFollowsCommands();
};

// ─────────────────────────────────────────────────────────────────────────────
// AddShapeCommand
// ─────────────────────────────────────────────────────────────────────────────

void TestCommands::addShapeUndoRedo()
{
    DrawingScene scene;
    auto *line = new LineItem(QLineF(0, 0, 100, 100), QPen(Qt::black, 1));

    QCOMPARE(scene.items().count(), 0);

    scene.undoStack()->push(new AddShapeCommand(&scene, line));
    QCOMPARE(scene.items().count(), 1);
    QVERIFY(scene.items().contains(line));

    scene.undoStack()->undo();
    QCOMPARE(scene.items().count(), 0);

    scene.undoStack()->redo();
    QCOMPARE(scene.items().count(), 1);
    QVERIFY(scene.items().contains(line));
}

// ─────────────────────────────────────────────────────────────────────────────
// RemoveShapesCommand
// ─────────────────────────────────────────────────────────────────────────────

void TestCommands::removeShapesUndoRedo()
{
    DrawingScene scene;
    auto *rect = new RectItem(QRectF(0, 0, 50, 50), QPen(Qt::black, 1));
    scene.addItem(rect);
    QCOMPARE(scene.items().count(), 1);

    scene.undoStack()->push(new RemoveShapesCommand(&scene, {rect}));
    QCOMPARE(scene.items().count(), 0);

    scene.undoStack()->undo();
    QCOMPARE(scene.items().count(), 1);

    scene.undoStack()->redo();
    QCOMPARE(scene.items().count(), 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// MoveShapesCommand
// ─────────────────────────────────────────────────────────────────────────────

void TestCommands::moveShapesUndoRedo()
{
    DrawingScene scene;
    auto *circle = new CircleItem(QRectF(0, 0, 40, 40), QPen(Qt::black, 1));
    scene.addItem(circle);

    const QPointF oldPos(0, 0);
    const QPointF newPos(100, 200);

    scene.undoStack()->push(new MoveShapesCommand({{circle, oldPos, newPos}}));
    QCOMPARE(circle->pos(), newPos);

    scene.undoStack()->undo();
    QCOMPARE(circle->pos(), oldPos);

    scene.undoStack()->redo();
    QCOMPARE(circle->pos(), newPos);
}

// ─────────────────────────────────────────────────────────────────────────────
// EditShapeCommand
// ─────────────────────────────────────────────────────────────────────────────

void TestCommands::editShapeUndoRedo()
{
    DrawingScene scene;
    auto *line = new LineItem(QLineF(0, 0, 100, 100), QPen(Qt::black, 1));
    scene.addItem(line);

    const QJsonObject oldParams = line->toJson();
    const QJsonObject newParams = QJsonObject{
        {"type", "line"},
        {"x1", 10.0}, {"y1", 10.0},
        {"x2", 200.0}, {"y2", 200.0},
        {"pen", penToJson(QPen(Qt::red, 3))}
    };

    scene.undoStack()->push(new EditShapeCommand(line, oldParams, newParams));
    QCOMPARE(line->line().x1(), 10.0);
    QCOMPARE(line->pen().color(), QColor(Qt::red));

    scene.undoStack()->undo();
    QCOMPARE(line->line().x1(), 0.0);
    QCOMPARE(line->pen().color(), QColor(Qt::black));

    scene.undoStack()->redo();
    QCOMPARE(line->line().x1(), 10.0);
}

// ─────────────────────────────────────────────────────────────────────────────
// Undo stack index tracking
// ─────────────────────────────────────────────────────────────────────────────

void TestCommands::undoStackIndexFollowsCommands()
{
    DrawingScene scene;
    QCOMPARE(scene.undoStack()->count(), 0);

    scene.undoStack()->push(new AddShapeCommand(&scene,
        new LineItem(QLineF(0,0,1,1), QPen(Qt::black,1))));
    QCOMPARE(scene.undoStack()->count(), 1);

    scene.undoStack()->push(new AddShapeCommand(&scene,
        new RectItem(QRectF(0,0,10,10), QPen(Qt::black,1))));
    QCOMPARE(scene.undoStack()->count(), 2);

    scene.undoStack()->undo();
    QCOMPARE(scene.undoStack()->index(), 1);

    scene.undoStack()->undo();
    QCOMPARE(scene.undoStack()->index(), 0);
}

// ─────────────────────────────────────────────────────────────────────────────

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    TestCommands tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "tst_commands.moc"
