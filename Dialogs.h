#pragma once
#include <QDialog>
#include <QSize>
#include <QDateTime>

class QSpinBox;
class QDoubleSpinBox;
class QRadioButton;
class QCheckBox;

// «Изменение размеров и наклона»
class ResizeDialog : public QDialog {
public:
    ResizeDialog(const QSize &current, QWidget *parent = nullptr);
    QSize newSize() const;
    double skewX() const;
    double skewY() const;

private:
    void sync(bool fromHorizontal);
    QSize m_orig;
    QRadioButton *m_percent, *m_pixels;
    QSpinBox *m_w, *m_h, *m_sx, *m_sy;
    QCheckBox *m_keep;
    bool m_updating = false;
};

// «Свойства изображения»
class PropertiesDialog : public QDialog {
public:
    PropertiesDialog(const QSize &size, const QString &fileName, const QDateTime &saved,
                     QWidget *parent = nullptr);
    QSize newSize() const;
    bool blackAndWhite() const;

private:
    double unitFactor() const;   // пикселей в единице
    void showSize();
    QSize m_size;
    QRadioButton *m_in, *m_cm, *m_px, *m_bw, *m_color;
    QDoubleSpinBox *m_w, *m_h;
    double m_lastFactor = 1.0;
};
