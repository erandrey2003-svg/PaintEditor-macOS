#include "BrushTool.h"
#include <QPainter>
#include <QRandomGenerator>
#include <QtMath>

QString BrushTool::name(BrushStyle s) {
    switch (s) {
    case BrushStyle::Brush:         return "Кисть";
    case BrushStyle::Calligraphy1:  return "Каллиграфическая кисть 1";
    case BrushStyle::Calligraphy2:  return "Каллиграфическая кисть 2";
    case BrushStyle::Airbrush:      return "Баллончик";
    case BrushStyle::Oil:           return "Масляная кисть";
    case BrushStyle::Crayon:        return "Пастель";
    case BrushStyle::Marker:        return "Маркер";
    case BrushStyle::NaturalPencil: return "Обычный карандаш";
    case BrushStyle::Watercolor:    return "Кисть для акварели";
    }
    return {};
}

static double rnd() { return QRandomGenerator::global()->generateDouble(); }

// Один «отпечаток» кисти в точке p
void BrushTool::stamp(QImage &image, const QPointF &p, const QColor &color) {
    QPainter g(&image);
    g.setRenderHint(QPainter::Antialiasing);
    const double s = m_size;

    switch (m_style) {
    case BrushStyle::Brush:
        g.setPen(Qt::NoPen);
        g.setBrush(color);
        g.drawEllipse(p, s * 1.5 + 1, s * 1.5 + 1);
        break;
    case BrushStyle::Calligraphy1:
    case BrushStyle::Calligraphy2: {
        const double d = s * 2 + 2;
        const double k = (m_style == BrushStyle::Calligraphy1) ? 1 : -1;
        g.setPen(QPen(color, 1.5));
        g.drawLine(QPointF(p.x() - d * 0.7, p.y() + k * d * 0.7), QPointF(p.x() + d * 0.7, p.y() - k * d * 0.7));
        break;
    }
    case BrushStyle::Airbrush: {
        const double r = s * 4 + 6;
        g.setRenderHint(QPainter::Antialiasing, false);
        g.setPen(color);
        for (int i = 0; i < int(r * 1.5); ++i) {
            double a = rnd() * 2 * M_PI, rr = r * std::sqrt(rnd());
            g.drawPoint(QPointF(p.x() + rr * std::cos(a), p.y() + rr * std::sin(a)));
        }
        break;
    }
    case BrushStyle::Oil: {
        // много «щетинок» разной плотности
        const double r = s * 2 + 4;
        g.setPen(Qt::NoPen);
        for (int i = -int(r); i <= int(r); i += 2) {
            QColor c = color;
            c.setAlphaF(0.35 + 0.65 * std::fabs(std::sin(i * 1.7)));
            g.setBrush(c);
            g.drawEllipse(QPointF(p.x() + i * 0.3, p.y() + i), 1.6, 1.6);
        }
        break;
    }
    case BrushStyle::Crayon: {
        const double r = s * 2 + 3;
        g.setRenderHint(QPainter::Antialiasing, false);
        g.setPen(color);
        for (int i = 0; i < int(r * r * 0.6); ++i) {
            double dx = (rnd() * 2 - 1) * r, dy = (rnd() * 2 - 1) * r;
            if (dx * dx + dy * dy <= r * r && rnd() < 0.55)
                g.drawPoint(QPointF(p.x() + dx, p.y() + dy));
        }
        break;
    }
    case BrushStyle::NaturalPencil: {
        QColor c = color;
        c.setAlphaF(0.55);
        g.setPen(Qt::NoPen);
        g.setBrush(c);
        for (int i = 0; i < 3; ++i)
            g.drawEllipse(QPointF(p.x() + (rnd() - 0.5) * s, p.y() + (rnd() - 0.5) * s), s * 0.5 + 0.5, s * 0.5 + 0.5);
        break;
    }
    case BrushStyle::Marker:
    case BrushStyle::Watercolor:
        break; // рисуются целым штрихом (redrawStroke)
    }
}

// Штрих от a до b: ставим отпечатки с небольшим шагом
void BrushTool::segment(QImage &image, const QPointF &a, const QPointF &b, const QColor &color) {
    if (m_style == BrushStyle::Brush) {
        QPainter g(&image);
        g.setRenderHint(QPainter::Antialiasing);
        g.setPen(QPen(color, (m_size * 1.5 + 1) * 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        g.drawLine(a, b);
        return;
    }
    if (m_style == BrushStyle::Calligraphy1 || m_style == BrushStyle::Calligraphy2) {
        // заполняем параллелограмм между двумя положениями пера
        const double d = (m_size * 2 + 2) * 0.7;
        const double k = (m_style == BrushStyle::Calligraphy1) ? 1 : -1;
        QPointF off(d, -k * d);
        QPainter g(&image);
        g.setRenderHint(QPainter::Antialiasing);
        g.setPen(QPen(color, 1));
        g.setBrush(color);
        g.drawPolygon(QPolygonF({a - off, a + off, b + off, b - off}));
        return;
    }
    const double len = QLineF(a, b).length();
    const double step = (m_style == BrushStyle::Airbrush) ? 6.0 : 2.0;
    const int n = std::max(1, int(len / step));
    for (int i = 1; i <= n; ++i)
        stamp(image, a + (b - a) * (double(i) / n), color);
}

// Маркер и акварель: весь штрих рисуется заново полупрозрачным цветом,
// чтобы перекрытия внутри одного штриха не темнели
void BrushTool::redrawStroke(QImage &image, const QColor &color) {
    image = m_backup;
    QPainter g(&image);
    g.setRenderHint(QPainter::Antialiasing);
    QColor c = color;
    if (m_style == BrushStyle::Marker) {
        c.setAlphaF(0.5);
        g.setPen(QPen(c, m_size * 2 + 6, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin));
    } else {
        c.setAlphaF(0.3);
        g.setPen(QPen(c, m_size * 3 + 8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    }
    g.drawPath(m_path);
    if (m_path.elementCount() == 1) g.drawPoint(m_path.currentPosition());
}

void BrushTool::mousePress(QImage &image, const QPoint &pos, const QColor &color) {
    if (m_style == BrushStyle::Marker || m_style == BrushStyle::Watercolor) {
        m_backup = image;
        m_path = QPainterPath(pos);
        redrawStroke(image, color);
        return;
    }
    if (m_style == BrushStyle::Brush || m_style == BrushStyle::Calligraphy1 || m_style == BrushStyle::Calligraphy2)
        segment(image, pos, pos, color);
    else
        stamp(image, pos, color);
}

void BrushTool::mouseMove(QImage &image, const QPoint &from, const QPoint &to, const QColor &color) {
    if (m_style == BrushStyle::Marker || m_style == BrushStyle::Watercolor) {
        m_path.lineTo(to);
        redrawStroke(image, color);
        return;
    }
    segment(image, from, to, color);
}
