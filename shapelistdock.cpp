#include "shapelistdock.h"
#include "drawingscene.h"
#include "shapes.h"

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

ShapeListDock::ShapeListDock(DrawingScene *scene, QWidget *parent)
    : QDockWidget(tr("Фигуры"), parent)
    , m_scene(scene)
    , m_list(new QListWidget)
{
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    setMinimumWidth(160);

    auto *container = new QWidget;
    auto *lay = new QVBoxLayout(container);
    lay->setContentsMargins(4, 4, 4, 4);
    lay->addWidget(m_list);
    setWidget(container);

    m_list->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // Refresh when undo stack changes (add/remove/undo/redo)
    connect(scene->undoStack(), &QUndoStack::indexChanged,
            this, &ShapeListDock::refresh);

    // Sync selection: list → scene
    connect(m_list, &QListWidget::itemSelectionChanged,
            this, &ShapeListDock::onListSelectionChanged);

    // Sync selection: scene → list
    connect(scene, &QGraphicsScene::selectionChanged,
            this, &ShapeListDock::onSceneSelectionChanged);

    refresh();
}

void ShapeListDock::refresh()
{
    m_syncing = true;
    m_list->clear();

    int lineN = 0, circleN = 0, rectN = 0, otherN = 0;

    // items() returns in stacking order (top first); reverse for natural order
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

        const QString name = shapeDisplayName(item, idx);
        auto *lwItem = new QListWidgetItem(name);
        lwItem->setData(Qt::UserRole, (qlonglong)(quintptr)item);
        lwItem->setSelected(item->isSelected());
        m_list->addItem(lwItem);
    }

    m_syncing = false;
}

void ShapeListDock::onListSelectionChanged()
{
    if (m_syncing) return;
    m_syncing = true;

    m_scene->clearSelection();
    for (int i = 0; i < m_list->count(); ++i) {
        auto *lwItem = m_list->item(i);
        if (!lwItem->isSelected()) continue;
        auto *item = (QGraphicsItem *)(quintptr)lwItem->data(Qt::UserRole).toLongLong();
        if (item) item->setSelected(true);
    }

    m_syncing = false;
}

void ShapeListDock::onSceneSelectionChanged()
{
    if (m_syncing) return;
    m_syncing = true;

    for (int i = 0; i < m_list->count(); ++i) {
        auto *lwItem = m_list->item(i);
        auto *item = (QGraphicsItem *)(quintptr)lwItem->data(Qt::UserRole).toLongLong();
        lwItem->setSelected(item && item->isSelected());
    }

    m_syncing = false;
}
