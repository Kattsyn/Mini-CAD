#include "drawingscene.h"
#include "commands.h"
#include "shapedialog.h"
#include "shapes.h"

#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QVarLengthArray>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSvgGenerator>
#include <QPainter>
#include <QUndoStack>

// ── File-local helpers ────────────────────────────────────────────────────────

/// Rounds @p p to the nearest 50 px grid node.
static QPointF snapPoint(QPointF p)
{
    constexpr double g = 50.0;
    return QPointF(qRound(p.x() / g) * g, qRound(p.y() / g) * g);
}

static QJsonObject shapeToJson(QGraphicsItem *item)
{
    switch (item->type()) {
        case LineItem::Type:   return static_cast<LineItem   *>(item)->toJson();
        case CircleItem::Type: return static_cast<CircleItem *>(item)->toJson();
        case RectItem::Type:   return static_cast<RectItem   *>(item)->toJson();
        default: return {};
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// DrawingScene
// ═════════════════════════════════════════════════════════════════════════════

DrawingScene::DrawingScene(QObject *parent)
    : QGraphicsScene(parent)
    , m_undoStack(new QUndoStack(this))
{
    setSceneRect(-2000, -2000, 4000, 4000);
    setBackgroundBrush(Qt::white);
}

void DrawingScene::setTool(DrawTool tool)
{
    m_tool = tool;
    clearSelection();

    const bool selectMode = (tool == DrawTool::Select);
    for (auto *item : items()) {
        auto flags = item->flags();
        if (selectMode)
            flags |=  QGraphicsItem::ItemIsMovable;
        else
            flags &= ~QGraphicsItem::ItemIsMovable;
        item->setFlags(flags);
    }
}

// ── Document I/O ──────────────────────────────────────────────────────────────

void DrawingScene::newDocument()
{
    clear();
    m_undoStack->clear();
    m_preview = nullptr;
    m_drawing = false;
}

bool DrawingScene::saveToFile(const QString &path) const
{
    QJsonArray arr;
    for (auto *item : items()) {
        const QJsonObject obj = shapeToJson(item);
        if (!obj.isEmpty())
            arr.append(obj);
    }

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    f.write(QJsonDocument(QJsonObject{{"shapes", arr}}).toJson());
    return true;
}

bool DrawingScene::loadFromFile(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    const auto doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject())
        return false;

    newDocument();

    for (const auto &val : doc.object()["shapes"].toArray()) {
        const auto obj  = val.toObject();
        const auto type = obj["type"].toString();

        QGraphicsItem *item = nullptr;
        if      (type == "line")   item = LineItem::fromJson(obj);
        else if (type == "circle") item = CircleItem::fromJson(obj);
        else if (type == "rect")   item = RectItem::fromJson(obj);

        if (item) addItem(item);
    }
    return true;
}

bool DrawingScene::exportToSvg(const QString &path)
{
    QRectF bounds = itemsBoundingRect().adjusted(-20, -20, 20, 20);
    if (bounds.isEmpty())
        bounds = QRectF(0, 0, 800, 600);

    QSvgGenerator gen;
    gen.setFileName(path);
    gen.setSize(bounds.size().toSize());
    gen.setViewBox(bounds);
    gen.setTitle("SAPR Export");
    gen.setDescription("Exported from Мини-САПР");

    QPainter painter;
    painter.begin(&gen);
    painter.setRenderHint(QPainter::Antialiasing);
    render(&painter, QRectF(), bounds);
    painter.end();
    return true;
}

void DrawingScene::deleteSelected()
{
    const QList<QGraphicsItem *> sel = selectedItems();
    if (sel.isEmpty())
        return;
    m_undoStack->push(new RemoveShapesCommand(this, sel));
}

// ── Copy / Paste ──────────────────────────────────────────────────────────────

void DrawingScene::copySelected()
{
    m_clipboard = QJsonArray{};
    for (auto *item : selectedItems()) {
        const QJsonObject obj = shapeToJson(item);
        if (!obj.isEmpty())
            m_clipboard.append(obj);
    }
}

void DrawingScene::paste()
{
    if (m_clipboard.isEmpty())
        return;

    clearSelection();

    m_undoStack->beginMacro(tr("Вставить"));
    for (const auto &val : std::as_const(m_clipboard)) {
        QJsonObject obj = val.toObject();

        // Shift geometry by 20 px so pasted items don't overlap originals
        const QString type = obj["type"].toString();
        if (type == "line") {
            obj["x1"] = obj["x1"].toDouble() + 20;
            obj["y1"] = obj["y1"].toDouble() + 20;
            obj["x2"] = obj["x2"].toDouble() + 20;
            obj["y2"] = obj["y2"].toDouble() + 20;
        } else {
            obj["x"] = obj["x"].toDouble() + 20;
            obj["y"] = obj["y"].toDouble() + 20;
        }

        QGraphicsItem *item = nullptr;
        if      (type == "line")   item = LineItem::fromJson(obj);
        else if (type == "circle") item = CircleItem::fromJson(obj);
        else if (type == "rect")   item = RectItem::fromJson(obj);

        if (item) {
            item->setFlag(QGraphicsItem::ItemIsMovable, m_tool == DrawTool::Select);
            m_undoStack->push(new AddShapeCommand(this, item));
            item->setSelected(true);
        }
    }
    m_undoStack->endMacro();
}

// ── Background grid ───────────────────────────────────────────────────────────

void DrawingScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsScene::drawBackground(painter, rect);

    constexpr int grid = 50;

    painter->setPen(QPen(QColor(230, 230, 230), 0));
    QVarLengthArray<QLineF, 200> lines;

    const int left = static_cast<int>(rect.left()) - (static_cast<int>(rect.left()) % grid);
    const int top  = static_cast<int>(rect.top())  - (static_cast<int>(rect.top())  % grid);

    for (int x = left; x <= static_cast<int>(rect.right());  x += grid)
        lines.append(QLineF(x, rect.top(), x, rect.bottom()));
    for (int y = top;  y <= static_cast<int>(rect.bottom()); y += grid)
        lines.append(QLineF(rect.left(), y, rect.right(), y));

    painter->drawLines(lines.data(), lines.size());

    painter->setPen(QPen(QColor(190, 190, 190), 1));
    painter->drawLine(QLineF(rect.left(), 0, rect.right(), 0));
    painter->drawLine(QLineF(0, rect.top(), 0, rect.bottom()));
}

// ── Mouse event handling ──────────────────────────────────────────────────────

void DrawingScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_tool == DrawTool::Select) {
        // Snapshot positions of all items so we can detect moves on release
        m_preMovePositions.clear();
        for (auto *item : items())
            m_preMovePositions[item] = item->pos();
        QGraphicsScene::mousePressEvent(event);
        return;
    }

    if (event->button() != Qt::LeftButton)
        return;

    m_startPos = m_snapToGrid ? snapPoint(event->scenePos()) : event->scenePos();
    m_drawing  = true;

    switch (m_tool) {
        case DrawTool::Line:
            m_preview = addLine(QLineF(m_startPos, m_startPos), m_pen);
            break;
        case DrawTool::Circle:
            m_preview = addEllipse(QRectF(m_startPos, QSizeF(0, 0)), m_pen);
            break;
        case DrawTool::Rectangle:
            m_preview = addRect(QRectF(m_startPos, QSizeF(0, 0)), m_pen);
            break;
        default:
            break;
    }
}

void DrawingScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_drawing || !m_preview) {
        QGraphicsScene::mouseMoveEvent(event);
        return;
    }

    const QPointF cur = m_snapToGrid ? snapPoint(event->scenePos()) : event->scenePos();

    switch (m_tool) {
        case DrawTool::Line:
            static_cast<QGraphicsLineItem    *>(m_preview)->setLine(QLineF(m_startPos, cur));
            break;
        case DrawTool::Circle:
            static_cast<QGraphicsEllipseItem *>(m_preview)->setRect(QRectF(m_startPos, cur).normalized());
            break;
        case DrawTool::Rectangle:
            static_cast<QGraphicsRectItem    *>(m_preview)->setRect(QRectF(m_startPos, cur).normalized());
            break;
        default:
            break;
    }

    emit statusChanged(QString("x: %1   y: %2")
                       .arg(cur.x(), 0, 'f', 1)
                       .arg(cur.y(), 0, 'f', 1));
}

void DrawingScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    // ── Select mode: detect drag-moves and record them as commands ────────────
    if (m_tool == DrawTool::Select) {
        QGraphicsScene::mouseReleaseEvent(event);

        QList<ItemMove> moves;
        for (auto *item : items()) {
            if (!m_preMovePositions.contains(item))
                continue;
            const QPointF oldP = m_preMovePositions[item];
            const QPointF newP = item->pos();
            if (oldP != newP)
                moves.append({item, oldP, newP});
        }

        if (!moves.isEmpty()) {
            // Items are already at newPos; reset them so redo() can apply newPos
            for (const auto &m : moves)
                m.item->setPos(m.oldPos);
            m_undoStack->push(new MoveShapesCommand(moves));
        }

        m_preMovePositions.clear();
        return;
    }

    // ── Drawing mode: finalise shape ──────────────────────────────────────────
    if (!m_drawing || !m_preview) {
        QGraphicsScene::mouseReleaseEvent(event);
        return;
    }

    m_drawing = false;
    const QPointF endPos = m_snapToGrid ? snapPoint(event->scenePos()) : event->scenePos();

    removeItem(m_preview);
    delete m_preview;
    m_preview = nullptr;

    if (QLineF(m_startPos, endPos).length() < 2.0)
        return;

    QGraphicsItem *final = nullptr;
    switch (m_tool) {
        case DrawTool::Line:
            final = new LineItem(QLineF(m_startPos, endPos), m_pen);
            break;
        case DrawTool::Circle:
            final = new CircleItem(QRectF(m_startPos, endPos).normalized(), m_pen);
            break;
        case DrawTool::Rectangle:
            final = new RectItem(QRectF(m_startPos, endPos).normalized(), m_pen);
            break;
        default:
            break;
    }

    if (final) {
        // Shapes start as non-movable; movability is toggled by setTool(Select)
        final->setFlag(QGraphicsItem::ItemIsMovable, false);
        m_undoStack->push(new AddShapeCommand(this, final));
    }
}

void DrawingScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_tool != DrawTool::Select) {
        QGraphicsScene::mouseDoubleClickEvent(event);
        return;
    }

    QGraphicsItem *item = itemAt(event->scenePos(), QTransform());
    if (!item) {
        QGraphicsScene::mouseDoubleClickEvent(event);
        return;
    }

    const QJsonObject oldParams = shapeToJson(item);
    if (oldParams.isEmpty()) {
        QGraphicsScene::mouseDoubleClickEvent(event);
        return;
    }

    // Open parametric edit dialog (blocking)
    const QJsonObject newParams = ShapeParamsDialog::editShape(item, nullptr);
    if (newParams != oldParams)
        m_undoStack->push(new EditShapeCommand(item, oldParams, newParams));

    event->accept();
}

// ═════════════════════════════════════════════════════════════════════════════
// DrawingView
// ═════════════════════════════════════════════════════════════════════════════

DrawingView::DrawingView(DrawingScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent)
{
    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setDragMode(QGraphicsView::RubberBandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
}

void DrawingView::wheelEvent(QWheelEvent *event)
{
    const qreal factor = event->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;
    scale(factor, factor);
    event->accept();
}

void DrawingView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        if (auto *ds = qobject_cast<DrawingScene *>(scene()))
            ds->deleteSelected();
        event->accept();
        return;
    }
    QGraphicsView::keyPressEvent(event);
}
