#include "commands.h"
#include "drawingscene.h"
#include "shapes.h"

// ─────────────────────────────────────────────────────────────────────────────
// AddShapeCommand
// ─────────────────────────────────────────────────────────────────────────────

AddShapeCommand::AddShapeCommand(DrawingScene *scene, QGraphicsItem *item,
                                 QUndoCommand *parent)
    : QUndoCommand(QObject::tr("Добавить фигуру"), parent)
    , m_scene(scene)
    , m_item(item)
{}

AddShapeCommand::~AddShapeCommand()
{
    if (!m_sceneOwns)
        delete m_item;
}

void AddShapeCommand::undo()
{
    m_scene->removeItem(m_item);
    m_sceneOwns = false;
}

void AddShapeCommand::redo()
{
    m_scene->addItem(m_item);
    m_sceneOwns = true;

    // Apply current tool's movability setting to the re-added item
    const bool movable = (m_scene->tool() == DrawTool::Select);
    m_item->setFlag(QGraphicsItem::ItemIsMovable, movable);
}

// ─────────────────────────────────────────────────────────────────────────────
// RemoveShapesCommand
// ─────────────────────────────────────────────────────────────────────────────

RemoveShapesCommand::RemoveShapesCommand(DrawingScene *scene,
                                         const QList<QGraphicsItem*> &items,
                                         QUndoCommand *parent)
    : QUndoCommand(QObject::tr("Удалить фигуры"), parent)
    , m_scene(scene)
    , m_items(items)
{}

RemoveShapesCommand::~RemoveShapesCommand()
{
    if (m_sceneOwns)
        qDeleteAll(m_items);
}

void RemoveShapesCommand::undo()
{
    for (auto *item : m_items)
        m_scene->addItem(item);
    m_sceneOwns = false;
}

void RemoveShapesCommand::redo()
{
    for (auto *item : m_items)
        m_scene->removeItem(item);
    m_sceneOwns = true;
}

// ─────────────────────────────────────────────────────────────────────────────
// MoveShapesCommand
// ─────────────────────────────────────────────────────────────────────────────

MoveShapesCommand::MoveShapesCommand(const QList<ItemMove> &moves,
                                     QUndoCommand *parent)
    : QUndoCommand(QObject::tr("Переместить"), parent)
    , m_moves(moves)
{}

void MoveShapesCommand::undo()
{
    for (const auto &m : m_moves)
        m.item->setPos(m.oldPos);
}

void MoveShapesCommand::redo()
{
    for (const auto &m : m_moves)
        m.item->setPos(m.newPos);
}

// ─────────────────────────────────────────────────────────────────────────────
// EditShapeCommand
// ─────────────────────────────────────────────────────────────────────────────

EditShapeCommand::EditShapeCommand(QGraphicsItem *item,
                                   const QJsonObject &oldParams,
                                   const QJsonObject &newParams,
                                   QUndoCommand *parent)
    : QUndoCommand(QObject::tr("Изменить параметры"), parent)
    , m_item(item)
    , m_old(oldParams)
    , m_new(newParams)
{}

void EditShapeCommand::undo() { applyParams(m_old); }
void EditShapeCommand::redo() { applyParams(m_new); }

void EditShapeCommand::applyParams(const QJsonObject &p)
{
    // Params are in scene-space; reset pos so geometry == scene coords
    m_item->setPos(0.0, 0.0);

    const QString type = p["type"].toString();
    if (type == "line") {
        auto *li = static_cast<LineItem *>(m_item);
        li->setLine(QLineF(p["x1"].toDouble(), p["y1"].toDouble(),
                           p["x2"].toDouble(), p["y2"].toDouble()));
        li->setPen(penFromJson(p["pen"].toObject()));
    } else if (type == "circle") {
        auto *ci = static_cast<CircleItem *>(m_item);
        ci->setRect(QRectF(p["x"].toDouble(), p["y"].toDouble(),
                           p["w"].toDouble(), p["h"].toDouble()));
        ci->setPen(penFromJson(p["pen"].toObject()));
    } else if (type == "rect") {
        auto *ri = static_cast<RectItem *>(m_item);
        ri->setRect(QRectF(p["x"].toDouble(), p["y"].toDouble(),
                           p["w"].toDouble(), p["h"].toDouble()));
        ri->setPen(penFromJson(p["pen"].toObject()));
    }
}
