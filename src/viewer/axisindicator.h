#pragma once

#include <QMatrix4x4>
#include <QWidget>

/**
 * @brief Small overlay widget showing X/Y/Z axis arrows.
 *
 * Place it in a corner of the 3D viewport and call updateView()
 * whenever the camera view matrix changes.
 * The axes rotate with the camera so the user always sees the
 * current world orientation.
 */
class AxisIndicator : public QWidget
{
    Q_OBJECT
public:
    explicit AxisIndicator(QWidget *parent = nullptr);

    /// Feed the current camera view matrix to update the display.
    void updateView(const QMatrix4x4 &viewMatrix);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QMatrix4x4 m_view;
};
