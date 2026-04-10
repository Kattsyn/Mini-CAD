#pragma once

#include <QDockWidget>

class DrawingScene;
class QGraphicsItem;
class QListWidget;
class QListWidgetItem;

/**
 * @brief Dock panel showing all shapes in the scene as a list.
 *
 * Clicking an item in the list selects the corresponding shape on the canvas,
 * and vice-versa. The list is refreshed automatically whenever the undo
 * stack changes (add / remove / undo / redo).
 */
class ShapeListDock : public QDockWidget
{
    Q_OBJECT
public:
    explicit ShapeListDock(DrawingScene *scene, QWidget *parent = nullptr);

    /// Rebuilds the list from the current scene contents.
    void refresh();

private slots:
    void onListSelectionChanged();
    void onSceneSelectionChanged();

private:
    DrawingScene *m_scene;
    QListWidget  *m_list;
    bool          m_syncing{false};
};
