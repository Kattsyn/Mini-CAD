#include "mainwindow.h"
#include "shapelistdock.h"
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
#include <QToolBar>
#include <QToolButton>
#include <QUndoStack>
#include <QUndoView>

// ─────────────────────────────────────────────────────────────────────────────
// Construction / destruction
// ─────────────────────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_scene(new DrawingScene(this))
    , m_view(new DrawingView(m_scene, this))
    , m_coordLabel(new QLabel(this))
{
    ui->setupUi(this);

    setCentralWidget(m_view);

    statusBar()->addPermanentWidget(m_coordLabel);

    setupMenus();
    setupToolBar();
    setupShapeListDock();
    setupHistoryDock();

    connect(m_scene, &DrawingScene::statusChanged,
            m_coordLabel, &QLabel::setText);

    updateTitle();
    resize(1280, 800);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ─────────────────────────────────────────────────────────────────────────────
// Menu setup
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::setupMenus()
{
    // Helper: create a QAction with shortcut and add to menu
    auto addAct = [&](QMenu *menu, const QString &text,
                      const QKeySequence &shortcut, auto slot) -> QAction * {
        auto *act = menu->addAction(text);
        act->setShortcut(shortcut);
        connect(act, &QAction::triggered, this, slot);
        return act;
    };

    // ── File ──────────────────────────────────────────────────────────────────
    QMenu *fileMenu = menuBar()->addMenu(tr("Файл"));

    addAct(fileMenu, tr("Новый"),          QKeySequence::New,              &MainWindow::onNew);
    addAct(fileMenu, tr("Открыть…"),       QKeySequence::Open,             &MainWindow::onOpen);
    fileMenu->addSeparator();
    addAct(fileMenu, tr("Сохранить"),      QKeySequence::Save,             &MainWindow::onSave);
    addAct(fileMenu, tr("Сохранить как…"), tr("Ctrl+Shift+S"),             &MainWindow::onSaveAs);
    fileMenu->addSeparator();
    addAct(fileMenu, tr("Экспорт SVG…"),   tr("Ctrl+E"),                   &MainWindow::onExportSvg);
    fileMenu->addSeparator();
    addAct(fileMenu, tr("Выход"),          QKeySequence::Quit,             &MainWindow::close);

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

    {
        auto *act = editMenu->addAction(tr("Выделить всё"));
        act->setShortcut(QKeySequence::SelectAll);
        connect(act, &QAction::triggered, m_scene, &QGraphicsScene::clearSelection);
    }
    addAct(editMenu, tr("Удалить выбранное"), QKeySequence::Delete, &MainWindow::onDeleteSelected);

    // ── View ──────────────────────────────────────────────────────────────────
    QMenu *viewMenu = menuBar()->addMenu(tr("Вид"));

    addAct(viewMenu, tr("Увеличить"),   QKeySequence::ZoomIn,  &MainWindow::onZoomIn);
    addAct(viewMenu, tr("Уменьшить"),   QKeySequence::ZoomOut, &MainWindow::onZoomOut);
    addAct(viewMenu, tr("По размеру…"), tr("Ctrl+0"),          &MainWindow::onFitView);
}

// ─────────────────────────────────────────────────────────────────────────────
// Toolbar setup
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::setupToolBar()
{
    QToolBar *tb = addToolBar(tr("Инструменты"));
    tb->setMovable(false);

    // ── Drawing tool buttons (exclusive) ──────────────────────────────────────
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
        tb->addAction(act);

        connect(act, &QAction::triggered, this, [this, tool]() {
            m_scene->setTool(tool);
            m_view->setDragMode(tool == DrawTool::Select
                                ? QGraphicsView::RubberBandDrag
                                : QGraphicsView::NoDrag);
        });
    }

    tb->addSeparator();

    // ── Colour picker ─────────────────────────────────────────────────────────
    m_colorBtn = new QToolButton(tb);
    m_colorBtn->setFixedSize(36, 36);
    m_colorBtn->setToolTip(tr("Выбрать цвет"));
    refreshColorButton();
    connect(m_colorBtn, &QToolButton::clicked, this, &MainWindow::onPickColor);
    tb->addWidget(m_colorBtn);

    // ── Line width ────────────────────────────────────────────────────────────
    tb->addWidget(new QLabel(tr("  Толщина: ")));
    auto *widthSpin = new QSpinBox(tb);
    widthSpin->setRange(1, 20);
    widthSpin->setValue(static_cast<int>(m_pen.widthF()));
    widthSpin->setFixedWidth(60);
    connect(widthSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onLineWidthChanged);
    tb->addWidget(widthSpin);

    tb->addSeparator();

    // ── Undo / Redo ───────────────────────────────────────────────────────────
    tb->addAction(m_scene->undoStack()->createUndoAction(this, tr("Отменить")));
    tb->addAction(m_scene->undoStack()->createRedoAction(this, tr("Повторить")));
    tb->addSeparator();

    // ── Snap to grid ──────────────────────────────────────────────────────────
    auto *snapAct = new QAction(tr("Сетка"), this);
    snapAct->setCheckable(true);
    snapAct->setChecked(false);
    snapAct->setToolTip(tr("Привязка к сетке (50 px)"));
    connect(snapAct, &QAction::toggled, m_scene, &DrawingScene::setSnapToGrid);
    tb->addAction(snapAct);
    tb->addSeparator();

    // ── Delete ────────────────────────────────────────────────────────────────
    tb->addAction(tr("Удалить"), this, &MainWindow::onDeleteSelected);
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
// Slots — Edit
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
// History dock
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::setupShapeListDock()
{
    auto *dock = new ShapeListDock(m_scene, this);
    addDockWidget(Qt::LeftDockWidgetArea, dock);
}

void MainWindow::setupHistoryDock()
{
    auto *dock = new QDockWidget(tr("История операций"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    auto *undoView = new QUndoView(m_scene->undoStack(), dock);
    undoView->setEmptyLabel(tr("<нет операций>"));
    dock->setWidget(undoView);

    addDockWidget(Qt::RightDockWidgetArea, dock);
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
