#pragma once
#include "Tool.h"
#include <QPainterPath>

// Все кисти как в Paint
enum class BrushStyle {
    Brush,          // Кисть
    Calligraphy1,   // Каллиграфическая кисть 1
    Calligraphy2,   // Каллиграфическая кисть 2
    Airbrush,       // Баллончик
    Oil,            // Масляная кисть
    Crayon,         // Пастель
    Marker,         // Маркер
    NaturalPencil,  // Обычный карандаш
    Watercolor      // Кисть для акварели
};

class BrushTool : public Tool {
public:
    explicit BrushTool(BrushStyle style = BrushStyle::Brush) : m_style(style) {}
    void mousePress(QImage &image, const QPoint &pos, const QColor &color) override;
    void mouseMove(QImage &image, const QPoint &from, const QPoint &to, const QColor &color) override;
    void mouseRelease(QImage &, const QPoint &, const QPoint &, const QColor &) override {}

    static QString name(BrushStyle style);

private:
    void stamp(QImage &image, const QPointF &p, const QColor &color);
    void segment(QImage &image, const QPointF &a, const QPointF &b, const QColor &color);
    void redrawStroke(QImage &image, const QColor &color);

    BrushStyle m_style;
    QImage m_backup;        // для полупрозрачных кистей (маркер, акварель)
    QPainterPath m_path;
};
