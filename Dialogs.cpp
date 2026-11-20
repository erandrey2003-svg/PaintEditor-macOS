#include "Dialogs.h"
#include "Icons.h"
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>
#include <QGroupBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QButtonGroup>
#include <QFileInfo>
#include <QLocale>

// ================================================================ ResizeDialog

ResizeDialog::ResizeDialog(const QSize &current, QWidget *parent) : QDialog(parent), m_orig(current) {
    setWindowTitle("Изменение размеров и наклона");
    auto *v = new QVBoxLayout(this);

    auto *sizeBox = new QGroupBox("Изменить размер");
    auto *g = new QGridLayout(sizeBox);
    m_percent = new QRadioButton("Проценты");
    m_pixels = new QRadioButton("Пиксели");
    m_percent->setChecked(true);
    auto *byRow = new QHBoxLayout;
    byRow->addWidget(new QLabel("По:"));
    byRow->addWidget(m_percent);
    byRow->addWidget(m_pixels);
    byRow->addStretch();
    g->addLayout(byRow, 0, 0, 1, 2);
    m_w = new QSpinBox;
    m_h = new QSpinBox;
    for (auto *s : {m_w, m_h}) { s->setRange(1, 50000); s->setValue(100); }
    g->addWidget(new QLabel("По горизонтали:"), 1, 0);
    g->addWidget(m_w, 1, 1);
    g->addWidget(new QLabel("По вертикали:"), 2, 0);
    g->addWidget(m_h, 2, 1);
    m_keep = new QCheckBox("Сохранить пропорции");
    m_keep->setChecked(true);
    g->addWidget(m_keep, 3, 0, 1, 2);
    v->addWidget(sizeBox);

    auto *skewBox = new QGroupBox("Наклон (градусы)");
    auto *g2 = new QGridLayout(skewBox);
    m_sx = new QSpinBox;
    m_sy = new QSpinBox;
    for (auto *s : {m_sx, m_sy}) s->setRange(-89, 89);
    g2->addWidget(new QLabel("По горизонтали:"), 0, 0);
    g2->addWidget(m_sx, 0, 1);
    g2->addWidget(new QLabel("По вертикали:"), 1, 0);
    g2->addWidget(m_sy, 1, 1);
    v->addWidget(skewBox);

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    bb->button(QDialogButtonBox::Ok)->setText("ОК");
    bb->button(QDialogButtonBox::Cancel)->setText("Отмена");
    connect(bb, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
    v->addWidget(bb);

    connect(m_w, &QSpinBox::valueChanged, this, [this]() { sync(true); });
    connect(m_h, &QSpinBox::valueChanged, this, [this]() { sync(false); });
    connect(m_pixels, &QRadioButton::toggled, this, [this](bool px) {
        m_updating = true;
        m_w->setValue(px ? m_orig.width() : 100);
        m_h->setValue(px ? m_orig.height() : 100);
        m_updating = false;
    });
}

void ResizeDialog::sync(bool fromH) {
    if (m_updating || !m_keep->isChecked()) return;
    m_updating = true;
    if (m_percent->isChecked()) {
        (fromH ? m_h : m_w)->setValue((fromH ? m_w : m_h)->value());
    } else if (fromH) {
        m_h->setValue(std::max(1, qRound(m_w->value() * double(m_orig.height()) / m_orig.width())));
    } else {
        m_w->setValue(std::max(1, qRound(m_h->value() * double(m_orig.width()) / m_orig.height())));
    }
    m_updating = false;
}

QSize ResizeDialog::newSize() const {
    if (m_pixels->isChecked()) return QSize(m_w->value(), m_h->value());
    return QSize(std::max(1, qRound(m_orig.width() * m_w->value() / 100.0)),
                 std::max(1, qRound(m_orig.height() * m_h->value() / 100.0)));
}
double ResizeDialog::skewX() const { return m_sx->value(); }
double ResizeDialog::skewY() const { return m_sy->value(); }

// ================================================================ PropertiesDialog

PropertiesDialog::PropertiesDialog(const QSize &size, const QString &fileName, const QDateTime &saved,
                                   QWidget *parent)
    : QDialog(parent), m_size(size) {
    setWindowTitle("Свойства изображения");
    auto *v = new QVBoxLayout(this);

    auto *attr = new QGroupBox("Атрибуты файла");
    auto *ga = new QGridLayout(attr);
    QFileInfo fi(fileName);
    ga->addWidget(new QLabel("Последнее сохранение:"), 0, 0);
    ga->addWidget(new QLabel(saved.isValid() ? QLocale().toString(saved, QLocale::ShortFormat) : "Недоступно"), 0, 1);
    ga->addWidget(new QLabel("Размер на диске:"), 1, 0);
    ga->addWidget(new QLabel(fi.exists() ? QLocale().formattedDataSize(fi.size()) : "Недоступно"), 1, 1);
    ga->addWidget(new QLabel("Разрешение:"), 2, 0);
    ga->addWidget(new QLabel("96 точек на дюйм"), 2, 1);
    v->addWidget(attr);

    auto *row = new QHBoxLayout;
    auto *units = new QGroupBox("Единицы измерения");
    auto *vu = new QVBoxLayout(units);
    m_in = new QRadioButton("Дюймы");
    m_cm = new QRadioButton("Сантиметры");
    m_px = new QRadioButton("Пиксели");
    m_px->setChecked(true);
    vu->addWidget(m_in); vu->addWidget(m_cm); vu->addWidget(m_px);
    row->addWidget(units);

    auto *colors = new QGroupBox("Цвета");
    auto *vc = new QVBoxLayout(colors);
    m_bw = new QRadioButton("Черно-белый");
    m_color = new QRadioButton("Цветной");
    m_color->setChecked(true);
    vc->addWidget(m_bw); vc->addWidget(m_color); vc->addStretch();
    row->addWidget(colors);
    v->addLayout(row);

    auto *gs = new QGridLayout;
    m_w = new QDoubleSpinBox;
    m_h = new QDoubleSpinBox;
    for (auto *s : {m_w, m_h}) s->setRange(0.01, 50000);
    gs->addWidget(new QLabel("Ширина:"), 0, 0);
    gs->addWidget(m_w, 0, 1);
    gs->addWidget(new QLabel("Высота:"), 0, 2);
    gs->addWidget(m_h, 0, 3);
    v->addLayout(gs);
    showSize();

    for (auto *r : {m_in, m_cm, m_px})
        connect(r, &QRadioButton::toggled, this, [this](bool on) {
            if (!on) return;
            // пересчитываем введённые числа в новые единицы
            m_size = QSize(qRound(m_w->value() * m_lastFactor), qRound(m_h->value() * m_lastFactor));
            showSize();
        });

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
    bb->button(QDialogButtonBox::Ok)->setText("ОК");
    bb->button(QDialogButtonBox::Cancel)->setText("Отмена");
    bb->button(QDialogButtonBox::RestoreDefaults)->setText("По умолчанию");
    connect(bb, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(bb->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, this, [this]() {
        m_size = QSize(1920, 1080);
        m_px->setChecked(true);
        m_color->setChecked(true);
        showSize();
    });
    v->addWidget(bb);
}

double PropertiesDialog::unitFactor() const {
    if (m_in->isChecked()) return 96.0;
    if (m_cm->isChecked()) return 96.0 / 2.54;
    return 1.0;
}

void PropertiesDialog::showSize() {
    m_lastFactor = unitFactor();
    const int dec = m_px->isChecked() ? 0 : 2;
    m_w->setDecimals(dec);
    m_h->setDecimals(dec);
    m_w->setValue(m_size.width() / m_lastFactor);
    m_h->setValue(m_size.height() / m_lastFactor);
}

QSize PropertiesDialog::newSize() const {
    return QSize(std::max(1, qRound(m_w->value() * unitFactor())), std::max(1, qRound(m_h->value() * unitFactor())));
}

bool PropertiesDialog::blackAndWhite() const { return m_bw->isChecked(); }
