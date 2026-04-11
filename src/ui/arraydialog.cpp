#include "arraydialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

// ─────────────────────────────────────────────────────────────────────────────
// MirrorDialog
// ─────────────────────────────────────────────────────────────────────────────

MirrorDialog::MirrorDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Зеркало"));
    setMinimumWidth(260);

    m_axis = new QComboBox(this);
    m_axis->addItem(tr("X (YZ плоскость)"), 0);
    m_axis->addItem(tr("Y (XZ плоскость)"), 1);
    m_axis->addItem(tr("Z (XY плоскость)"), 2);

    auto *form = new QFormLayout;
    form->addRow(tr("Ось симметрии:"), m_axis);

    auto *btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *lay = new QVBoxLayout(this);
    lay->addLayout(form);
    lay->addWidget(btns);
}

MirrorParams MirrorDialog::params() const
{
    return MirrorParams{ m_axis->currentData().toInt() };
}

std::optional<MirrorParams> MirrorDialog::getParams(QWidget *parent)
{
    MirrorDialog dlg(parent);
    if (dlg.exec() != QDialog::Accepted)
        return std::nullopt;
    return dlg.params();
}

// ─────────────────────────────────────────────────────────────────────────────
// LinearArrayDialog
// ─────────────────────────────────────────────────────────────────────────────

static QDoubleSpinBox *makeStepSpin(double val, QWidget *parent)
{
    auto *sb = new QDoubleSpinBox(parent);
    sb->setRange(-100.0, 100.0);
    sb->setSingleStep(0.5);
    sb->setDecimals(2);
    sb->setValue(val);
    sb->setFixedWidth(80);
    return sb;
}

LinearArrayDialog::LinearArrayDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Линейный массив"));
    setMinimumWidth(300);

    m_dx = makeStepSpin(2.0, this);
    m_dy = makeStepSpin(0.0, this);
    m_dz = makeStepSpin(0.0, this);

    m_count = new QSpinBox(this);
    m_count->setRange(2, 100);
    m_count->setValue(3);
    m_count->setFixedWidth(80);

    // Step row (X / Y / Z inline)
    auto *stepWidget = new QWidget(this);
    auto *stepLay    = new QHBoxLayout(stepWidget);
    stepLay->setContentsMargins(0, 0, 0, 0);
    stepLay->addWidget(new QLabel("X:")); stepLay->addWidget(m_dx);
    stepLay->addWidget(new QLabel("Y:")); stepLay->addWidget(m_dy);
    stepLay->addWidget(new QLabel("Z:")); stepLay->addWidget(m_dz);

    auto *form = new QFormLayout;
    form->addRow(tr("Шаг:"),         stepWidget);
    form->addRow(tr("Кол-во копий:"), m_count);

    auto *btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *lay = new QVBoxLayout(this);
    lay->addLayout(form);
    lay->addWidget(btns);
}

LinearArrayParams LinearArrayDialog::params() const
{
    return LinearArrayParams{
        QVector3D(static_cast<float>(m_dx->value()),
                  static_cast<float>(m_dy->value()),
                  static_cast<float>(m_dz->value())),
        m_count->value()
    };
}

std::optional<LinearArrayParams> LinearArrayDialog::getParams(QWidget *parent)
{
    LinearArrayDialog dlg(parent);
    if (dlg.exec() != QDialog::Accepted)
        return std::nullopt;
    return dlg.params();
}

// ─────────────────────────────────────────────────────────────────────────────
// CircularArrayDialog
// ─────────────────────────────────────────────────────────────────────────────

CircularArrayDialog::CircularArrayDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Круговой массив"));
    setMinimumWidth(300);

    auto makeCentreSpin = [this](double val) {
        auto *sb = new QDoubleSpinBox(this);
        sb->setRange(-200.0, 200.0);
        sb->setSingleStep(1.0);
        sb->setDecimals(2);
        sb->setValue(val);
        sb->setFixedWidth(80);
        return sb;
    };

    m_cx = makeCentreSpin(0.0);
    m_cz = makeCentreSpin(0.0);

    m_count = new QSpinBox(this);
    m_count->setRange(2, 360);
    m_count->setValue(6);
    m_count->setFixedWidth(80);

    // Centre row (X / Z inline)
    auto *centreWidget = new QWidget(this);
    auto *centreLay    = new QHBoxLayout(centreWidget);
    centreLay->setContentsMargins(0, 0, 0, 0);
    centreLay->addWidget(new QLabel("X:")); centreLay->addWidget(m_cx);
    centreLay->addWidget(new QLabel("Z:")); centreLay->addWidget(m_cz);

    auto *form = new QFormLayout;
    form->addRow(tr("Центр (XZ):"),   centreWidget);
    form->addRow(tr("Кол-во копий:"), m_count);

    auto *btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *lay = new QVBoxLayout(this);
    lay->addLayout(form);
    lay->addWidget(btns);
}

CircularArrayParams CircularArrayDialog::params() const
{
    return CircularArrayParams{
        static_cast<float>(m_cx->value()),
        static_cast<float>(m_cz->value()),
        m_count->value()
    };
}

std::optional<CircularArrayParams> CircularArrayDialog::getParams(QWidget *parent)
{
    CircularArrayDialog dlg(parent);
    if (dlg.exec() != QDialog::Accepted)
        return std::nullopt;
    return dlg.params();
}
