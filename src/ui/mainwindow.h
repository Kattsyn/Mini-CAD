#pragma once

#include <QMainWindow>
#include <QPen>

#include "drawingscene.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QDockWidget;
class QLabel;
class QToolButton;
class QUndoView;
class ShapeListDock;

/// @brief Main application window for Мини-САПР.
///
/// Owns the DrawingScene and DrawingView, provides menus/toolbar for
/// drawing tools, file I/O (open/save/export SVG), undo/redo actions,
/// and an operation-history dock (QUndoView).
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
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

private:
    void setupMenus();
    void setupToolBar();
    void setupShapeListDock();
    void setupHistoryDock();
    void updateTitle();
    void refreshColorButton();

    Ui::MainWindow *ui;
    DrawingScene   *m_scene;
    DrawingView    *m_view;
    QLabel         *m_coordLabel;
    QToolButton    *m_colorBtn{nullptr};
    QPen            m_pen{Qt::black, 2};
    QString         m_currentFile;
};
