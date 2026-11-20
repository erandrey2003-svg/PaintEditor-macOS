#pragma once
#include <QIcon>
#include <QPixmap>
#include "ShapeTool.h"
#include "BrushTool.h"

// Все иконки рисуются кодом (QPainter), чтобы проект не зависел от файлов картинок
namespace Icons {
QIcon pencil();
QIcon bucket();
QIcon text();
QIcon eraser();
QIcon picker();
QIcon magnifier();

QIcon paste();
QIcon cut();
QIcon copy();
QIcon select();
QIcon crop();
QIcon resize();
QIcon rotate();
QIcon rotateRight();
QIcon rotateLeft();
QIcon rotate180();
QIcon flipVertical();
QIcon flipHorizontal();

QIcon zoomIn();
QIcon zoomOut();
QIcon zoom100();
QIcon fullScreen();
QIcon thumbnail();

QIcon fileNew();
QIcon fileOpen();
QIcon fileSave();
QIcon fileSaveAs();
QIcon print();
QIcon scanner();
QIcon email();
QIcon desktop();
QIcon properties();
QIcon about();
QIcon exit();
QIcon undo();
QIcon redo();
QIcon paint();

QIcon outline();
QIcon fill();
QIcon thickness();
QIcon colorEdit();
QIcon textOpaque();
QIcon textTransparent();
QIcon galleryMore();

QIcon shape(ShapeType type);
QIcon brush(BrushStyle style);
QPixmap thicknessLine(int width, int px, int length);
}
