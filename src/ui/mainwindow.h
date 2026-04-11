#pragma once

#include <QMainWindow>
#include <QPen>

#include "drawingscene.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class PropertyPanel;
class QDockWidget;
class QLabel;
class QTabWidget;
class QToolBar;
class QToolButton;
class QUndoStack;
class QUndoView;
class ShapeListDock;
class Viewer3D;


/// @brief Main application window for Мини-САПР.
///
/// Owns the DrawingScene (2D) and Viewer3D (3D), switching between them via
/// a QTabWidget. Provides menus/toolbars, file I/O, undo/redo, shape list,
/// operation-history dock, and a property panel.
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    // 2D file / edit
    void onNew();
    void onOpen();
    void onSave();
    void onSaveAs();
    void onExportSvg();
    void onPickColor();
    void onLineWidthChanged(int w);
    void onDeleteSelected();
    void onCopy();
    void onPaste();
    void onZoomIn();
    void onZoomOut();
    void onFitView();

    // 3D actions
    void on3DAdd();
    void on3DRemove();
    void on3DEdit();
    void on3DMirror();
    void on3DLinearArray();
    void on3DCircularArray();

    // Tab switching
    void onTabChanged(int index);

private:
    void setupMenus();
    void setupToolBar();
    void setup3DToolBar();
    void setupShapeListDock();
    void setupHistoryDock();
    void setupPropertyDock();
    void updateTitle();
    void refreshColorButton();

    Ui::MainWindow *ui;
    DrawingScene   *m_scene;
    DrawingView    *m_view;
    Viewer3D       *m_viewer3d;
    QTabWidget     *m_tabs;
    QToolBar       *m_toolbar2d{nullptr};
    QToolBar       *m_toolbar3d{nullptr};
    PropertyPanel  *m_propPanel{nullptr};
    QUndoView      *m_undoView{nullptr};
    ShapeListDock  *m_shapeListDock{nullptr};
    QLabel         *m_coordLabel;
    QToolButton    *m_colorBtn{nullptr};
    QPen            m_pen{Qt::black, 2};
    QString         m_currentFile;
};
