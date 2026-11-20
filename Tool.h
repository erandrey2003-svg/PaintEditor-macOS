#pragma once
#include <QImage>
#include <QPoint>
#include <QColor>

// Базовый класс инструмента рисования.
// Canvas вызывает mousePress / mouseMove / mouseRelease.
//  * Для обычных инструментов (кисть, ластик) mouseMove получает (предыдущая точка, текущая точка).
//  * Для инструментов с предпросмотром (wantsPreview() == true — линии, фигуры)
//    Canvas перед каждым вызовом восстанавливает картинку из резервной копии,
//    а mouseMove/mouseRelease получают (начальная точка, текущая точка).
class Tool {
protected:
    int m_size = 3;                     // толщина
    QColor m_secondary = Qt::white;     // второй цвет (заливка фигур / ластик)

public:
    virtual ~Tool() = default;

    void setSize(int size) { m_size = size; }
    int size() const { return m_size; }
    void setSecondary(const QColor &c) { m_secondary = c; }

    virtual void mousePress(QImage &image, const QPoint &pos, const QColor &color) = 0;
    virtual void mouseMove(QImage &image, const QPoint &from, const QPoint &to, const QColor &color) = 0;
    virtual void mouseRelease(QImage &image, const QPoint &from, const QPoint &to, const QColor &color) = 0;

    // Предпросмотр: Canvas восстанавливает изображение перед каждым шагом
    virtual bool wantsPreview() const { return false; }
    // Многошаговые инструменты (кривая, многоугольник) — ещё не закончены
    virtual bool busy() const { return false; }
    // Принудительно завершить многошаговое рисование (смена инструмента)
    virtual void finish(QImage &) {}
    // Двойной щелчок (завершение многоугольника)
    virtual void doubleClick(QImage &) {}
};
