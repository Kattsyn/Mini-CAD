#include "shapelistdock.h"
#include "drawingscene.h"
#include "shapes.h"
#include "viewer3d.h"

#include <QGraphicsItem>
#include <QListWidget>
#include <QUndoStack>
#include <QVBoxLayout>
#include <QWidget>

// ─────────────────────────────────────────────────────────────────────────────

static QString shapeDisplayName(QGraphicsItem *item, int index)
{
    switch (item->type()) {
        case LineItem::Type:   return QObject::tr("Линия %1").arg(index);
        case CircleItem::Type: return QObject::tr("Окружность %1").arg(index);
        case RectItem::Type:   return QObject::tr("Прямоугольник %1").arg(index);
        default:               return QObject::tr("Фигура %1").arg(index);
    }
}

// ─────────────────────────────────────────────────────────────────────────────

ShapeListDock::ShapeListDock(DrawingScene *scene, Viewer3D *viewer3d,
                             QWidget *parent)
    : QDockWidget(tr("Фигуры"), parent)
    , m_scene(scene)
    , m_viewer3d(viewer3d)
    , m_list(new QListWidget)
{
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    setMinimumWidth(160);

    auto *container = new QWidget;
    auto *lay = new QVBoxLayout(container);
    lay->setContentsMargins(4, 4, 4, 4);
    lay->addWidget(m_list);
    setWidget(container);

    m_list->setSelectionMode(QAbstractItemView::SingleSelection);

    // 2D connections
    connect(scene->undoStack(), &QUndoStack::indexChanged,
            this, &ShapeListDock::refresh);
    connect(scene, &QGraphicsScene::selectionChanged,
            this, &ShapeListDock::onSceneSelectionChanged);

    // 3D connections
    connect(viewer3d, &Viewer3D::shapesChanged,
            this, &ShapeListDock::refresh);
    connect(viewer3d, &Viewer3D::selectionChanged,
            this, &ShapeListDock::on3DSelectionChanged);

    // List click
    connect(m_list, &QListWidget::itemClicked,
            this, &ShapeListDock::onListItemClicked);

    refresh();
}

// ─────────────────────────────────────────────────────────────────────────────

void ShapeListDock::setMode(bool is3D)
{
    m_is3D = is3D;
    refresh();
}

void ShapeListDock::refresh()
{
    m_is3D ? refresh3D() : refresh2D();
}

// ─────────────────────────────────────────────────────────────────────────────

void ShapeListDock::refresh2D()
{
    m_syncing = true;
    m_list->clear();

    int lineN = 0, circleN = 0, rectN = 0, otherN = 0;
    const auto allItems = m_scene->items();
    QList<QGraphicsItem *> ordered(allItems.rbegin(), allItems.rend());

    for (auto *item : ordered) {
        int idx = 0;
        switch (item->type()) {
            case LineItem::Type:   idx = ++lineN;   break;
            case CircleItem::Type: idx = ++circleN; break;
            case RectItem::Type:   idx = ++rectN;   break;
            default:               idx = ++otherN;  break;
        }
        auto *lwItem = new QListWidgetItem(shapeDisplayName(item, idx));
        lwItem->setData(Qt::UserRole, (qlonglong)(quintptr)item);
        lwItem->setSelected(item->isSelected());
        m_list->addItem(lwItem);
    }

    m_syncing = false;
}

void ShapeListDock::refresh3D()
{
    m_syncing = true;
    m_list->clear();

    const auto shapes = m_viewer3d->shapes();
    const int  sel    = m_viewer3d->selectedIndex();

    for (int i = 0; i < shapes.size(); ++i) {
        const auto &p     = shapes[i];
        const QString label = QString("%1 — %2").arg(i + 1).arg(p.name);
        auto *lwItem = new QListWidgetItem(label);
        lwItem->setData(Qt::UserRole, i);
        m_list->addItem(lwItem);
        if (i == sel)
            m_list->setCurrentItem(lwItem);
    }

    m_syncing = false;
}

// ─────────────────────────────────────────────────────────────────────────────

void ShapeListDock::onListItemClicked(QListWidgetItem *item)
{
    if (m_syncing || !item) return;

    if (m_is3D) {
        const int idx = item->data(Qt::UserRole).toInt();
        m_syncing = true;
        m_viewer3d->setSelectedIndex(idx);
        m_syncing = false;
    } else {
        m_syncing = true;
        m_scene->clearSelection();
        auto *gfx = (QGraphicsItem *)(quintptr)item->data(Qt::UserRole).toLongLong();
        if (gfx) gfx->setSelected(true);
        m_syncing = false;
    }
}

void ShapeListDock::onSceneSelectionChanged()
{
    if (m_syncing || m_is3D) return;
    m_syncing = true;

    for (int i = 0; i < m_list->count(); ++i) {
        auto *lwItem = m_list->item(i);
        auto *item = (QGraphicsItem *)(quintptr)lwItem->data(Qt::UserRole).toLongLong();
        lwItem->setSelected(item && item->isSelected());
    }

    m_syncing = false;
}

void ShapeListDock::on3DSelectionChanged(int index)
{
    if (m_syncing || !m_is3D) return;
    m_syncing = true;

    m_list->clearSelection();
    if (index >= 0 && index < m_list->count())
        m_list->setCurrentRow(index);

    m_syncing = false;
}
