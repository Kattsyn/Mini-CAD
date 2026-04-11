#pragma once

#include <QDockWidget>

class DrawingScene;
class QGraphicsItem;
class QListWidget;
class QListWidgetItem;
class Viewer3D;

/**
 * @brief Dock panel showing all shapes as a list.
 *
 * Supports both 2D (DrawingScene) and 3D (Viewer3D) modes.
 * Call setMode(true) when switching to the 3D tab and
 * setMode(false) when switching back.
 */
class ShapeListDock : public QDockWidget
{
    Q_OBJECT
public:
    explicit ShapeListDock(DrawingScene *scene, Viewer3D *viewer3d,
                           QWidget *parent = nullptr);

    /// Rebuilds the list from the current active model.
    void refresh();

    /// Switch between 2D (false) and 3D (true) display modes.
    void setMode(bool is3D);

private slots:
    void onListItemClicked(QListWidgetItem *item);
    void onSceneSelectionChanged();
    void on3DSelectionChanged(int index);

private:
    void refresh2D();
    void refresh3D();

    DrawingScene *m_scene;
    Viewer3D     *m_viewer3d;
    QListWidget  *m_list;
    bool          m_is3D   {false};
    bool          m_syncing{false};
};
