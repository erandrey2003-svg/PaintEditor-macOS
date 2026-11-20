#pragma once
#include <QWidget>
#include <QPainter>
#include <cmath>

// Линейка (Вид → Линейки): деления в пикселях изображения
class Ruler : public QWidget {
public:
    explicit Ruler(Qt::Orientation o, QWidget *parent = nullptr) : QWidget(parent), m_o(o) {
        if (o == Qt::Horizontal) setFixedHeight(20); else setFixedWidth(20);
    }
    void setZoom(double z)    { m_zoom = z; update(); }
    void setOffset(int off)   { m_offset = off; update(); }
    void setCursorPos(int p)  { m_cursor = p; update(); }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.fillRect(rect(), QColor("#f5f6f7"));
        p.setPen(QColor("#a0a0a0"));
        const bool h = m_o == Qt::Horizontal;
        const int len = h ? width() : height();
        if (h) p.drawLine(0, height() - 1, width(), height() - 1);
        else   p.drawLine(width() - 1, 0, width() - 1, height());

        // шаг подписи: 10, 20, 50, 100, 200, 500 ... так, чтобы было не теснее 50 px экрана
        const double steps[] = {1, 2, 5, 10, 20, 50, 100, 200, 500, 1000, 2000};
        double major = 1000;
        for (double s : steps) if (s * m_zoom >= 50) { major = s; break; }
        const double minor = major / 10.0;

        QFont f = font();
        f.setPixelSize(9);
        p.setFont(f);
        p.setPen(QColor("#606060"));
        const int first = int(std::floor(m_offset / m_zoom / minor));
        for (int i = first;; ++i) {
            const double v = i * minor;
            const int s = int(std::round(v * m_zoom)) - m_offset;
            if (s > len) break;
            if (s < 0) continue;
            const bool isMajor = std::fmod(v, major) == 0;
            const int t = isMajor ? 10 : (std::fmod(v, major / 2) == 0 ? 6 : 3);
            if (h) {
                p.drawLine(s, 20 - t, s, 19);
                if (isMajor) p.drawText(s + 2, 9, QString::number(int(v)));
            } else {
                p.drawLine(20 - t, s, 19, s);
                if (isMajor) {
                    p.save();
                    p.translate(9, s + 2);
                    p.rotate(90);
                    p.drawText(0, 0, QString::number(int(v)));
                    p.restore();
                }
            }
        }
        if (m_cursor >= 0) {               // положение курсора
            const int s = int(std::round(m_cursor * m_zoom)) - m_offset;
            p.setPen(QColor("#1979ca"));
            if (h) p.drawLine(s, 0, s, 19); else p.drawLine(0, s, 19, s);
        }
    }

private:
    Qt::Orientation m_o;
    double m_zoom = 1.0;
    int m_offset = 0;
    int m_cursor = -1;
};
