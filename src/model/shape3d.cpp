#include "shape3d.h"

#include <QJsonArray>

// ── Type helpers ──────────────────────────────────────────────────────────────

QString shapeTypeName(ShapeType3D t)
{
    switch (t) {
        case ShapeType3D::Box:      return QObject::tr("Параллелепипед");
        case ShapeType3D::Sphere:   return QObject::tr("Сфера");
        case ShapeType3D::Cylinder: return QObject::tr("Цилиндр");
        case ShapeType3D::Cone:     return QObject::tr("Конус");
        case ShapeType3D::Torus:    return QObject::tr("Тор");
    }
    return {};
}

ShapeType3D shapeTypeFromString(const QString &s)
{
    if (s == "sphere")   return ShapeType3D::Sphere;
    if (s == "cylinder") return ShapeType3D::Cylinder;
    if (s == "cone")     return ShapeType3D::Cone;
    if (s == "torus")    return ShapeType3D::Torus;
    return ShapeType3D::Box;
}

QString shapeTypeToString(ShapeType3D t)
{
    switch (t) {
        case ShapeType3D::Box:      return "box";
        case ShapeType3D::Sphere:   return "sphere";
        case ShapeType3D::Cylinder: return "cylinder";
        case ShapeType3D::Cone:     return "cone";
        case ShapeType3D::Torus:    return "torus";
    }
    return "box";
}

// ── Shape3DParams ─────────────────────────────────────────────────────────────

static QJsonArray vec3ToJson(const QVector3D &v)
{
    return QJsonArray{v.x(), v.y(), v.z()};
}

static QVector3D vec3FromJson(const QJsonArray &a)
{
    return QVector3D(
        static_cast<float>(a.at(0).toDouble()),
        static_cast<float>(a.at(1).toDouble()),
        static_cast<float>(a.at(2).toDouble())
    );
}

QJsonObject Shape3DParams::toJson() const
{
    return QJsonObject{
        {"name",     name},
        {"type",     shapeTypeToString(type)},
        {"color",    color.name()},
        {"xSize",    xSize},
        {"ySize",    ySize},
        {"zSize",    zSize},
        {"radius2",  radius2},
        {"position", vec3ToJson(position)},
        {"rotation", vec3ToJson(rotation)},
        {"scale",    vec3ToJson(scale)}
    };
}

Shape3DParams Shape3DParams::fromJson(const QJsonObject &obj)
{
    Shape3DParams p;
    p.name    = obj["name"].toString("Объект");
    p.type    = shapeTypeFromString(obj["type"].toString("box"));
    p.color   = QColor(obj["color"].toString("#4682b4"));
    p.xSize   = static_cast<float>(obj["xSize"].toDouble(2.0));
    p.ySize   = static_cast<float>(obj["ySize"].toDouble(2.0));
    p.zSize   = static_cast<float>(obj["zSize"].toDouble(2.0));
    p.radius2 = static_cast<float>(obj["radius2"].toDouble(0.4));
    p.position = vec3FromJson(obj["position"].toArray());
    p.rotation = vec3FromJson(obj["rotation"].toArray());
    p.scale    = vec3FromJson(obj["scale"].toArray({1,1,1}));
    return p;
}
