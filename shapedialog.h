#pragma once

#include <QJsonObject>

class QGraphicsItem;
class QWidget;

// ─────────────────────────────────────────────────────────────────────────────
// ShapeParamsDialog
// ─────────────────────────────────────────────────────────────────────────────

/// @brief Utility class for parametric editing of shape properties.
///
/// Shows a modal dialog with numeric input fields for the geometry and pen
/// of any supported shape type (line, circle/ellipse, rectangle).
/// All coordinates are in scene space.
class ShapeParamsDialog
{
public:
    /// Shows a parameter dialog for @p item and returns the new JSON params.
    /// If the user cancels, the original params are returned unchanged.
    /// @return Updated JSON snapshot (same format as shape's toJson()).
    static QJsonObject editShape(QGraphicsItem *item, QWidget *parent = nullptr);

private:
    static QJsonObject editLine   (const QJsonObject &p, QWidget *parent);
    static QJsonObject editEllipse(const QJsonObject &p, QWidget *parent);
    static QJsonObject editRect   (const QJsonObject &p, QWidget *parent);
};
