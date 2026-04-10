#pragma once

#include <QGraphicsItem>
#include <QJsonObject>
#include <QPen>

/**
 * @file shapes.h
 * @brief Geometric shape items and JSON serialisation helpers.
 */

/// @brief Converts a QPen to a JSON object for serialisation.
/// @param pen  The pen to serialise.
/// @return JSON object with keys "color" (hex string) and "width" (number).
QJsonObject penToJson(const QPen &pen);

/// @brief Restores a QPen from a JSON object produced by penToJson().
/// @param obj  JSON object with keys "color" and "width".
/// @return Restored QPen; defaults to black, width 1 if keys are missing.
QPen penFromJson(const QJsonObject &obj);

// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief A selectable, movable line segment on the drawing canvas.
 *
 * Wraps QGraphicsLineItem and adds JSON serialisation so the shape can be
 * saved to / loaded from a .sapr project file.
 */
class LineItem : public QGraphicsLineItem {
public:
    /// @cond INTERNAL
    enum { Type = UserType + 1 };
    int type() const override { return Type; }
    /// @endcond

    /// @brief Constructs a line from @p line with the given @p pen.
    LineItem(const QLineF &line, const QPen &pen);

    /// @brief Serialises the line to JSON (scene-space coordinates).
    QJsonObject toJson() const;

    /// @brief Creates a LineItem from a JSON object produced by toJson().
    /// @return Heap-allocated item; caller takes ownership.
    static LineItem *fromJson(const QJsonObject &obj);
};

// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief A selectable, movable ellipse (or circle) on the drawing canvas.
 *
 * Geometry is stored and serialised as a bounding rectangle.
 * The parametric editing dialog exposes centre + radii for usability.
 */
class CircleItem : public QGraphicsEllipseItem {
public:
    /// @cond INTERNAL
    enum { Type = UserType + 2 };
    int type() const override { return Type; }
    /// @endcond

    /// @brief Constructs an ellipse from bounding @p rect with the given @p pen.
    CircleItem(const QRectF &rect, const QPen &pen);

    /// @brief Serialises the ellipse to JSON (scene-space coordinates).
    QJsonObject toJson() const;

    /// @brief Creates a CircleItem from a JSON object produced by toJson().
    /// @return Heap-allocated item; caller takes ownership.
    static CircleItem *fromJson(const QJsonObject &obj);
};

// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief A selectable, movable axis-aligned rectangle on the drawing canvas.
 */
class RectItem : public QGraphicsRectItem {
public:
    /// @cond INTERNAL
    enum { Type = UserType + 3 };
    int type() const override { return Type; }
    /// @endcond

    /// @brief Constructs a rectangle from @p rect with the given @p pen.
    RectItem(const QRectF &rect, const QPen &pen);

    /// @brief Serialises the rectangle to JSON (scene-space coordinates).
    QJsonObject toJson() const;

    /// @brief Creates a RectItem from a JSON object produced by toJson().
    /// @return Heap-allocated item; caller takes ownership.
    static RectItem *fromJson(const QJsonObject &obj);
};
