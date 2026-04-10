#pragma once

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QHash>
#include <QJsonArray>
#include <QPen>
#include <QUndoStack>

/**
 * @file drawingscene.h
 * @brief Interactive drawing canvas and viewport.
 */

// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Identifies the active drawing tool.
 *
 * | Value     | Behaviour |
 * |-----------|-----------|
 * | Select    | Rubber-band selection; shapes are draggable |
 * | Line      | Draw a line segment |
 * | Circle    | Draw an ellipse/circle |
 * | Rectangle | Draw an axis-aligned rectangle |
 */
enum class DrawTool { Select, Line, Circle, Rectangle };

// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Graphics scene with interactive shape creation, parametric editing,
 *        and a built-in undo/redo stack.
 *
 * ### Usage
 * 1. Set the active tool with setTool().
 * 2. Set the pen with setCurrentPen().
 * 3. User draws shapes by dragging; each finalised shape is pushed onto
 *    the QUndoStack as an AddShapeCommand.
 * 4. Double-clicking a shape in Select mode opens ShapeParamsDialog for
 *    parametric editing (wrapped in an EditShapeCommand).
 * 5. Drag-moves are detected on mouse release and wrapped in
 *    MoveShapesCommand.
 *
 * ### File I/O
 * - saveToFile() / loadFromFile() use a JSON format (.sapr).
 * - exportToSvg() renders the scene to an SVG file.
 */
class DrawingScene : public QGraphicsScene {
    Q_OBJECT
public:
    /// @brief Constructs an empty scene with a 4000×4000 scene rect.
    explicit DrawingScene(QObject *parent = nullptr);

    /// @name Tool and pen
    /// @{

    /// @brief Activates @p tool; updates item movability accordingly.
    void         setTool(DrawTool tool);

    /// @brief Returns the currently active drawing tool.
    DrawTool     tool()  const { return m_tool; }

    /// @brief Sets the pen used for newly created shapes.
    void         setCurrentPen(const QPen &pen) { m_pen = pen; }

    /// @brief Returns the pen used for newly created shapes.
    const QPen  &currentPen()  const { return m_pen; }

    /// @}

    /// @name Undo / redo
    /// @{

    /// @brief Returns the undo stack (use with QUndoView for history display).
    QUndoStack *undoStack() const { return m_undoStack; }

    /// @}

    /// @name Document operations
    /// @{

    /// @brief Clears the scene and resets the undo stack.
    void newDocument();

    /// @brief Saves all shapes to a JSON .sapr file at @p path.
    /// @return @c true on success.
    bool saveToFile(const QString &path) const;

    /// @brief Loads shapes from a JSON .sapr file at @p path.
    /// @return @c true on success.
    bool loadFromFile(const QString &path);

    /// @brief Exports the scene contents to an SVG file at @p path.
    /// @return @c true on success.
    bool exportToSvg(const QString &path);

    /// @}

    /// @brief Removes the currently selected items via an undoable command.
    void deleteSelected();

    /// @name Copy / Paste
    /// @{

    /// @brief Copies selected shapes to the internal clipboard.
    void copySelected();

    /// @brief Pastes shapes from the clipboard with a +20 px offset (undoable).
    void paste();

    /// @brief Returns true if the clipboard has shapes ready to paste.
    bool canPaste() const { return !m_clipboard.isEmpty(); }

    /// @}

    /// @name Snap to grid
    /// @{

    /// @brief Enables or disables snapping to the 50 px grid while drawing.
    void setSnapToGrid(bool snap) { m_snapToGrid = snap; }

    /// @brief Returns whether snap-to-grid is active.
    bool snapToGrid() const { return m_snapToGrid; }

    /// @}

signals:
    /// @brief Emitted with a "x: … y: …" string while drawing to update the status bar.
    void statusChanged(const QString &msg);

protected:
    /// @brief Draws the background grid and coordinate axes.
    void drawBackground(QPainter *painter, const QRectF &rect) override;

    void mousePressEvent(QGraphicsSceneMouseEvent *event)       override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event)        override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event)     override;

    /// @brief Opens the parametric edit dialog when double-clicking a shape
    ///        in Select mode.
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    DrawTool        m_tool{DrawTool::Select};
    QPen            m_pen{Qt::black, 2};
    QPointF         m_startPos;
    QGraphicsItem  *m_preview{nullptr};
    bool            m_drawing{false};

    QUndoStack                     *m_undoStack{nullptr};
    QHash<QGraphicsItem *, QPointF>  m_preMovePositions; ///< Snapshot taken on mouse press for move-tracking.

    QJsonArray  m_clipboard;          ///< Internal copy/paste buffer.
    bool        m_snapToGrid{false};  ///< Whether to snap draw points to the 50 px grid.
};

// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Viewport for DrawingScene with mouse-wheel zoom and Delete key support.
 *
 * - Wheel up/down → scale the view by ±15 %.
 * - Delete / Backspace → calls DrawingScene::deleteSelected().
 */
class DrawingView : public QGraphicsView {
    Q_OBJECT
public:
    /// @brief Constructs a view for the given @p scene.
    explicit DrawingView(DrawingScene *scene, QWidget *parent = nullptr);

protected:
    void wheelEvent(QWheelEvent *event)   override;
    void keyPressEvent(QKeyEvent  *event) override;
};
