#include "PencilTool.h"
#include <QPainter>

void PencilTool::mousePress(QImage &image, const QPoint &pos, const QColor &color) {
    mouseMove(image, pos, pos, color);
}

void PencilTool::mouseMove(QImage &image, const QPoint &from, const QPoint &to, const QColor &color) {
    QPainter p(&image);
    p.setRenderHint(QPainter::Antialiasing, false);
    p.setPen(QPen(color, m_size, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
    if (from == to) {
        // drawLine с нулевой длиной ничего не рисует — ставим квадрат
        int s = m_size;
        p.fillRect(QRect(to.x() - (s - 1) / 2, to.y() - (s - 1) / 2, s, s), color);
    } else {
        p.drawLine(from, to);
    }
}
