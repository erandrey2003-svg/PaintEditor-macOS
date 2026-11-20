#include "FillTool.h"
#include <QVector>

// Построчная заливка (scanline flood fill) без рекурсии
void FillTool::mousePress(QImage &image, const QPoint &pos, const QColor &color) {
    if (!image.rect().contains(pos)) return;
    if (image.format() != QImage::Format_ARGB32)
        image = image.convertToFormat(QImage::Format_ARGB32);

    const int w = image.width(), h = image.height();
    const QRgb target = image.pixel(pos);
    const QRgb repl = color.rgba();
    if (target == repl) return;

    auto px = [&](int x, int y) -> QRgb & {
        return reinterpret_cast<QRgb *>(image.scanLine(y))[x];
    };

    QVector<QPoint> stack;
    stack.push_back(pos);
    while (!stack.isEmpty()) {
        QPoint p = stack.takeLast();
        int x = p.x(), y = p.y();
        if (px(x, y) != target) continue;
        while (x > 0 && px(x - 1, y) == target) --x;   // к левому краю
        bool up = false, down = false;
        for (; x < w && px(x, y) == target; ++x) {
            px(x, y) = repl;
            if (y > 0) {
                bool t = px(x, y - 1) == target;
                if (t && !up) stack.push_back(QPoint(x, y - 1));
                up = t;
            }
            if (y < h - 1) {
                bool t = px(x, y + 1) == target;
                if (t && !down) stack.push_back(QPoint(x, y + 1));
                down = t;
            }
        }
    }
}
