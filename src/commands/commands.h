#pragma once

#include <QUndoCommand>
#include <QList>
#include <QPointF>
#include <QJsonObject>

class DrawingScene;
class QGraphicsItem;

// ─────────────────────────────────────────────────────────────────────────────
// AddShapeCommand
// ─────────────────────────────────────────────────────────────────────────────

/// @brief Undo command: add a single shape to the scene.
///
/// On redo() the shape is added; on undo() it is removed.
/// The command takes ownership of the item while it is not on the scene.
class AddShapeCommand : public QUndoCommand
{
public:
    /// @param scene   Scene that will own the item when on canvas.
    /// @param item    Heap-allocated shape; command takes ownership when undone.
    /// @param parent  Optional parent command (for macro commands).
    AddShapeCommand(DrawingScene *scene, QGraphicsItem *item,
                    QUndoCommand *parent = nullptr);
    ~AddShapeCommand() override;

    void undo() override; ///< Removes the shape from the scene.
    void redo() override; ///< (Re-)adds the shape to the scene.

private:
    DrawingScene  *m_scene;
    QGraphicsItem *m_item;
    bool           m_sceneOwns{true};
};

// ─────────────────────────────────────────────────────────────────────────────
// RemoveShapesCommand
// ─────────────────────────────────────────────────────────────────────────────

/// @brief Undo command: remove one or more shapes from the scene.
class RemoveShapesCommand : public QUndoCommand
{
public:
    RemoveShapesCommand(DrawingScene *scene,
                        const QList<QGraphicsItem*> &items,
                        QUndoCommand *parent = nullptr);
    ~RemoveShapesCommand() override;

    void undo() override; ///< Re-adds shapes to the scene.
    void redo() override; ///< Removes shapes from the scene.

private:
    DrawingScene          *m_scene;
    QList<QGraphicsItem*>  m_items;
    bool                   m_sceneOwns{false};
};

// ─────────────────────────────────────────────────────────────────────────────
// MoveShapesCommand
// ─────────────────────────────────────────────────────────────────────────────

/// @brief Holds old/new position for a single moved item.
struct ItemMove {
    QGraphicsItem *item;    ///< The moved item.
    QPointF        oldPos;  ///< Position before the move.
    QPointF        newPos;  ///< Position after the move.
};

/// @brief Undo command: move one or more shapes.
class MoveShapesCommand : public QUndoCommand
{
public:
    explicit MoveShapesCommand(const QList<ItemMove> &moves,
                               QUndoCommand *parent = nullptr);

    void undo() override; ///< Restores all shapes to their old positions.
    void redo() override; ///< Moves all shapes to their new positions.

private:
    QList<ItemMove> m_moves;
};

// ─────────────────────────────────────────────────────────────────────────────
// EditShapeCommand
// ─────────────────────────────────────────────────────────────────────────────

/// @brief Undo command: parametrically edit a shape's geometry and pen.
///
/// Parameters are stored as JSON snapshots in scene-space coordinates
/// (the format returned by each shape's toJson()).
class EditShapeCommand : public QUndoCommand
{
public:
    /// @param item       The shape to edit (must remain alive for command lifetime).
    /// @param oldParams  JSON snapshot before the edit.
    /// @param newParams  JSON snapshot after the edit.
    /// @param parent     Optional parent command (for macro commands).
    EditShapeCommand(QGraphicsItem *item,
                     const QJsonObject &oldParams,
                     const QJsonObject &newParams,
                     QUndoCommand *parent = nullptr);

    void undo() override; ///< Restores old geometry/pen.
    void redo() override; ///< Applies new geometry/pen.

private:
    void applyParams(const QJsonObject &params);

    QGraphicsItem *m_item;
    QJsonObject    m_old;
    QJsonObject    m_new;
};
