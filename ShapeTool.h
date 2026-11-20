#pragma once
#include "Tool.h"
#include <QPainterPath>
#include <QVector>

// Все фигуры из галереи Paint (в том же порядке)
enum class ShapeType {
    Line,             // Линия
    Curve,            // Кривая
    Ellipse,          // Овал
    Rectangle,        // Прямоугольник
    RoundedRect,      // Скругленный прямоугольник
    Polygon,          // Многоугольник
    Triangle,         // Треугольник
    RightTriangle,    // Прямоугольный треугольник
    Diamond,          // Ромб
    Pentagon,         // Пятиугольник
    Hexagon,          // Шестиугольник
    ArrowRight,       // Стрелка вправо
    ArrowLeft,        // Стрелка влево
    ArrowUp,          // Стрелка вверх
    ArrowDown,        // Стрелка вниз
    Star4,            // Четырехконечная звезда
    Star5,            // Пятиконечная звезда
    Star6,            // Шестиконечная звезда
    CalloutRect,      // Скругленная прямоугольная выноска
    CalloutOval,      // Овальная выноска
    CalloutCloud,     // Выноска-облако
    Heart,            // Сердце
    Lightning         // Молния
};

enum class OutlineMode { None, Solid, Marker, Watercolor };   // «Контур»
enum class FillMode    { None, Solid, Marker, Watercolor, Gradient };   // «Заливка»

class ShapeTool : public Tool {
public:
    explicit ShapeTool(ShapeType type) : m_type(type) {}

    void mousePress(QImage &image, const QPoint &pos, const QColor &color) override;
    void mouseMove(QImage &image, const QPoint &from, const QPoint &to, const QColor &color) override;
    void mouseRelease(QImage &image, const QPoint &from, const QPoint &to, const QColor &color) override;
    bool wantsPreview() const override { return true; }
    bool busy() const override;
    void finish(QImage &image) override;
    void doubleClick(QImage &image) override;

    // Настройки «Контур» и «Заливка» общие для всех фигур
    static OutlineMode outline;
    static FillMode fill;

    static QString name(ShapeType type);
    static constexpr int count = 23;
    // Контур фигуры, вписанной в прямоугольник r (используется и для иконок)
    static QPainterPath shapePath(ShapeType type, const QRectF &r);

private:
    void preview(QImage &image, const QPoint &from, const QPoint &to);
    void drawPath(QImage &image, const QPainterPath &path, bool closed);
    void drawCurve(QImage &image);
    void drawPolygon(QImage &image, bool closed);

    ShapeType m_type;
    QColor m_color;
    // Кривая: 0 — нет, 1 — линия задана, 2 — первый изгиб задан
    int m_curveStage = 0;
    QPointF m_p0, m_p3, m_c1, m_c2;
    // Многоугольник
    QVector<QPoint> m_points;
    bool m_polyActive = false;
};
