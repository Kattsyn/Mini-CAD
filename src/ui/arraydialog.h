#pragma once

#include <QDialog>
#include <QVector3D>
#include <optional>

class QComboBox;
class QDoubleSpinBox;
class QSpinBox;

// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Parameters returned by MirrorDialog.
 */
struct MirrorParams {
    /// Axis to mirror around: 0=X, 1=Y, 2=Z
    int axis{0};
};

/**
 * @brief Dialog for mirroring a 3D shape across X, Y, or Z axis.
 */
class MirrorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MirrorDialog(QWidget *parent = nullptr);

    /// Returns current dialog values.
    MirrorParams params() const;

    /// Convenience: show dialog and return params if accepted, else nullopt.
    static std::optional<MirrorParams> getParams(QWidget *parent);

private:
    QComboBox *m_axis;
};

// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Parameters returned by LinearArrayDialog.
 */
struct LinearArrayParams {
    QVector3D step{2.0f, 0.0f, 0.0f};  ///< Offset between copies
    int       count{3};                  ///< Total number of copies (including original)
};

/**
 * @brief Dialog for creating a linear array of a 3D shape.
 *
 * Specifies step vector (X/Y/Z offset per copy) and total copy count.
 */
class LinearArrayDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LinearArrayDialog(QWidget *parent = nullptr);

    /// Returns current dialog values.
    LinearArrayParams params() const;

    /// Convenience: show dialog and return params if accepted, else nullopt.
    static std::optional<LinearArrayParams> getParams(QWidget *parent);

private:
    QDoubleSpinBox *m_dx, *m_dy, *m_dz;
    QSpinBox       *m_count;
};

// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Parameters returned by CircularArrayDialog.
 */
struct CircularArrayParams {
    float cx{0.0f};   ///< Centre X of the circular array
    float cz{0.0f};   ///< Centre Z of the circular array
    int   count{6};   ///< Number of copies (including original)
};

/**
 * @brief Dialog for creating a circular array of a 3D shape around the Y axis.
 *
 * Distributes @p count copies evenly around a centre point (cx, cz)
 * at the same Y level as the original shape.
 */
class CircularArrayDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CircularArrayDialog(QWidget *parent = nullptr);

    /// Returns current dialog values.
    CircularArrayParams params() const;

    /// Convenience: show dialog and return params if accepted, else nullopt.
    static std::optional<CircularArrayParams> getParams(QWidget *parent);

private:
    QDoubleSpinBox *m_cx, *m_cz;
    QSpinBox       *m_count;
};
