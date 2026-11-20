#pragma once
#include "Tool.h"

// Карандаш: жёсткая линия без сглаживания
class PencilTool : public Tool {
public:
    void mousePress(QImage &image, const QPoint &pos, const QColor &color) override;
    void mouseMove(QImage &image, const QPoint &from, const QPoint &to, const QColor &color) override;
    void mouseRelease(QImage &, const QPoint &, const QPoint &, const QColor &) override {}
};
