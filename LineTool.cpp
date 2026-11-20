#include "LineTool.h"
#include <QPainter>

static void drawLine(QImage &image, const QPoint &from, const QPoint &to, const QColor &color, int size) {
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(color, size, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(from, to);
}

void LineTool::mousePress(QImage &image, const QPoint &pos, const QColor &color) {
    drawLine(image, pos, pos, color, m_size);
}
void LineTool::mouseMove(QImage &image, const QPoint &from, const QPoint &to, const QColor &color) {
    drawLine(image, from, to, color, m_size);   // предпросмотр
}
void LineTool::mouseRelease(QImage &image, const QPoint &from, const QPoint &to, const QColor &color) {
    drawLine(image, from, to, color, m_size);
}
