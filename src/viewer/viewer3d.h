#pragma once

#include <QList>
#include <QMatrix4x4>
#include <QWidget>

#include "shape3d.h"

class QUndoStack;

namespace Qt3DCore    { class QEntity; }
namespace Qt3DExtras  { class Qt3DWindow; }
namespace Qt3DRender  { class QCamera; class QLayer; class QCameraSelector; }

// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Qt3D-based 3D viewport widget.
 *
 * Embeds a Qt3DExtras::Qt3DWindow in a QWidget via createWindowContainer().
 * Manages a list of Shape3DParams and rebuilds the Qt3D entity graph whenever
 * the model changes.
 *
 * Features a corner axis-orientation gizmo rendered via a second Qt3D viewport
 * (bottom-left 18% of the screen) with its own camera that mirrors the main
 * camera's rotation.
 *
 * ### Camera controls (Qt3DExtras::QOrbitCameraController)
 * - Left drag   — orbit
 * - Right drag  — zoom
 * - Middle drag — pan
 */
class Viewer3D : public QWidget
{
    Q_OBJECT
public:
    explicit Viewer3D(QWidget *parent = nullptr);

    // ── Undo stack ────────────────────────────────────────────────────────────
    QUndoStack *undoStack() const { return m_undoStack; }

    // ── Shape data model ──────────────────────────────────────────────────────

    /// Returns a copy of all shapes in scene order.
    QList<Shape3DParams> shapes() const { return m_shapes; }

    /// Number of shapes in the scene.
    int shapeCount() const { return m_shapes.size(); }

    /// Returns the index of the currently selected shape, or -1.
    int selectedIndex() const { return m_selectedIndex; }

    /// Returns the selected shape's params, or a default if nothing selected.
    Shape3DParams selectedShape() const;

    // ── Mutating operations (go through undo stack) ───────────────────────────

    /// Adds a new shape (undoable).
    void addShape(const Shape3DParams &params);

    /// Removes the selected shape (undoable). No-op if nothing selected.
    void removeSelected();

    /// Replaces the selected shape's params (undoable). No-op if nothing selected.
    void editSelected(const Shape3DParams &newParams);

    // ── Low-level API used by commands ────────────────────────────────────────

    void cmdInsert(int index, const Shape3DParams &params);
    void cmdRemove(int index);
    void cmdReplace(int index, const Shape3DParams &params);

    /// Sets the selected shape index and highlights it.
    void setSelectedIndex(int index);

signals:
    /// Emitted when the selection changes.
    void selectionChanged(int index);

    /// Emitted when the shape list changes (add / remove / edit).
    void shapesChanged();

private:
    void setupScene();
    void setupFrameGraph();
    void setupLights();
    void setupGrid();
    void setupGizmo();
    void syncGizmoCamera(const QMatrix4x4 &viewMatrix);
    void rebuildShapes();
    Qt3DCore::QEntity *buildEntity(const Shape3DParams &params, int index,
                                   Qt3DCore::QEntity *parent);

    Qt3DExtras::Qt3DWindow  *m_window3d      {nullptr};
    Qt3DCore::QEntity       *m_rootEntity    {nullptr};
    Qt3DCore::QEntity       *m_shapesRoot    {nullptr};
    Qt3DRender::QCamera     *m_gizmoCamera   {nullptr};
    Qt3DRender::QLayer      *m_gizmoLayer    {nullptr};
    Qt3DRender::QCameraSelector *m_gizmoCamSel {nullptr};

    QList<Shape3DParams>     m_shapes;
    int                      m_selectedIndex {-1};
    QUndoStack              *m_undoStack     {nullptr};
};
