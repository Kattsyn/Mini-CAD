#pragma once

#include <QColor>
#include <QJsonObject>
#include <QString>
#include <QVector3D>

/**
 * @file shape3d.h
 * @brief Data model for a 3D parametric shape.
 */

/// @brief Type of 3D primitive.
enum class ShapeType3D {
    Box,
    Sphere,
    Cylinder,
    Cone,
    Torus
};

/// @brief Returns a localised display name for a shape type.
QString shapeTypeName(ShapeType3D t);

/// @brief Returns the ShapeType3D for a stored string key.
ShapeType3D shapeTypeFromString(const QString &s);

/// @brief Returns the string key for a ShapeType3D.
QString shapeTypeToString(ShapeType3D t);

// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief All parameters that fully describe a 3D shape.
 *
 * Dimensions are interpreted per type:
 * | Type     | xSize       | ySize      | zSize       | radius2      |
 * |----------|-------------|------------|-------------|--------------|
 * | Box      | width       | height     | depth       | —            |
 * | Sphere   | radius      | —          | —           | —            |
 * | Cylinder | radius      | length     | —           | —            |
 * | Cone     | bottomRadius| length     | —           | topRadius    |
 * | Torus    | majorRadius | —          | —           | minorRadius  |
 */
struct Shape3DParams {
    QString     name   {"Объект"};
    ShapeType3D type   {ShapeType3D::Box};
    QColor      color  {QColor(70, 130, 180)};  // steel blue

    // Primitive dimensions
    float xSize  {2.0f};
    float ySize  {2.0f};
    float zSize  {2.0f};
    float radius2{0.4f};  // torus minor / cone top radius

    // Transform
    QVector3D position{0.0f, 0.0f, 0.0f};
    QVector3D rotation{0.0f, 0.0f, 0.0f};  ///< Euler angles (degrees) around X/Y/Z
    QVector3D scale   {1.0f, 1.0f, 1.0f};

    /// @brief Serialises to JSON.
    QJsonObject toJson() const;

    /// @brief Deserialises from JSON produced by toJson().
    static Shape3DParams fromJson(const QJsonObject &obj);
};
