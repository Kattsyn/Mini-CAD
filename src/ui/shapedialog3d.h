#pragma once

#include <QDialog>
#include "shape3d.h"

class QComboBox;
class QDoubleSpinBox;
class QLineEdit;
class QPushButton;

/**
 * @brief Dialog for creating or editing a 3D shape.
 *
 * Shows fields for:
 * - Name and type (Box / Sphere / Cylinder / Cone / Torus)
 * - Dimensions (depend on type)
 * - Colour
 * - Transform: position (X/Y/Z), rotation (X/Y/Z°), scale (X/Y/Z)
 */
class ShapeDialog3D : public QDialog
{
    Q_OBJECT
public:
    explicit ShapeDialog3D(QWidget *parent = nullptr);

    /// Pre-fills all fields from @p params.
    void setParams(const Shape3DParams &params);

    /// Returns the current field values as Shape3DParams.
    Shape3DParams params() const;

    /// Convenience: show dialog and return params if accepted, else nullopt.
    static std::optional<Shape3DParams> getParams(
        QWidget *parent,
        const Shape3DParams &initial = Shape3DParams{});

private slots:
    void onTypeChanged(int index);
    void onPickColor();

private:
    QDoubleSpinBox *makeSpin(double val, double mn, double mx, double step = 0.1);
    void           refreshColorButton();
    void           updateDimensionLabels();

    QLineEdit      *m_name;
    QComboBox      *m_typeCombo;
    QPushButton    *m_colorBtn;
    QColor          m_color{QColor(70, 130, 180)};

    // Dimension spins (labelled dynamically)
    QDoubleSpinBox *m_xSize;
    QDoubleSpinBox *m_ySize;
    QDoubleSpinBox *m_zSize;
    QDoubleSpinBox *m_r2;

    // Transform
    QDoubleSpinBox *m_px, *m_py, *m_pz;  // position
    QDoubleSpinBox *m_rx, *m_ry, *m_rz;  // rotation degrees
    QDoubleSpinBox *m_sx, *m_sy, *m_sz;  // scale
};
