#include "EraserTool.h"
#include <QPainter>

// Ластик стирает вторым цветом (цветом фона), как в Paint
void EraserTool::mousePress(QImage &image, const QPoint &pos, const QColor &color) {
    mouseMove(image, pos, pos, color);
}
void EraserTool::mouseMove(QImage &image, const QPoint &from, const QPoint &to, const QColor &) {
    QPainter painter(&image);
    const int s = m_size * 2 + 2;
    painter.setPen(QPen(m_secondary, s, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin));
    if (from == to)
        painter.fillRect(QRect(to.x() - s / 2, to.y() - s / 2, s, s), m_secondary);
    else
        painter.drawLine(from, to);
}
