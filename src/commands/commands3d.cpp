#include "commands3d.h"
#include "viewer3d.h"

// ─────────────────────────────────────────────────────────────────────────────
// AddShape3DCommand
// ─────────────────────────────────────────────────────────────────────────────

AddShape3DCommand::AddShape3DCommand(Viewer3D *viewer, const Shape3DParams &params,
                                     QUndoCommand *parent)
    : QUndoCommand(QObject::tr("Добавить 3D фигуру"), parent)
    , m_viewer(viewer)
    , m_params(params)
{}

void AddShape3DCommand::redo()
{
    m_index = m_viewer->shapeCount();
    m_viewer->cmdInsert(m_index, m_params);
    m_viewer->setSelectedIndex(m_index);
}

void AddShape3DCommand::undo()
{
    m_viewer->cmdRemove(m_index);
}

// ─────────────────────────────────────────────────────────────────────────────
// RemoveShape3DCommand
// ─────────────────────────────────────────────────────────────────────────────

RemoveShape3DCommand::RemoveShape3DCommand(Viewer3D *viewer, int index,
                                           QUndoCommand *parent)
    : QUndoCommand(QObject::tr("Удалить 3D фигуру"), parent)
    , m_viewer(viewer)
    , m_index(index)
    , m_saved(viewer->shapes().at(index))
{}

void RemoveShape3DCommand::redo()
{
    m_viewer->cmdRemove(m_index);
}

void RemoveShape3DCommand::undo()
{
    m_viewer->cmdInsert(m_index, m_saved);
    m_viewer->setSelectedIndex(m_index);
}

// ─────────────────────────────────────────────────────────────────────────────
// EditShape3DCommand
// ─────────────────────────────────────────────────────────────────────────────

EditShape3DCommand::EditShape3DCommand(Viewer3D *viewer, int index,
                                       const Shape3DParams &oldP,
                                       const Shape3DParams &newP,
                                       QUndoCommand *parent)
    : QUndoCommand(QObject::tr("Изменить 3D фигуру"), parent)
    , m_viewer(viewer)
    , m_index(index)
    , m_old(oldP)
    , m_new(newP)
{}

void EditShape3DCommand::redo()  { m_viewer->cmdReplace(m_index, m_new); }
void EditShape3DCommand::undo()  { m_viewer->cmdReplace(m_index, m_old); }
