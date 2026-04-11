#pragma once

#include <QUndoCommand>
#include "shape3d.h"

class Viewer3D;

/**
 * @file commands3d.h
 * @brief Undo/redo commands for 3D shape operations.
 */

// ─────────────────────────────────────────────────────────────────────────────

/// @brief Undo command: add a 3D shape to the viewer.
class AddShape3DCommand : public QUndoCommand
{
public:
    AddShape3DCommand(Viewer3D *viewer, const Shape3DParams &params,
                      QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
private:
    Viewer3D      *m_viewer;
    Shape3DParams  m_params;
    int            m_index{-1};
};

// ─────────────────────────────────────────────────────────────────────────────

/// @brief Undo command: remove a 3D shape from the viewer.
class RemoveShape3DCommand : public QUndoCommand
{
public:
    RemoveShape3DCommand(Viewer3D *viewer, int index,
                         QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
private:
    Viewer3D      *m_viewer;
    int            m_index;
    Shape3DParams  m_saved;
};

// ─────────────────────────────────────────────────────────────────────────────

/// @brief Undo command: replace the parameters of an existing 3D shape.
class EditShape3DCommand : public QUndoCommand
{
public:
    EditShape3DCommand(Viewer3D *viewer, int index,
                       const Shape3DParams &oldP, const Shape3DParams &newP,
                       QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
private:
    Viewer3D      *m_viewer;
    int            m_index;
    Shape3DParams  m_old, m_new;
};
