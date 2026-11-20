#pragma once
#include "Tool.h"

class LineTool : public Tool {
public:
    void mousePress(QImage &image, const QPoint &pos, const QColor &color) override;
    void mouseMove(QImage &image, const QPoint &from, const QPoint &to, const QColor &color) override;
    void mouseRelease(QImage &image, const QPoint &from, const QPoint &to, const QColor &color) override;
    bool wantsPreview() const override { return true; }
};
