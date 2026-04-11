#include "shapedialog3d.h"

#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>

// ─────────────────────────────────────────────────────────────────────────────

ShapeDialog3D::ShapeDialog3D(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Параметры 3D фигуры"));
    setMinimumWidth(360);

    // ── Name + type ───────────────────────────────────────────────────────────
    m_name = new QLineEdit(this);
    m_name->setPlaceholderText(tr("Имя объекта"));

    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItem(shapeTypeName(ShapeType3D::Box),      "box");
    m_typeCombo->addItem(shapeTypeName(ShapeType3D::Sphere),   "sphere");
    m_typeCombo->addItem(shapeTypeName(ShapeType3D::Cylinder), "cylinder");
    m_typeCombo->addItem(shapeTypeName(ShapeType3D::Cone),     "cone");
    m_typeCombo->addItem(shapeTypeName(ShapeType3D::Torus),    "torus");
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ShapeDialog3D::onTypeChanged);

    m_colorBtn = new QPushButton(this);
    connect(m_colorBtn, &QPushButton::clicked, this, &ShapeDialog3D::onPickColor);
    refreshColorButton();

    // ── Dimensions ────────────────────────────────────────────────────────────
    m_xSize = makeSpin(2.0, 0.01, 50.0, 0.5);
    m_ySize = makeSpin(2.0, 0.01, 50.0, 0.5);
    m_zSize = makeSpin(2.0, 0.01, 50.0, 0.5);
    m_r2    = makeSpin(0.4, 0.01, 10.0, 0.1);

    auto *dimGroup = new QGroupBox(tr("Размеры"), this);
    auto *dimForm  = new QFormLayout(dimGroup);
    dimForm->addRow(tr("Размер X:"), m_xSize);
    dimForm->addRow(tr("Размер Y:"), m_ySize);
    dimForm->addRow(tr("Размер Z:"), m_zSize);
    dimForm->addRow(tr("Радиус 2:"), m_r2);

    // ── Transform ─────────────────────────────────────────────────────────────
    m_px = makeSpin(0, -100, 100);  m_py = makeSpin(0, -100, 100);  m_pz = makeSpin(0, -100, 100);
    m_rx = makeSpin(0, -360, 360, 5); m_ry = makeSpin(0, -360, 360, 5); m_rz = makeSpin(0, -360, 360, 5);
    m_sx = makeSpin(1, 0.01, 20, 0.1); m_sy = makeSpin(1, 0.01, 20, 0.1); m_sz = makeSpin(1, 0.01, 20, 0.1);

    auto makeXYZ = [](QDoubleSpinBox *x, QDoubleSpinBox *y, QDoubleSpinBox *z) {
        auto *w = new QWidget;
        auto *h = new QHBoxLayout(w);
        h->setContentsMargins(0,0,0,0);
        h->addWidget(new QLabel("X:")); h->addWidget(x);
        h->addWidget(new QLabel("Y:")); h->addWidget(y);
        h->addWidget(new QLabel("Z:")); h->addWidget(z);
        return w;
    };

    auto *tfGroup = new QGroupBox(tr("Трансформация"), this);
    auto *tfForm  = new QFormLayout(tfGroup);
    tfForm->addRow(tr("Позиция:"),  makeXYZ(m_px, m_py, m_pz));
    tfForm->addRow(tr("Поворот °:"), makeXYZ(m_rx, m_ry, m_rz));
    tfForm->addRow(tr("Масштаб:"),  makeXYZ(m_sx, m_sy, m_sz));

    // ── Top form ──────────────────────────────────────────────────────────────
    auto *topForm = new QFormLayout;
    topForm->addRow(tr("Имя:"),  m_name);
    topForm->addRow(tr("Тип:"),  m_typeCombo);
    topForm->addRow(tr("Цвет:"), m_colorBtn);

    // ── Buttons ───────────────────────────────────────────────────────────────
    auto *btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // ── Layout ────────────────────────────────────────────────────────────────
    auto *mainLay = new QVBoxLayout(this);
    mainLay->addLayout(topForm);
    mainLay->addWidget(dimGroup);
    mainLay->addWidget(tfGroup);
    mainLay->addWidget(btns);

    updateDimensionLabels();
}

// ── Helpers ───────────────────────────────────────────────────────────────────

QDoubleSpinBox *ShapeDialog3D::makeSpin(double val, double mn, double mx, double step)
{
    auto *sb = new QDoubleSpinBox(this);
    sb->setRange(mn, mx);
    sb->setSingleStep(step);
    sb->setDecimals(2);
    sb->setValue(val);
    sb->setFixedWidth(72);
    return sb;
}

void ShapeDialog3D::refreshColorButton()
{
    QPixmap px(20, 20);
    px.fill(m_color);
    m_colorBtn->setIcon(QIcon(px));
    m_colorBtn->setText(m_color.name());
}

void ShapeDialog3D::onPickColor()
{
    QColor c = QColorDialog::getColor(m_color, this, tr("Выбрать цвет"));
    if (c.isValid()) { m_color = c; refreshColorButton(); }
}

void ShapeDialog3D::onTypeChanged(int /*index*/)
{
    updateDimensionLabels();
}

void ShapeDialog3D::updateDimensionLabels()
{
    const ShapeType3D t = shapeTypeFromString(m_typeCombo->currentData().toString());
    // Show/hide dimensions depending on type
    m_zSize->setEnabled(t == ShapeType3D::Box);
    m_r2->setEnabled(t == ShapeType3D::Torus || t == ShapeType3D::Cone);

    auto *form = qobject_cast<QFormLayout*>(m_xSize->parentWidget()->layout());
    if (!form) {
        // dimensions are in a group box form
        auto *box = qobject_cast<QGroupBox*>(m_xSize->parentWidget());
        if (!box) return;
        form = qobject_cast<QFormLayout*>(box->layout());
    }
}

// ── Data accessors ────────────────────────────────────────────────────────────

void ShapeDialog3D::setParams(const Shape3DParams &p)
{
    m_name->setText(p.name);
    // Set combo by matching data string
    for (int i = 0; i < m_typeCombo->count(); ++i) {
        if (m_typeCombo->itemData(i).toString() == shapeTypeToString(p.type)) {
            m_typeCombo->setCurrentIndex(i);
            break;
        }
    }
    m_color = p.color;
    refreshColorButton();

    m_xSize->setValue(p.xSize);
    m_ySize->setValue(p.ySize);
    m_zSize->setValue(p.zSize);
    m_r2->setValue(p.radius2);

    m_px->setValue(p.position.x()); m_py->setValue(p.position.y()); m_pz->setValue(p.position.z());
    m_rx->setValue(p.rotation.x()); m_ry->setValue(p.rotation.y()); m_rz->setValue(p.rotation.z());
    m_sx->setValue(p.scale.x());    m_sy->setValue(p.scale.y());    m_sz->setValue(p.scale.z());
}

Shape3DParams ShapeDialog3D::params() const
{
    Shape3DParams p;
    p.name  = m_name->text().isEmpty() ? tr("Объект") : m_name->text();
    p.type  = shapeTypeFromString(m_typeCombo->currentData().toString());
    p.color = m_color;

    p.xSize   = static_cast<float>(m_xSize->value());
    p.ySize   = static_cast<float>(m_ySize->value());
    p.zSize   = static_cast<float>(m_zSize->value());
    p.radius2 = static_cast<float>(m_r2->value());

    p.position = QVector3D(m_px->value(), m_py->value(), m_pz->value());
    p.rotation = QVector3D(m_rx->value(), m_ry->value(), m_rz->value());
    p.scale    = QVector3D(m_sx->value(), m_sy->value(), m_sz->value());
    return p;
}

std::optional<Shape3DParams> ShapeDialog3D::getParams(QWidget *parent, const Shape3DParams &initial)
{
    ShapeDialog3D dlg(parent);
    dlg.setParams(initial);
    if (dlg.exec() != QDialog::Accepted)
        return std::nullopt;
    return dlg.params();
}
