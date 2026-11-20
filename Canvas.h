#pragma once
#include <QWidget>
#include <QImage>
#include <QPoint>
#include <QColor>
#include <QFont>
#include <QVector>
#include <memory>
#include "Tool.h"

class QPlainTextEdit;

// Режим работы холста
enum class CanvasMode {
    Draw,       // рисование текущим инструментом (Tool)
    Select,     // «Выделить» — прямоугольная область
    Text,       // «A» — надпись
    Picker,     // «Палитра» (пипетка)
    Magnifier   // «Масштаб» (лупа)
};

class Canvas : public QWidget {
    Q_OBJECT

signals:
    void mouseMoved(int x, int y);           // координаты курсора на изображении
    void mouseLeft();                        // курсор ушёл с холста
    void colorPicked(const QColor &color, bool primary);
    void zoomRequested(int direction);       // лупа: +1 / -1
    void imageSizeChanged(const QSize &size);
    void selectionChanged(const QRect &rect);
    void imageChanged();                     // любое изменение изображения
    void textEditingChanged(bool active);    // появилась / исчезла надпись
    void historyChanged(bool canUndo, bool canRedo);

public:
    explicit Canvas(QWidget *parent = nullptr);

    // инструменты и режимы
    void setTool(std::unique_ptr<Tool> newTool);
    void setMode(CanvasMode mode);
    CanvasMode mode() const { return m_mode; }
    void setToolSize(int size);

    // цвета
    void setPrimaryColor(const QColor &color);
    void setSecondaryColor(const QColor &color);
    QColor primaryColor() const { return m_primaryColor; }
    QColor secondaryColor() const { return m_secondaryColor; }

    // масштаб и вид
    void setZoom(float zoom);
    float zoom() const { return m_zoom; }
    void setShowGrid(bool show);

    // изображение
    const QImage &image() const { return m_image; }
    void newImage(const QSize &size);
    void loadImage(const QImage &image);
    bool saveImage(const QString &fileName);
    void clear();
    void resizeCanvas(const QSize &size);                      // размер листа (без растяжения)
    void scaleAndSkew(const QSize &size, double skewX, double skewY); // «Изменить размер»
    void rotate(int degrees);
    void flip(bool horizontal);
    void toBlackAndWhite();

    // выделение и буфер обмена
    bool hasSelection() const { return !m_selRect.isNull(); }
    QRect selectionRect() const { return m_selRect; }
    void selectAll();
    void deleteSelection();
    void copySelection();
    void cutSelection();
    void paste(const QImage &img);
    void crop();
    void setTransparentSelection(bool on) { m_transparentSel = on; }

    // текст
    void setTextFont(const QFont &font);   // размер шрифта — в пунктах
    QFont textFont() const { return m_textFont; }
    void setTextOpaque(bool opaque);
    bool isEditingText() const { return m_textEdit != nullptr; }

    // история
    void undo();
    void redo();
    bool canUndo() const { return !m_undo.isEmpty(); }
    bool canRedo() const { return !m_redo.isEmpty(); }

    // завершить всё «плавающее» (надпись, перемещаемое выделение, многоугольник)
    void commitAll();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QPoint toImage(const QPointF &widgetPos) const;
    QRect toWidget(const QRect &imageRect) const;
    void pushUndo();
    void emitHistory();
    void setImageInternal(const QImage &img);
    void finishTool();
    void liftSelection();
    void commitFloating();
    void clearSelection();
    void makeTransparent(QImage &img) const;
    void startText(const QRect &rect);
    void commitText();
    void updateTextEditStyle();
    int textPixelSize(float zoom) const;
    void updateCursor(const QPoint &imagePos);

    QImage m_image;
    QImage m_backup;                      // копия до начала штриха (для предпросмотра)
    std::unique_ptr<Tool> m_currentTool;
    CanvasMode m_mode = CanvasMode::Draw;
    QColor m_primaryColor{Qt::black};
    QColor m_secondaryColor{Qt::white};
    QColor m_strokeColor;
    Qt::MouseButton m_button = Qt::NoButton;
    bool m_drawing{false};
    QPoint m_startPoint;
    QPoint m_lastPoint;
    float m_zoom = 1.0f;
    bool m_showGrid = false;

    // выделение
    QRect m_selRect;
    QImage m_floating;                    // «поднятый» фрагмент, который можно двигать
    QPoint m_moveOffset;
    bool m_selecting = false;
    bool m_moving = false;
    bool m_transparentSel = false;

    // текст
    QPlainTextEdit *m_textEdit = nullptr;
    QRect m_textRect;
    QFont m_textFont;
    bool m_textOpaque = false;

    QVector<QImage> m_undo, m_redo;
};
