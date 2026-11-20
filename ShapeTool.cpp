#include "ShapeTool.h"
#include <QPainter>
#include <QTransform>
#include <QLinearGradient>
#include <QtMath>

OutlineMode ShapeTool::outline = OutlineMode::Solid;
FillMode ShapeTool::fill = FillMode::None;

QString ShapeTool::name(ShapeType t) {
    static const char *names[] = {
        "Линия", "Кривая", "Овал", "Прямоугольник", "Скругленный прямоугольник",
        "Многоугольник", "Треугольник", "Прямоугольный треугольник", "Ромб",
        "Пятиугольник", "Шестиугольник", "Стрелка вправо", "Стрелка влево",
        "Стрелка вверх", "Стрелка вниз", "Четырехконечная звезда",
        "Пятиконечная звезда", "Шестиконечная звезда",
        "Скругленная прямоугольная выноска", "Овальная выноска", "Выноска-облако",
        "Сердце", "Молния"};
    return QString::fromUtf8(names[int(t)]);
}

// --------------------------------------------------------------- геометрия

// Многоугольник в координатах 0..100
static QPainterPath poly100(std::initializer_list<QPointF> pts) {
    QPainterPath p;
    p.addPolygon(QPolygonF(QVector<QPointF>(pts)));
    p.closeSubpath();
    return p;
}

static QPainterPath star100(int n, double inner) {
    QPolygonF poly;
    for (int i = 0; i < 2 * n; ++i) {
        double a = -M_PI / 2 + i * M_PI / n;
        double r = (i % 2 == 0) ? 50 : 50 * inner;
        poly << QPointF(50 + r * std::cos(a), 50 + r * std::sin(a));
    }
    QPainterPath p;
    p.addPolygon(poly);
    p.closeSubpath();
    return p;
}

static QPainterPath regular100(int n) {
    QPolygonF poly;
    for (int i = 0; i < n; ++i) {
        double a = -M_PI / 2 + i * 2 * M_PI / n;
        poly << QPointF(50 + 50 * std::cos(a), 50 + 50 * std::sin(a));
    }
    QPainterPath p;
    p.addPolygon(poly);
    p.closeSubpath();
    return p;
}

QPainterPath ShapeTool::shapePath(ShapeType t, const QRectF &r) {
    QPainterPath p;   // строим в квадрате 100x100, потом растягиваем в r
    const QPainterPath arrowRight = poly100({{0, 25}, {55, 25}, {55, 0}, {100, 50}, {55, 100}, {55, 75}, {0, 75}});

    switch (t) {
    case ShapeType::Line:        p.moveTo(0, 100); p.lineTo(100, 0); break;
    case ShapeType::Curve:       p.moveTo(0, 90); p.cubicTo(20, -40, 80, 140, 100, 10); break;
    case ShapeType::Ellipse:     p.addEllipse(0, 0, 100, 100); break;
    case ShapeType::Rectangle:   p.addRect(0, 0, 100, 100); break;
    case ShapeType::RoundedRect: p.addRoundedRect(0, 0, 100, 100, 18, 18); break;
    case ShapeType::Polygon:     p = poly100({{0, 100}, {15, 15}, {60, 40}, {100, 0}, {100, 100}}); break;
    case ShapeType::Triangle:    p = poly100({{50, 0}, {100, 100}, {0, 100}}); break;
    case ShapeType::RightTriangle: p = poly100({{0, 0}, {100, 100}, {0, 100}}); break;
    case ShapeType::Diamond:     p = poly100({{50, 0}, {100, 50}, {50, 100}, {0, 50}}); break;
    case ShapeType::Pentagon:    p = regular100(5); break;
    case ShapeType::Hexagon:     p = regular100(6); break;
    case ShapeType::ArrowRight:  p = arrowRight; break;
    case ShapeType::ArrowLeft:   p = QTransform(-1, 0, 0, 1, 100, 0).map(arrowRight); break;
    case ShapeType::ArrowUp:     p = QTransform(0, -1, 1, 0, 0, 100).map(arrowRight); break;
    case ShapeType::ArrowDown:   p = QTransform(0, 1, 1, 0, 0, 0).map(arrowRight); break;
    case ShapeType::Star4:       p = star100(4, 0.38); break;
    case ShapeType::Star5:       p = star100(5, 0.4); break;
    case ShapeType::Star6:       p = star100(6, 0.58); break;
    case ShapeType::CalloutRect: {
        QPainterPath body;
        body.addRoundedRect(0, 0, 100, 72, 12, 12);
        p = body.united(poly100({{18, 65}, {38, 65}, {15, 100}}));
        break;
    }
    case ShapeType::CalloutOval: {
        QPainterPath body;
        body.addEllipse(0, 0, 100, 76);
        p = body.united(poly100({{18, 58}, {38, 70}, {12, 100}}));
        break;
    }
    case ShapeType::CalloutCloud: {
        QPainterPath cloud;
        const QRectF puffs[] = {{20, 16, 60, 46}, {8, 18, 34, 34}, {28, 4, 34, 32}, {54, 6, 36, 34},
                                {64, 26, 36, 34}, {44, 42, 34, 30}, {14, 40, 34, 30}};
        for (const auto &e : puffs) {
            QPainterPath c;
            c.addEllipse(e);
            cloud = cloud.united(c);
        }
        cloud.addEllipse(12, 76, 10, 9);   // «хвостик» из пузырьков
        cloud.addEllipse(4, 90, 7, 6);
        p = cloud;
        break;
    }
    case ShapeType::Heart:
        p.moveTo(50, 28);
        p.cubicTo(50, 0, 0, -2, 0, 32);
        p.cubicTo(0, 62, 36, 76, 50, 100);
        p.cubicTo(64, 76, 100, 62, 100, 32);
        p.cubicTo(100, -2, 50, 0, 50, 28);
        p.closeSubpath();
        break;
    case ShapeType::Lightning:
        p = poly100({{38, 0}, {62, 26}, {52, 31}, {78, 56}, {68, 61}, {100, 100},
                     {46, 70}, {57, 64}, {22, 42}, {34, 36}, {0, 12}});
        break;
    }
    QTransform tr;
    tr.translate(r.x(), r.y());
    tr.scale(r.width() / 100.0, r.height() / 100.0);
    return tr.map(p);
}

// --------------------------------------------------------------- рисование

void ShapeTool::drawPath(QImage &image, const QPainterPath &path, bool closed) {
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    if (closed && fill != FillMode::None) {
        QColor c = m_secondary;
        if (fill == FillMode::Marker) c.setAlphaF(0.5);
        if (fill == FillMode::Watercolor) c.setAlphaF(0.3);
        if (fill == FillMode::Gradient) {
            // градиент от цвета 1 к цвету 2 (бывший инструмент «Градиентная заливка»)
            QRectF b = path.boundingRect();
            QLinearGradient g(b.topLeft(), b.bottomRight());
            g.setColorAt(0, m_color);
            g.setColorAt(1, m_secondary);
            painter.fillPath(path, g);
        } else {
            painter.fillPath(path, c);
        }
    }
    if (outline != OutlineMode::None || !closed) {
        QColor c = m_color;
        if (outline == OutlineMode::Marker) c.setAlphaF(0.5);
        if (outline == OutlineMode::Watercolor) c.setAlphaF(0.3);
        // у фигур с изгибами (сердце, облако, овал) острый «митер» дал бы шип в точке излома
        const bool curved = m_type == ShapeType::Heart || m_type == ShapeType::CalloutCloud ||
                            m_type == ShapeType::Ellipse || m_type == ShapeType::CalloutOval;
        QPen pen(c, m_size, Qt::SolidLine, closed ? Qt::SquareCap : Qt::RoundCap,
                 (closed && !curved) ? Qt::MiterJoin : Qt::RoundJoin);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(path);
    }
}

void ShapeTool::drawCurve(QImage &image) {
    QPainterPath path(m_p0);
    path.cubicTo(m_c1, m_c2, m_p3);
    drawPath(image, path, false);
}

void ShapeTool::drawPolygon(QImage &image, bool closed) {
    if (m_points.isEmpty()) return;
    QPainterPath path;
    path.addPolygon(QPolygonF(QPolygon(m_points)));
    if (closed) path.closeSubpath();
    drawPath(image, path, closed);
}

bool ShapeTool::busy() const {
    if (m_type == ShapeType::Curve) return m_curveStage != 0;
    if (m_type == ShapeType::Polygon) return m_polyActive;
    return false;
}

void ShapeTool::mousePress(QImage &image, const QPoint &pos, const QColor &color) {
    if (m_type == ShapeType::Curve) {
        if (m_curveStage == 0) {
            m_color = color;
            m_p0 = m_p3 = m_c1 = m_c2 = pos;
        } else if (m_curveStage == 1) {
            m_c1 = m_c2 = pos;
        } else {
            m_c2 = pos;
        }
        drawCurve(image);
        return;
    }
    if (m_type == ShapeType::Polygon) {
        if (!m_polyActive) {
            m_color = color;
            m_points = {pos, pos};
        } else {
            m_points.append(pos);
        }
        drawPolygon(image, false);
        return;
    }
    m_color = color;
}

void ShapeTool::mouseMove(QImage &image, const QPoint &from, const QPoint &to, const QColor &) {
    preview(image, from, to);
}

void ShapeTool::preview(QImage &image, const QPoint &from, const QPoint &to) {
    if (m_type == ShapeType::Curve) {
        if (m_curveStage == 0) m_p3 = to;
        else if (m_curveStage == 1) m_c1 = m_c2 = to;
        else m_c2 = to;
        drawCurve(image);
        return;
    }
    if (m_type == ShapeType::Polygon) {
        if (!m_points.isEmpty()) m_points.last() = to;
        drawPolygon(image, false);
        return;
    }
    if (m_type == ShapeType::Line) {
        QPainterPath path(from);
        path.lineTo(to);
        drawPath(image, path, false);
        return;
    }
    QRectF rect = QRectF(from, to).normalized();
    if (rect.width() < 1 && rect.height() < 1) return;
    drawPath(image, shapePath(m_type, rect), true);
}

void ShapeTool::mouseRelease(QImage &image, const QPoint &from, const QPoint &to, const QColor &) {
    if (m_type == ShapeType::Polygon && m_polyActive && m_points.size() > 3 &&
        QLineF(to, m_points.first()).length() <= 8) {
        // щелчок рядом с первой вершиной — замыкаем многоугольник
        m_points.removeLast();
        drawPolygon(image, true);
        m_polyActive = false;
        m_points.clear();
        return;
    }
    preview(image, from, to);

    if (m_type == ShapeType::Curve) {
        // 1-й шаг — линия, 2-й и 3-й — изгибы
        if (m_curveStage == 0 && from == to) return;
        m_curveStage = (m_curveStage + 1) % 3;
    } else if (m_type == ShapeType::Polygon && !m_polyActive) {
        if (from != to) m_polyActive = true;
        else m_points.clear();
    }
}

// Принудительное завершение (смена инструмента, двойной щелчок)
void ShapeTool::finish(QImage &image) {
    if (m_type == ShapeType::Curve && m_curveStage != 0) {
        drawCurve(image);
        m_curveStage = 0;
    } else if (m_type == ShapeType::Polygon && m_polyActive) {
        drawPolygon(image, true);
        m_polyActive = false;
        m_points.clear();
    }
}

void ShapeTool::doubleClick(QImage &image) {
    if (m_type == ShapeType::Polygon && m_polyActive) finish(image);
}
