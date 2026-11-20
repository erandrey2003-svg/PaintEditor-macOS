#include "Canvas.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPlainTextEdit>
#include <QApplication>
#include <QClipboard>
#include <QTransform>
#include <QtMath>

static const int kUndoLimit = 20;

Canvas::Canvas(QWidget *parent) : QWidget(parent) {
    m_image = QImage(1920, 1080, QImage::Format_ARGB32);
    m_image.fill(Qt::white);
    setFixedSize(m_image.size());
    setMouseTracking(true);          // mouseMove приходит и без нажатия — для координат
    setFocusPolicy(Qt::StrongFocus);
    m_textFont = QFont("Calibri");
    m_textFont.setPointSize(11);
    setCursor(Qt::CrossCursor);
}

// ---------------------------------------------------------------- координаты

QPoint Canvas::toImage(const QPointF &p) const {
    return QPoint(int(std::floor(p.x() / m_zoom)), int(std::floor(p.y() / m_zoom)));
}

QRect Canvas::toWidget(const QRect &r) const {
    return QRect(QPoint(qRound(r.left() * m_zoom), qRound(r.top() * m_zoom)),
                 QPoint(qRound((r.right() + 1) * m_zoom) - 1, qRound((r.bottom() + 1) * m_zoom) - 1));
}

// ---------------------------------------------------------------- отрисовка

void Canvas::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.save();
    painter.scale(m_zoom, m_zoom);
    painter.drawImage(0, 0, m_image);
    if (!m_floating.isNull())
        painter.drawImage(m_selRect.topLeft(), m_floating);
    painter.restore();

    if (m_showGrid) {
        // шаг сетки в пикселях изображения так, чтобы линии не сливались
        int step = std::max(1, int(std::ceil(8.0 / m_zoom)));
        painter.setPen(QPen(QColor(0, 0, 0, 45), 0));
        for (int x = step; x < m_image.width(); x += step)
            painter.drawLine(QPointF(x * m_zoom, 0), QPointF(x * m_zoom, height()));
        for (int y = step; y < m_image.height(); y += step)
            painter.drawLine(QPointF(0, y * m_zoom), QPointF(width(), y * m_zoom));
    }

    // рамка выделения / будущей надписи
    QRect frame = m_selRect;
    if (m_mode == CanvasMode::Text && m_selecting) frame = QRect(m_startPoint, m_lastPoint).normalized();
    if (!frame.isNull()) {
        QRect w = toWidget(frame);
        painter.setPen(QPen(Qt::white, 1));
        painter.drawRect(w);
        QPen dash(QColor(30, 30, 30), 1, Qt::DashLine);
        dash.setDashPattern({3, 3});
        painter.setPen(dash);
        painter.drawRect(w);
    }
}

// ---------------------------------------------------------------- мышь

void Canvas::mousePressEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton && event->button() != Qt::RightButton) return;
    const QPoint pos = toImage(event->position());
    const bool left = event->button() == Qt::LeftButton;
    m_button = event->button();
    m_startPoint = m_lastPoint = pos;

    switch (m_mode) {
    case CanvasMode::Picker:
        if (m_image.rect().contains(pos)) emit colorPicked(m_image.pixelColor(pos), left);
        return;

    case CanvasMode::Magnifier:
        emit zoomRequested(left ? +1 : -1);
        return;

    case CanvasMode::Text:
        if (m_textEdit) { commitText(); return; }   // щелчок вне надписи — завершить её
        if (!left) return;
        m_selecting = true;
        return;

    case CanvasMode::Select:
        if (!left) return;
        if (!m_selRect.isNull() && m_selRect.contains(pos)) {
            liftSelection();                         // начинаем перетаскивать фрагмент
            m_moving = true;
            m_moveOffset = pos - m_selRect.topLeft();
        } else {
            commitFloating();
            clearSelection();
            m_selecting = true;
        }
        update();
        return;

    case CanvasMode::Draw:
        break;
    }

    if (!m_currentTool) return;
    if (!m_currentTool->busy()) {
        pushUndo();
        m_backup = m_image;
        m_strokeColor = left ? m_primaryColor : m_secondaryColor;
        m_currentTool->setSecondary(left ? m_secondaryColor : m_primaryColor);
    }
    if (m_currentTool->wantsPreview()) m_image = m_backup;
    m_drawing = true;
    m_currentTool->mousePress(m_image, pos, m_strokeColor);
    update();
}

void Canvas::mouseMoveEvent(QMouseEvent *event) {
    const QPoint pos = toImage(event->position());
    // координаты в строке состояния обновляются ВСЕГДА, а не только при рисовании
    if (m_image.rect().contains(pos)) emit mouseMoved(pos.x(), pos.y());
    else emit mouseLeft();
    updateCursor(pos);

    if (m_mode == CanvasMode::Select || m_mode == CanvasMode::Text) {
        if (m_moving) {
            m_selRect.moveTopLeft(pos - m_moveOffset);
            emit selectionChanged(m_selRect);
            update();
        } else if (m_selecting) {
            QPoint p(qBound(0, pos.x(), m_image.width() - 1), qBound(0, pos.y(), m_image.height() - 1));
            m_lastPoint = p;
            if (m_mode == CanvasMode::Select) {
                m_selRect = QRect(m_startPoint, p).normalized();
                emit selectionChanged(m_selRect);
            }
            update();
        }
        return;
    }

    if (m_drawing && m_currentTool) {
        if (m_currentTool->wantsPreview()) {
            m_image = m_backup;
            m_currentTool->mouseMove(m_image, m_startPoint, pos, m_strokeColor);
        } else {
            m_currentTool->mouseMove(m_image, m_lastPoint, pos, m_strokeColor);
        }
        m_lastPoint = pos;
        update();
    }
}

void Canvas::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() != m_button) return;
    const QPoint pos = toImage(event->position());

    if (m_mode == CanvasMode::Select) {
        if (m_selecting && (m_selRect.width() < 2 || m_selRect.height() < 2)) clearSelection();
        m_selecting = m_moving = false;
        update();
        return;
    }
    if (m_mode == CanvasMode::Text) {
        if (m_selecting) {
            m_selecting = false;
            QRect r = QRect(m_startPoint, m_lastPoint).normalized();
            if (r.width() < 10 || r.height() < 10)
                r = QRect(m_startPoint, QSize(220, textPixelSize(1.0f) * 2 + 8));
            startText(r.intersected(m_image.rect()));
        }
        update();
        return;
    }

    if (m_drawing && m_currentTool) {
        if (m_currentTool->wantsPreview()) m_image = m_backup;
        m_currentTool->mouseRelease(m_image, m_startPoint, pos, m_strokeColor);
        m_drawing = false;
        update();
        emit imageChanged();
    }
}

void Canvas::mouseDoubleClickEvent(QMouseEvent *event) {
    if (m_mode == CanvasMode::Draw && m_currentTool && m_currentTool->busy()) {
        m_image = m_backup;
        m_currentTool->doubleClick(m_image);
        m_drawing = false;
        update();
        emit imageChanged();
        return;
    }
    mousePressEvent(event);
}

void Canvas::leaveEvent(QEvent *) {
    emit mouseLeft();
}

void Canvas::updateCursor(const QPoint &p) {
    switch (m_mode) {
    case CanvasMode::Select:
        setCursor(!m_selRect.isNull() && m_selRect.contains(p) ? Qt::SizeAllCursor : Qt::CrossCursor);
        break;
    case CanvasMode::Text:      setCursor(Qt::IBeamCursor); break;
    case CanvasMode::Magnifier: setCursor(Qt::PointingHandCursor); break;
    default:                    setCursor(Qt::CrossCursor); break;
    }
}

// ---------------------------------------------------------------- инструменты

void Canvas::finishTool() {
    if (m_currentTool && m_currentTool->busy()) {
        m_image = m_backup;
        m_currentTool->finish(m_image);
        emit imageChanged();
    }
    m_drawing = false;
}

void Canvas::commitAll() {
    finishTool();
    commitText();
    commitFloating();
    if (m_mode != CanvasMode::Select) clearSelection();
    update();
}

void Canvas::setTool(std::unique_ptr<Tool> newTool) {
    commitAll();
    m_currentTool = std::move(newTool);
    m_mode = CanvasMode::Draw;
    clearSelection();
    update();
}

void Canvas::setMode(CanvasMode mode) {
    if (mode == m_mode && mode != CanvasMode::Draw) return;
    finishTool();
    commitText();
    commitFloating();
    clearSelection();
    m_mode = mode;
    updateCursor(QPoint(-1, -1));
    update();
}

void Canvas::setToolSize(int size) {
    if (m_currentTool) m_currentTool->setSize(size);
}

void Canvas::setPrimaryColor(const QColor &color) {
    m_primaryColor = color;
    updateTextEditStyle();
}

void Canvas::setSecondaryColor(const QColor &color) {
    m_secondaryColor = color;
    updateTextEditStyle();
}

void Canvas::setZoom(float zoom) {
    m_zoom = zoom;
    setFixedSize(QSize(qRound(m_image.width() * m_zoom), qRound(m_image.height() * m_zoom)));
    if (m_textEdit) {
        m_textEdit->setGeometry(toWidget(m_textRect));
        updateTextEditStyle();
    }
    update();
}

void Canvas::setShowGrid(bool show) {
    m_showGrid = show;
    update();
}

// ---------------------------------------------------------------- история

void Canvas::pushUndo() {
    m_undo.push_back(m_image);
    if (m_undo.size() > kUndoLimit) m_undo.removeFirst();
    m_redo.clear();
    emitHistory();
}

void Canvas::emitHistory() {
    emit historyChanged(canUndo(), canRedo());
}

void Canvas::setImageInternal(const QImage &img) {
    const bool sizeChanged = img.size() != m_image.size();
    m_image = img.convertToFormat(QImage::Format_ARGB32);
    if (sizeChanged) {
        setZoom(m_zoom);
        emit imageSizeChanged(m_image.size());
    }
    update();
    emit imageChanged();
}

void Canvas::undo() {
    // незавершённые надпись/выделение отбрасываются
    if (m_textEdit) { m_textEdit->deleteLater(); m_textEdit = nullptr; emit textEditingChanged(false); }
    m_floating = QImage();
    clearSelection();
    if (m_currentTool && m_currentTool->busy()) { m_image = m_backup; m_currentTool->finish(m_backup); }
    m_drawing = false;
    if (m_undo.isEmpty()) { update(); return; }
    m_redo.push_back(m_image);
    setImageInternal(m_undo.takeLast());
    emitHistory();
}

void Canvas::redo() {
    if (m_redo.isEmpty()) return;
    commitAll();
    m_undo.push_back(m_image);
    setImageInternal(m_redo.takeLast());
    emitHistory();
}

// ---------------------------------------------------------------- изображение

void Canvas::newImage(const QSize &size) {
    commitAll();
    QImage img(size, QImage::Format_ARGB32);
    img.fill(Qt::white);
    m_undo.clear();
    m_redo.clear();
    setImageInternal(img);
    emitHistory();
}

void Canvas::loadImage(const QImage &image) {
    commitAll();
    m_undo.clear();
    m_redo.clear();
    // прозрачные области открытого файла делаем белыми, как Paint
    QImage img(image.size(), QImage::Format_ARGB32);
    img.fill(Qt::white);
    QPainter(&img).drawImage(0, 0, image);
    setImageInternal(img);
    emitHistory();
}

bool Canvas::saveImage(const QString &fileName) {
    commitAll();
    return m_image.save(fileName);
}

void Canvas::clear() {
    commitAll();
    pushUndo();
    m_image.fill(Qt::white);
    update();
    emit imageChanged();
}

void Canvas::resizeCanvas(const QSize &size) {
    commitAll();
    pushUndo();
    QImage img(size, QImage::Format_ARGB32);
    img.fill(Qt::white);
    QPainter(&img).drawImage(0, 0, m_image);
    setImageInternal(img);
}

// изменение размера (в пикселях) и наклон (в градусах)
static QImage scaleSkew(const QImage &src, const QSize &size, double skewX, double skewY, const QColor &bg) {
    QImage scaled = src.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    if (qFuzzyIsNull(skewX) && qFuzzyIsNull(skewY)) return scaled;
    QTransform t;
    t.shear(std::tan(qDegreesToRadians(skewX)), std::tan(qDegreesToRadians(skewY)));
    QImage sk = scaled.transformed(t, Qt::SmoothTransformation);
    QImage out(sk.size(), QImage::Format_ARGB32);
    out.fill(bg);
    QPainter(&out).drawImage(0, 0, sk);
    return out;
}

void Canvas::scaleAndSkew(const QSize &size, double skewX, double skewY) {
    if (hasSelection()) {           // как в Paint: если есть выделение — меняется только оно
        liftSelection();
        m_floating = scaleSkew(m_floating, size, skewX, skewY, Qt::transparent);
        m_selRect.setSize(m_floating.size());
        emit selectionChanged(m_selRect);
        update();
        return;
    }
    commitAll();
    pushUndo();
    setImageInternal(scaleSkew(m_image, size, skewX, skewY, m_secondaryColor));
}

void Canvas::rotate(int degrees) {
    QTransform t;
    t.rotate(degrees);
    if (hasSelection()) {
        liftSelection();
        const QPoint c = m_selRect.center();
        m_floating = m_floating.transformed(t);
        m_selRect = QRect(QPoint(), m_floating.size());
        m_selRect.moveCenter(c);
        emit selectionChanged(m_selRect);
        update();
        return;
    }
    commitAll();
    pushUndo();
    setImageInternal(m_image.transformed(t));
}

void Canvas::flip(bool horizontal) {
    QTransform t;
    t.scale(horizontal ? -1 : 1, horizontal ? 1 : -1);
    if (hasSelection()) {
        liftSelection();
        m_floating = m_floating.transformed(t);
        update();
        return;
    }
    commitAll();
    pushUndo();
    setImageInternal(m_image.transformed(t));
}

void Canvas::toBlackAndWhite() {
    commitAll();
    pushUndo();
    setImageInternal(m_image.convertToFormat(QImage::Format_Mono, Qt::ThresholdDither));
}

// ---------------------------------------------------------------- выделение

void Canvas::clearSelection() {
    m_selRect = QRect();
    m_selecting = m_moving = false;
    emit selectionChanged(QRect());
}

void Canvas::makeTransparent(QImage &img) const {
    const QRgb key = m_secondaryColor.rgb();
    for (int y = 0; y < img.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x)
            if ((line[x] & 0xffffff) == (key & 0xffffff)) line[x] = 0;
    }
}

// «поднять» выделенный фрагмент с листа, чтобы его можно было двигать
void Canvas::liftSelection() {
    if (!m_floating.isNull() || m_selRect.isNull()) return;
    pushUndo();
    QRect r = m_selRect.intersected(m_image.rect());
    m_floating = m_image.copy(m_selRect).convertToFormat(QImage::Format_ARGB32);
    if (m_transparentSel) makeTransparent(m_floating);
    QPainter(&m_image).fillRect(r, m_secondaryColor);
}

void Canvas::commitFloating() {
    if (m_floating.isNull()) return;
    QPainter(&m_image).drawImage(m_selRect.topLeft(), m_floating);
    m_floating = QImage();
    clearSelection();
    update();
    emit imageChanged();
}

void Canvas::selectAll() {
    commitAll();
    m_selRect = m_image.rect();
    emit selectionChanged(m_selRect);
    update();
}

void Canvas::deleteSelection() {
    if (m_selRect.isNull()) return;
    if (!m_floating.isNull()) {
        m_floating = QImage();     // дыра уже залита при «поднятии»
    } else {
        pushUndo();
        QPainter(&m_image).fillRect(m_selRect, m_secondaryColor);
    }
    clearSelection();
    update();
    emit imageChanged();
}

void Canvas::copySelection() {
    if (m_selRect.isNull()) return;
    QImage img = m_floating.isNull() ? m_image.copy(m_selRect) : m_floating;
    QApplication::clipboard()->setImage(img);
}

void Canvas::cutSelection() {
    copySelection();
    deleteSelection();
}

void Canvas::paste(const QImage &src) {
    if (src.isNull()) return;
    commitAll();
    if (m_mode != CanvasMode::Select) setMode(CanvasMode::Select);
    pushUndo();
    // если вставляемое больше листа — лист увеличивается, как в Paint
    if (src.width() > m_image.width() || src.height() > m_image.height()) {
        QImage bigger(QSize(std::max(src.width(), m_image.width()), std::max(src.height(), m_image.height())),
                      QImage::Format_ARGB32);
        bigger.fill(Qt::white);
        QPainter(&bigger).drawImage(0, 0, m_image);
        setImageInternal(bigger);
    }
    m_floating = src.convertToFormat(QImage::Format_ARGB32);
    if (m_transparentSel) makeTransparent(m_floating);
    m_selRect = QRect(QPoint(0, 0), m_floating.size());
    emit selectionChanged(m_selRect);
    update();
    emit imageChanged();
}

void Canvas::crop() {
    if (m_selRect.isNull()) return;
    QRect r = m_selRect.intersected(m_image.rect());
    QImage part = m_floating.isNull() ? m_image.copy(r) : m_floating;
    pushUndo();
    m_floating = QImage();
    clearSelection();
    QImage img(part.size(), QImage::Format_ARGB32);
    img.fill(Qt::white);
    QPainter(&img).drawImage(0, 0, part);
    setImageInternal(img);
}

// ---------------------------------------------------------------- текст

int Canvas::textPixelSize(float zoom) const {
    // пункты -> пиксели при 96 dpi, как в Paint
    return std::max(1, qRound(m_textFont.pointSizeF() * 96.0 / 72.0 * zoom));
}

void Canvas::startText(const QRect &rect) {
    m_textRect = rect;
    m_textEdit = new QPlainTextEdit(this);
    m_textEdit->setFrameShape(QFrame::NoFrame);
    m_textEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    m_textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_textEdit->document()->setDocumentMargin(0);
    m_textEdit->setGeometry(toWidget(m_textRect));
    updateTextEditStyle();
    m_textEdit->show();
    m_textEdit->setFocus();
    // рамка растёт вниз, если текст не помещается
    connect(m_textEdit, &QPlainTextEdit::textChanged, this, [this]() {
        if (!m_textEdit) return;
        QFont f = m_textFont;
        f.setPixelSize(textPixelSize(1.0f));
        QFontMetrics fm(f);
        QRect need = fm.boundingRect(QRect(0, 0, m_textRect.width(), 100000),
                                     Qt::TextWordWrap, m_textEdit->toPlainText() + "W");
        if (need.height() + 4 > m_textRect.height()) {
            m_textRect.setHeight(std::min(need.height() + 4, m_image.height() - m_textRect.top()));
            m_textEdit->setGeometry(toWidget(m_textRect));
        }
    });
    emit textEditingChanged(true);
}

void Canvas::updateTextEditStyle() {
    if (!m_textEdit) return;
    QFont f = m_textFont;
    f.setPixelSize(textPixelSize(m_zoom));
    m_textEdit->setFont(f);
    m_textEdit->setStyleSheet(QString(
        "QPlainTextEdit { background: %1; color: %2; border: 1px dashed #333; }")
        .arg(m_textOpaque ? m_secondaryColor.name() : "transparent", m_primaryColor.name()));
}

void Canvas::setTextFont(const QFont &font) {
    m_textFont = font;
    updateTextEditStyle();
}

void Canvas::setTextOpaque(bool opaque) {
    m_textOpaque = opaque;
    updateTextEditStyle();
}

void Canvas::commitText() {
    if (!m_textEdit) return;
    const QString text = m_textEdit->toPlainText();
    if (!text.isEmpty()) {
        pushUndo();
        QPainter p(&m_image);
        p.setRenderHint(QPainter::TextAntialiasing);
        if (m_textOpaque) p.fillRect(m_textRect, m_secondaryColor);
        QFont f = m_textFont;
        f.setPixelSize(textPixelSize(1.0f));
        p.setFont(f);
        p.setPen(m_primaryColor);
        p.drawText(m_textRect.adjusted(1, 1, 0, 0), Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, text);
    }
    m_textEdit->deleteLater();
    m_textEdit = nullptr;
    setFocus();
    update();
    emit imageChanged();
    emit textEditingChanged(false);
}
