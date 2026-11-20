#pragma once
#include <QToolButton>

// Кнопка ленты в стиле Paint (Windows 7):
//  Large       — большая иконка, подпись под ней, при необходимости стрелочка ▾ ниже
//  LargeBottom — нижняя половина «разделённой» кнопки: только подпись и ▾ (Кисти, Вставить, Выделить)
//  Small       — маленькая иконка, подпись справа, стрелочка ▾ справа (Контур, Повернуть...)
class RibbonButton : public QToolButton {
public:
    enum Kind { Large, LargeBottom, Small };

    RibbonButton(Kind kind, const QString &text, const QIcon &icon = QIcon(),
                 bool arrow = false, QWidget *parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override { return sizeHint(); }

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    Kind m_kind;
    bool m_arrow;
};
