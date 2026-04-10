#include "shapedialog.h"
#include "shapes.h"

#include <QColorDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGraphicsItem>
#include <QIcon>
#include <QObject>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

static QDoubleSpinBox *makeSpin(double val, double mn = -9999.0, double mx = 9999.0)
{
    auto *sb = new QDoubleSpinBox;
    sb->setRange(mn, mx);
    sb->setDecimals(1);
    sb->setValue(val);
    return sb;
}

static void setupColorButton(QPushButton *btn, QColor &color, QDialog *dlg)
{
    auto refresh = [btn, &color]() {
        QPixmap px(20, 20);
        px.fill(color);
        btn->setIcon(QIcon(px));
        btn->setText(color.name());
    };
    refresh();
    QObject::connect(btn, &QPushButton::clicked, dlg, [btn, &color, refresh, dlg]() {
        QColor c = QColorDialog::getColor(color, dlg);
        if (c.isValid()) { color = c; refresh(); }
    });
}

// ─────────────────────────────────────────────────────────────────────────────
// Public entry point
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject ShapeParamsDialog::editShape(QGraphicsItem *item, QWidget *parent)
{
    if (!item) return {};
    switch (item->type()) {
        case LineItem::Type:
            return editLine(static_cast<LineItem *>(item)->toJson(), parent);
        case CircleItem::Type:
            return editEllipse(static_cast<CircleItem *>(item)->toJson(), parent);
        case RectItem::Type:
            return editRect(static_cast<RectItem *>(item)->toJson(), parent);
        default:
            return {};
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Line editor
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject ShapeParamsDialog::editLine(const QJsonObject &p, QWidget *parent)
{
    QDialog dlg(parent);
    dlg.setWindowTitle(QObject::tr("Параметры: Линия"));

    auto *x1Sp = makeSpin(p["x1"].toDouble());
    auto *y1Sp = makeSpin(p["y1"].toDouble());
    auto *x2Sp = makeSpin(p["x2"].toDouble());
    auto *y2Sp = makeSpin(p["y2"].toDouble());

    QPen  pen    = penFromJson(p["pen"].toObject());
    QColor color = pen.color();
    auto *colorBtn  = new QPushButton(&dlg);
    setupColorButton(colorBtn, color, &dlg);

    auto *widthSp = new QDoubleSpinBox(&dlg);
    widthSp->setRange(0.1, 20.0);
    widthSp->setValue(pen.widthF());

    auto *form = new QFormLayout;
    form->addRow(QObject::tr("X1:"),       x1Sp);
    form->addRow(QObject::tr("Y1:"),       y1Sp);
    form->addRow(QObject::tr("X2:"),       x2Sp);
    form->addRow(QObject::tr("Y2:"),       y2Sp);
    form->addRow(QObject::tr("Цвет:"),     colorBtn);
    form->addRow(QObject::tr("Толщина:"),  widthSp);

    auto *btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    QObject::connect(btns, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    auto *lay = new QVBoxLayout(&dlg);
    lay->addLayout(form);
    lay->addWidget(btns);

    if (dlg.exec() != QDialog::Accepted) return p;

    return QJsonObject{
        {"type", "line"},
        {"x1", x1Sp->value()}, {"y1", y1Sp->value()},
        {"x2", x2Sp->value()}, {"y2", y2Sp->value()},
        {"pen", penToJson(QPen(color, widthSp->value()))}
    };
}

// ─────────────────────────────────────────────────────────────────────────────
// Ellipse / Circle editor  (shows centre + radii for usability)
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject ShapeParamsDialog::editEllipse(const QJsonObject &p, QWidget *parent)
{
    QDialog dlg(parent);
    dlg.setWindowTitle(QObject::tr("Параметры: Окружность / Эллипс"));

    const double bx = p["x"].toDouble(), by = p["y"].toDouble();
    const double bw = p["w"].toDouble(), bh = p["h"].toDouble();

    auto *cxSp = makeSpin(bx + bw / 2.0);
    auto *cySp = makeSpin(by + bh / 2.0);
    auto *rxSp = makeSpin(bw / 2.0, 0.1, 9999.0);
    auto *rySp = makeSpin(bh / 2.0, 0.1, 9999.0);

    QPen   pen   = penFromJson(p["pen"].toObject());
    QColor color = pen.color();
    auto *colorBtn = new QPushButton(&dlg);
    setupColorButton(colorBtn, color, &dlg);

    auto *widthSp = new QDoubleSpinBox(&dlg);
    widthSp->setRange(0.1, 20.0);
    widthSp->setValue(pen.widthF());

    auto *form = new QFormLayout;
    form->addRow(QObject::tr("Центр X:"),   cxSp);
    form->addRow(QObject::tr("Центр Y:"),   cySp);
    form->addRow(QObject::tr("Радиус X:"),  rxSp);
    form->addRow(QObject::tr("Радиус Y:"),  rySp);
    form->addRow(QObject::tr("Цвет:"),      colorBtn);
    form->addRow(QObject::tr("Толщина:"),   widthSp);

    auto *btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    QObject::connect(btns, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    auto *lay = new QVBoxLayout(&dlg);
    lay->addLayout(form);
    lay->addWidget(btns);

    if (dlg.exec() != QDialog::Accepted) return p;

    const double rx = rxSp->value(), ry = rySp->value();
    const double cx = cxSp->value(), cy = cySp->value();
    return QJsonObject{
        {"type", "circle"},
        {"x", cx - rx}, {"y", cy - ry},
        {"w", rx * 2.0}, {"h", ry * 2.0},
        {"pen", penToJson(QPen(color, widthSp->value()))}
    };
}

// ─────────────────────────────────────────────────────────────────────────────
// Rectangle editor
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject ShapeParamsDialog::editRect(const QJsonObject &p, QWidget *parent)
{
    QDialog dlg(parent);
    dlg.setWindowTitle(QObject::tr("Параметры: Прямоугольник"));

    auto *xSp = makeSpin(p["x"].toDouble());
    auto *ySp = makeSpin(p["y"].toDouble());
    auto *wSp = makeSpin(p["w"].toDouble(), 0.1, 9999.0);
    auto *hSp = makeSpin(p["h"].toDouble(), 0.1, 9999.0);

    QPen   pen   = penFromJson(p["pen"].toObject());
    QColor color = pen.color();
    auto *colorBtn = new QPushButton(&dlg);
    setupColorButton(colorBtn, color, &dlg);

    auto *widthSp = new QDoubleSpinBox(&dlg);
    widthSp->setRange(0.1, 20.0);
    widthSp->setValue(pen.widthF());

    auto *form = new QFormLayout;
    form->addRow(QObject::tr("X:"),       xSp);
    form->addRow(QObject::tr("Y:"),       ySp);
    form->addRow(QObject::tr("Ширина:"),  wSp);
    form->addRow(QObject::tr("Высота:"),  hSp);
    form->addRow(QObject::tr("Цвет:"),    colorBtn);
    form->addRow(QObject::tr("Толщина:"), widthSp);

    auto *btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    QObject::connect(btns, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    auto *lay = new QVBoxLayout(&dlg);
    lay->addLayout(form);
    lay->addWidget(btns);

    if (dlg.exec() != QDialog::Accepted) return p;

    return QJsonObject{
        {"type", "rect"},
        {"x", xSp->value()}, {"y", ySp->value()},
        {"w", wSp->value()}, {"h", hSp->value()},
        {"pen", penToJson(QPen(color, widthSp->value()))}
    };
}
