#pragma once

#include <QDockWidget>
#include "shape3d.h"

class QLabel;
class QGraphicsItem;

/**
 * @brief Dock panel showing properties of the currently selected object.
 *
 * In 3D mode, shows name, type, position, rotation, scale, and dimensions
 * of the selected Shape3DParams.
 *
 * In 2D mode, shows type and geometric measurements (length for lines,
 * radius/area for circles, width/height/area for rectangles).
 */
class PropertyPanel : public QDockWidget
{
    Q_OBJECT
public:
    explicit PropertyPanel(QWidget *parent = nullptr);

public slots:
    /// Display properties of a 3D shape.
    void showShape3D(const Shape3DParams &p);

    /// Display properties of a 2D QGraphicsItem.
    void showShape2D(QGraphicsItem *item);

    /// Clear the panel (nothing selected).
    void clear();

private:
    /// Sets the label row text; creates row if needed.
    void setRow(const QString &key, const QString &value);
    void buildRows(const QStringList &keys);

    QWidget     *m_content;
    QList<QLabel*> m_keyLabels;
    QList<QLabel*> m_valLabels;
};
