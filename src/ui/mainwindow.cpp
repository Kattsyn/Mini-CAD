#include "mainwindow.h"
#include "arraydialog.h"
#include "propertypanel.h"
#include "shapedialog3d.h"
#include "shapelistdock.h"
#include "viewer3d.h"
#include "ui_mainwindow.h"

#include <QActionGroup>
#include <QColorDialog>
#include <QDockWidget>
#include <QFileDialog>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmap>
#include <QSpinBox>
#include <QStatusBar>
#include <QFileInfo>
#include <QTabWidget>
#include <QToolBar>
#include <QToolButton>
#include <QUndoStack>
#include <QUndoView>
#include <QtMath>

// ─────────────────────────────────────────────────────────────────────────────
// Construction / destruction
// ─────────────────────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_scene(new DrawingScene(this))
    , m_view(new DrawingView(m_scene, this))
    , m_viewer3d(new Viewer3D(this))
    , m_tabs(new QTabWidget(this))
    , m_coordLabel(new QLabel(this))
{
    ui->setupUi(this);

    // Central widget: tab between 2D and 3D
    m_tabs->addTab(m_view,     tr("2D Чертёж"));
    m_tabs->addTab(m_viewer3d, tr("3D Модель"));
    setCentralWidget(m_tabs);
    connect(m_tabs, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);

    statusBar()->addPermanentWidget(m_coordLabel);

    setupMenus();
    setupToolBar();
    setup3DToolBar();
    setupShapeListDock();
    setupHistoryDock();
    setupPropertyDock();

    connect(m_scene, &DrawingScene::statusChanged,
            m_coordLabel, &QLabel::setText);

    // 3D selection → property panel
    connect(m_viewer3d, &Viewer3D::selectionChanged, this, [this](int /*idx*/) {
        if (m_propPanel)
            m_propPanel->showShape3D(m_viewer3d->selectedShape());
    });
    // 2D selection → property panel
    connect(m_scene, &QGraphicsScene::selectionChanged, this, [this]() {
        if (!m_propPanel) return;
        const auto sel = m_scene->selectedItems();
        if (sel.isEmpty())
            m_propPanel->clear();
        else
            m_propPanel->showShape2D(sel.first());
    });

    // Start in 2D mode
    onTabChanged(0);

    updateTitle();
    resize(1280, 800);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ─────────────────────────────────────────────────────────────────────────────
// Tab switching
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::onTabChanged(int index)
{
    const bool is3D = (index == 1);
    if (m_toolbar2d) m_toolbar2d->setVisible(!is3D);
    if (m_toolbar3d) m_toolbar3d->setVisible(is3D);

    // Switch history dock to the active tab's undo stack
    if (m_undoView) {
        QUndoStack *stack = is3D ? m_viewer3d->undoStack()
                                 : m_scene->undoStack();
        m_undoView->setStack(stack);
    }

    // Switch shape list to the active tab's model
    if (m_shapeListDock)
        m_shapeListDock->setMode(is3D);
}

// ─────────────────────────────────────────────────────────────────────────────
// Menu setup
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::setupMenus()
{
    auto addAct = [&](QMenu *menu, const QString &text,
                      const QKeySequence &shortcut, auto slot) -> QAction * {
        auto *act = menu->addAction(text);
        act->setShortcut(shortcut);
        connect(act, &QAction::triggered, this, slot);
        return act;
    };

    // ── File ──────────────────────────────────────────────────────────────────
    QMenu *fileMenu = menuBar()->addMenu(tr("Файл"));

    addAct(fileMenu, tr("Новый"),          QKeySequence::New,    &MainWindow::onNew);
    addAct(fileMenu, tr("Открыть…"),       QKeySequence::Open,   &MainWindow::onOpen);
    fileMenu->addSeparator();
    addAct(fileMenu, tr("Сохранить"),      QKeySequence::Save,   &MainWindow::onSave);
    addAct(fileMenu, tr("Сохранить как…"), tr("Ctrl+Shift+S"),   &MainWindow::onSaveAs);
    fileMenu->addSeparator();
    addAct(fileMenu, tr("Экспорт SVG…"),   tr("Ctrl+E"),         &MainWindow::onExportSvg);
    fileMenu->addSeparator();
    addAct(fileMenu, tr("Выход"),          QKeySequence::Quit,   &MainWindow::close);

    // ── Edit ──────────────────────────────────────────────────────────────────
    QMenu *editMenu = menuBar()->addMenu(tr("Правка"));

    {
        QAction *undoAct = m_scene->undoStack()->createUndoAction(this, tr("Отменить"));
        undoAct->setShortcut(QKeySequence::Undo);
        editMenu->addAction(undoAct);

        QAction *redoAct = m_scene->undoStack()->createRedoAction(this, tr("Повторить"));
        redoAct->setShortcut(QKeySequence::Redo);
        editMenu->addAction(redoAct);
    }
    editMenu->addSeparator();

    addAct(editMenu, tr("Копировать"), QKeySequence::Copy,  &MainWindow::onCopy);
    addAct(editMenu, tr("Вставить"),   QKeySequence::Paste, &MainWindow::onPaste);
    editMenu->addSeparator();
    addAct(editMenu, tr("Удалить выбранное"), QKeySequence::Delete, &MainWindow::onDeleteSelected);

    // ── View ──────────────────────────────────────────────────────────────────
    QMenu *viewMenu = menuBar()->addMenu(tr("Вид"));

    addAct(viewMenu, tr("Увеличить"),   QKeySequence::ZoomIn,  &MainWindow::onZoomIn);
    addAct(viewMenu, tr("Уменьшить"),   QKeySequence::ZoomOut, &MainWindow::onZoomOut);
    addAct(viewMenu, tr("По размеру…"), tr("Ctrl+0"),          &MainWindow::onFitView);
}

// ─────────────────────────────────────────────────────────────────────────────
// 2D Toolbar
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::setupToolBar()
{
    m_toolbar2d = addToolBar(tr("Инструменты 2D"));
    m_toolbar2d->setMovable(false);

    auto *toolGroup = new QActionGroup(this);
    toolGroup->setExclusive(true);

    struct Entry { QString label; DrawTool tool; };
    const QList<Entry> tools = {
        { tr("Выбор"),       DrawTool::Select    },
        { tr("Линия"),       DrawTool::Line      },
        { tr("Окружность"),  DrawTool::Circle    },
        { tr("Прямоугол."),  DrawTool::Rectangle },
    };

    for (const auto &entry : tools) {
        const QString  label = entry.label;
        const DrawTool tool  = entry.tool;
        auto *act = toolGroup->addAction(label);
        act->setCheckable(true);
        if (tool == DrawTool::Select)
            act->setChecked(true);
        m_toolbar2d->addAction(act);

        connect(act, &QAction::triggered, this, [this, tool]() {
            m_scene->setTool(tool);
            m_view->setDragMode(tool == DrawTool::Select
                                ? QGraphicsView::RubberBandDrag
                                : QGraphicsView::NoDrag);
        });
    }

    m_toolbar2d->addSeparator();

    m_colorBtn = new QToolButton(m_toolbar2d);
    m_colorBtn->setFixedSize(36, 36);
    m_colorBtn->setToolTip(tr("Выбрать цвет"));
    refreshColorButton();
    connect(m_colorBtn, &QToolButton::clicked, this, &MainWindow::onPickColor);
    m_toolbar2d->addWidget(m_colorBtn);

    m_toolbar2d->addWidget(new QLabel(tr("  Толщина: ")));
    auto *widthSpin = new QSpinBox(m_toolbar2d);
    widthSpin->setRange(1, 20);
    widthSpin->setValue(static_cast<int>(m_pen.widthF()));
    widthSpin->setFixedWidth(60);
    connect(widthSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onLineWidthChanged);
    m_toolbar2d->addWidget(widthSpin);

    m_toolbar2d->addSeparator();
    m_toolbar2d->addAction(m_scene->undoStack()->createUndoAction(this, tr("Отменить")));
    m_toolbar2d->addAction(m_scene->undoStack()->createRedoAction(this, tr("Повторить")));
    m_toolbar2d->addSeparator();

    auto *snapAct = new QAction(tr("Сетка"), this);
    snapAct->setCheckable(true);
    snapAct->setChecked(false);
    snapAct->setToolTip(tr("Привязка к сетке (50 px)"));
    connect(snapAct, &QAction::toggled, m_scene, &DrawingScene::setSnapToGrid);
    m_toolbar2d->addAction(snapAct);
    m_toolbar2d->addSeparator();

    m_toolbar2d->addAction(tr("Удалить"), this, &MainWindow::onDeleteSelected);
}

// ─────────────────────────────────────────────────────────────────────────────
// 3D Toolbar
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::setup3DToolBar()
{
    m_toolbar3d = addToolBar(tr("Инструменты 3D"));
    m_toolbar3d->setMovable(false);

    m_toolbar3d->addAction(tr("Добавить"),       this, &MainWindow::on3DAdd);
    m_toolbar3d->addAction(tr("Удалить"),         this, &MainWindow::on3DRemove);
    m_toolbar3d->addAction(tr("Редактировать"),   this, &MainWindow::on3DEdit);
    m_toolbar3d->addSeparator();
    m_toolbar3d->addAction(tr("Зеркало"),         this, &MainWindow::on3DMirror);
    m_toolbar3d->addAction(tr("Лин. массив"),     this, &MainWindow::on3DLinearArray);
    m_toolbar3d->addAction(tr("Кр. массив"),      this, &MainWindow::on3DCircularArray);
    m_toolbar3d->addSeparator();
    m_toolbar3d->addAction(m_viewer3d->undoStack()->createUndoAction(this, tr("Отменить")));
    m_toolbar3d->addAction(m_viewer3d->undoStack()->createRedoAction(this, tr("Повторить")));

    m_toolbar3d->setVisible(false);  // hidden until 3D tab is active
}

// ─────────────────────────────────────────────────────────────────────────────
// 3D Action slots
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::on3DAdd()
{
    const auto p = ShapeDialog3D::getParams(this);
    if (p)
        m_viewer3d->addShape(*p);
}

void MainWindow::on3DRemove()
{
    m_viewer3d->removeSelected();
}

void MainWindow::on3DEdit()
{
    if (m_viewer3d->selectedIndex() < 0) return;
    const Shape3DParams cur = m_viewer3d->selectedShape();
    const auto p = ShapeDialog3D::getParams(this, cur);
    if (p)
        m_viewer3d->editSelected(*p);
}

void MainWindow::on3DMirror()
{
    if (m_viewer3d->selectedIndex() < 0) return;
    const auto mp = MirrorDialog::getParams(this);
    if (!mp) return;

    Shape3DParams copy = m_viewer3d->selectedShape();
    copy.name += tr(" (зеркало)");

    switch (mp->axis) {
        case 0: copy.position.setX(-copy.position.x());  break;  // mirror X
        case 1: copy.position.setY(-copy.position.y());  break;  // mirror Y
        case 2: copy.position.setZ(-copy.position.z());  break;  // mirror Z
    }

    m_viewer3d->addShape(copy);
}

void MainWindow::on3DLinearArray()
{
    if (m_viewer3d->selectedIndex() < 0) return;
    const auto lp = LinearArrayDialog::getParams(this);
    if (!lp) return;

    const Shape3DParams base = m_viewer3d->selectedShape();
    for (int i = 1; i < lp->count; ++i) {
        Shape3DParams copy = base;
        copy.name  = base.name + QString(" (%1)").arg(i);
        copy.position = base.position + lp->step * static_cast<float>(i);
        m_viewer3d->addShape(copy);
    }
}

void MainWindow::on3DCircularArray()
{
    if (m_viewer3d->selectedIndex() < 0) return;
    const auto cp = CircularArrayDialog::getParams(this);
    if (!cp) return;

    const Shape3DParams base = m_viewer3d->selectedShape();
    const float cx = cp->cx;
    const float cz = cp->cz;

    // Radius from centre to base shape
    const float dx0 = base.position.x() - cx;
    const float dz0 = base.position.z() - cz;
    const float radius = std::sqrt(dx0 * dx0 + dz0 * dz0);
    const float angleStep = 360.0f / static_cast<float>(cp->count);
    const float baseAngle = std::atan2(dz0, dx0) * 180.0f / static_cast<float>(M_PI);

    for (int i = 1; i < cp->count; ++i) {
        const float angleDeg = baseAngle + angleStep * static_cast<float>(i);
        const float angleRad = angleDeg * static_cast<float>(M_PI) / 180.0f;

        Shape3DParams copy = base;
        copy.name = base.name + QString(" (%1)").arg(i);
        copy.position.setX(cx + radius * std::cos(angleRad));
        copy.position.setZ(cz + radius * std::sin(angleRad));
        copy.rotation.setY(base.rotation.y() + angleStep * static_cast<float>(i));
        m_viewer3d->addShape(copy);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Dock setup
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::setupShapeListDock()
{
    m_shapeListDock = new ShapeListDock(m_scene, m_viewer3d, this);
    addDockWidget(Qt::LeftDockWidgetArea, m_shapeListDock);
}

void MainWindow::setupHistoryDock()
{
    auto *dock = new QDockWidget(tr("История операций"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    m_undoView = new QUndoView(m_scene->undoStack(), dock);
    m_undoView->setEmptyLabel(tr("<нет операций>"));
    dock->setWidget(m_undoView);

    addDockWidget(Qt::RightDockWidgetArea, dock);
}

void MainWindow::setupPropertyDock()
{
    m_propPanel = new PropertyPanel(this);
    m_propPanel->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, m_propPanel);
}

// ─────────────────────────────────────────────────────────────────────────────
// Slots — File
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::onNew()
{
    if (!m_scene->items().isEmpty()) {
        const auto btn = QMessageBox::question(
            this, tr("Новый документ"),
            tr("Создать новый документ? Несохранённые изменения будут потеряны."));
        if (btn != QMessageBox::Yes)
            return;
    }
    m_scene->newDocument();
    m_currentFile.clear();
    updateTitle();
}

void MainWindow::onOpen()
{
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Открыть проект"), QString(),
        tr("Проект САПР (*.sapr);;Все файлы (*)"));
    if (path.isEmpty())
        return;

    if (!m_scene->loadFromFile(path)) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось открыть файл:\n%1").arg(path));
        return;
    }
    m_currentFile = path;
    m_view->fitInView(m_scene->itemsBoundingRect().adjusted(-50, -50, 50, 50),
                      Qt::KeepAspectRatio);
    updateTitle();
}

void MainWindow::onSave()
{
    if (m_currentFile.isEmpty()) {
        onSaveAs();
        return;
    }
    if (!m_scene->saveToFile(m_currentFile))
        QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось сохранить файл:\n%1").arg(m_currentFile));
}

void MainWindow::onSaveAs()
{
    const QString path = QFileDialog::getSaveFileName(
        this, tr("Сохранить как…"), QString(),
        tr("Проект САПР (*.sapr);;Все файлы (*)"));
    if (path.isEmpty())
        return;

    m_currentFile = path;
    if (!m_scene->saveToFile(m_currentFile)) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось сохранить файл:\n%1").arg(m_currentFile));
        m_currentFile.clear();
    }
    updateTitle();
}

void MainWindow::onExportSvg()
{
    const QString path = QFileDialog::getSaveFileName(
        this, tr("Экспорт SVG…"), QString(),
        tr("SVG изображение (*.svg);;Все файлы (*)"));
    if (path.isEmpty())
        return;

    if (!m_scene->exportToSvg(path))
        QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось экспортировать SVG:\n%1").arg(path));
    else
        statusBar()->showMessage(tr("SVG сохранён: %1").arg(path), 3000);
}

// ─────────────────────────────────────────────────────────────────────────────
// Slots — Edit (2D)
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::onDeleteSelected()
{
    m_scene->deleteSelected();
}

void MainWindow::onCopy()
{
    m_scene->copySelected();
}

void MainWindow::onPaste()
{
    m_scene->paste();
}

// ─────────────────────────────────────────────────────────────────────────────
// Slots — Pen properties
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::onPickColor()
{
    const QColor c = QColorDialog::getColor(m_pen.color(), this, tr("Выбрать цвет"));
    if (!c.isValid())
        return;
    m_pen.setColor(c);
    m_scene->setCurrentPen(m_pen);
    refreshColorButton();
}

void MainWindow::onLineWidthChanged(int w)
{
    m_pen.setWidth(w);
    m_scene->setCurrentPen(m_pen);
}

// ─────────────────────────────────────────────────────────────────────────────
// Slots — View
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::onZoomIn()
{
    m_view->scale(1.2, 1.2);
}

void MainWindow::onZoomOut()
{
    m_view->scale(1.0 / 1.2, 1.0 / 1.2);
}

void MainWindow::onFitView()
{
    const QRectF bounds = m_scene->itemsBoundingRect().adjusted(-50, -50, 50, 50);
    if (bounds.isEmpty())
        m_view->resetTransform();
    else
        m_view->fitInView(bounds, Qt::KeepAspectRatio);
}

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::updateTitle()
{
    const QString name = m_currentFile.isEmpty()
                         ? tr("Без имени")
                         : QFileInfo(m_currentFile).fileName();
    setWindowTitle(tr("Мини-САПР — %1").arg(name));
}

void MainWindow::refreshColorButton()
{
    if (!m_colorBtn) return;
    QPixmap px(24, 24);
    px.fill(m_pen.color());
    m_colorBtn->setIcon(QIcon(px));
    m_colorBtn->setToolTip(tr("Цвет: %1").arg(m_pen.color().name()));
}
