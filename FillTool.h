#pragma once
#include "Tool.h"

// Заливка цветом (flood fill) замкнутой области
class FillTool : public Tool {
public:
    void mousePress(QImage &image, const QPoint &pos, const QColor &color) override;
    void mouseMove(QImage &, const QPoint &, const QPoint &, const QColor &) override {}
    void mouseRelease(QImage &, const QPoint &, const QPoint &, const QColor &) override {}
};
