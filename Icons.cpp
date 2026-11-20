#include "Icons.h"
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <functional>
#include <QtMath>

namespace {

// Рисуем иконку в квадрате 64x64
QIcon make(const std::function<void(QPainter &)> &draw, int size = 64) {
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.scale(size / 64.0, size / 64.0);
    draw(p);
    p.end();
    return QIcon(pm);
}

const QColor kBlue("#2f6db5");
const QColor kLightBlue("#9cc3ea");
const QColor kDark("#3a3a3a");
const QColor kPaper("#ffffff");

void paper(QPainter &p, const QRectF &r) {
    p.setPen(QPen(QColor("#7a8a9a"), 2));
    p.setBrush(kPaper);
    p.drawRect(r);
}

void magnifierGlass(QPainter &p, const QPointF &c, double r) {
    p.setPen(QPen(QColor("#8a6a2a"), 7, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(c + QPointF(r * 0.7, r * 0.7), c + QPointF(r * 1.6, r * 1.6));
    p.setPen(QPen(QColor("#555"), 4));
    p.setBrush(QColor(220, 238, 255, 200));
    p.drawEllipse(c, r, r);
}

} // namespace

namespace Icons {

// ---------------------------------------------------------------- «Инструменты»

QIcon pencil() {
    return make([](QPainter &p) {
        p.translate(32, 32);
        p.rotate(45);
        p.setPen(QPen(kDark, 1.5));
        p.setBrush(QColor("#f5c518"));
        p.drawRect(QRectF(-6, -28, 12, 38));          // корпус
        p.setBrush(QColor("#e88"));
        p.drawRect(QRectF(-6, -32, 12, 6));            // ластик
        p.setBrush(QColor("#f1d9a8"));
        p.drawPolygon(QPolygonF({QPointF(-6, 10), QPointF(6, 10), QPointF(0, 26)}));
        p.setBrush(kDark);
        p.drawPolygon(QPolygonF({QPointF(-2, 20), QPointF(2, 20), QPointF(0, 26)}));
    });
}

QIcon bucket() {
    return make([](QPainter &p) {
        p.translate(30, 34);
        p.rotate(-30);
        p.setPen(QPen(kDark, 2));
        QLinearGradient g(-14, 0, 14, 0);
        g.setColorAt(0, QColor("#cfd8e3"));
        g.setColorAt(1, QColor("#8fa3b8"));
        p.setBrush(g);
        p.drawRoundedRect(QRectF(-14, -14, 28, 30), 3, 3);
        p.setBrush(Qt::NoBrush);
        p.drawArc(QRectF(-10, -26, 20, 22), 0, 180 * 16);  // ручка
        p.resetTransform();
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#e8641b"));
        QPainterPath drop;                                  // капля краски
        drop.moveTo(50, 36);
        drop.cubicTo(44, 48, 46, 56, 52, 56);
        drop.cubicTo(58, 56, 60, 48, 50, 36);
        p.drawPath(drop);
    });
}

QIcon text() {
    return make([](QPainter &p) {
        QFont f("Times New Roman");
        f.setBold(true);
        f.setPixelSize(56);
        p.setFont(f);
        p.setPen(QColor("#1f3864"));
        p.drawText(QRectF(0, 0, 64, 64), Qt::AlignCenter, "A");
    });
}

QIcon eraser() {
    return make([](QPainter &p) {
        p.translate(32, 34);
        p.rotate(-30);
        p.setPen(QPen(QColor("#a0505a"), 2));
        p.setBrush(QColor("#f08a96"));
        p.drawRoundedRect(QRectF(-24, -12, 48, 24), 5, 5);
        p.setBrush(QColor("#f7b8c0"));
        p.drawRoundedRect(QRectF(-24, -12, 48, 9), 4, 4);
    });
}

QIcon picker() {
    return make([](QPainter &p) {
        p.translate(32, 32);
        p.rotate(45);
        p.setPen(QPen(kDark, 1.5));
        p.setBrush(QColor("#1e5fbf"));
        p.drawRoundedRect(QRectF(-7, -30, 14, 16), 4, 4);   // резиновая груша
        p.setBrush(QColor("#2a2a2a"));
        p.drawRect(QRectF(-8, -15, 16, 5));
        p.setBrush(QColor(200, 230, 255));
        p.drawPolygon(QPolygonF({QPointF(-4, -10), QPointF(4, -10), QPointF(2, 22), QPointF(-2, 22)}));
        p.setBrush(QColor("#3b8de0"));
        p.drawPolygon(QPolygonF({QPointF(-3, 6), QPointF(3, 6), QPointF(2, 22), QPointF(-2, 22)}));
    });
}

QIcon magnifier() {
    return make([](QPainter &p) { magnifierGlass(p, QPointF(26, 26), 16); });
}

// ---------------------------------------------------------------- «Буфер обмена» и «Изображение»

QIcon paste() {
    return make([](QPainter &p) {
        p.setPen(QPen(QColor("#6b8bb0"), 2));
        p.setBrush(QColor("#dfe8f3"));
        p.drawRoundedRect(QRectF(10, 8, 44, 52), 3, 3);
        p.setBrush(QColor("#9fb3cb"));
        p.drawRoundedRect(QRectF(22, 3, 20, 10), 3, 3);
        p.setBrush(kPaper);
        p.drawRect(QRectF(17, 18, 30, 36));
    });
}

QIcon cut() {
    return make([](QPainter &p) {
        p.setPen(QPen(QColor("#6f7f90"), 4, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(20, 6), QPointF(40, 40));
        p.drawLine(QPointF(44, 6), QPointF(24, 40));
        p.setPen(QPen(kBlue, 4));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(19, 48), 8, 8);
        p.drawEllipse(QPointF(45, 48), 8, 8);
    });
}

QIcon copy() {
    return make([](QPainter &p) {
        paper(p, QRectF(8, 6, 32, 38));
        paper(p, QRectF(24, 20, 32, 38));
        p.setPen(QPen(kLightBlue, 2));
        for (int y = 30; y < 54; y += 6) p.drawLine(30, y, 50, y);
    });
}

QIcon select() {
    return make([](QPainter &p) {
        QPen pen(kDark, 2, Qt::DotLine);
        p.setPen(pen);
        p.setBrush(QColor(255, 255, 255, 180));
        p.drawRect(QRectF(6, 12, 52, 40));
    });
}

QIcon crop() {
    return make([](QPainter &p) {
        p.setPen(QPen(kBlue, 4));
        p.drawLine(16, 4, 16, 48);
        p.drawLine(16, 48, 60, 48);
        p.drawLine(4, 16, 48, 16);
        p.drawLine(48, 16, 48, 60);
    });
}

QIcon resize() {
    return make([](QPainter &p) {
        p.setPen(QPen(kBlue, 3));
        p.setBrush(kPaper);
        p.drawRect(QRectF(22, 6, 36, 30));
        p.setBrush(QColor("#dbe9f8"));
        p.drawRect(QRectF(6, 24, 30, 30));
    });
}

static void rotateArrow(QPainter &p, bool clockwise) {
    p.setPen(QPen(kBlue, 4, Qt::SolidLine, Qt::RoundCap));
    p.setBrush(Qt::NoBrush);
    p.drawArc(QRectF(10, 10, 40, 40), clockwise ? 200 * 16 : -20 * 16, clockwise ? -220 * 16 : 220 * 16);
    p.setPen(Qt::NoPen);
    p.setBrush(kBlue);
    if (clockwise) p.drawPolygon(QPolygonF({QPointF(44, 40), QPointF(58, 38), QPointF(52, 52)}));
    else           p.drawPolygon(QPolygonF({QPointF(20, 40), QPointF(6, 38), QPointF(12, 52)}));
}

QIcon rotate() {
    return make([](QPainter &p) {
        p.setPen(Qt::NoPen);
        p.setBrush(kBlue);
        p.drawPolygon(QPolygonF({QPointF(26, 58), QPointF(58, 58), QPointF(58, 14)}));
        p.setPen(QPen(kBlue, 3));
        p.setBrush(Qt::NoBrush);
        p.drawArc(QRectF(4, 6, 40, 40), 90 * 16, 90 * 16);
        p.drawLine(QPointF(4, 26), QPointF(10, 20));
    });
}
QIcon rotateRight() { return make([](QPainter &p) { rotateArrow(p, true); }); }
QIcon rotateLeft()  { return make([](QPainter &p) { rotateArrow(p, false); }); }
QIcon rotate180() {
    return make([](QPainter &p) {
        rotateArrow(p, true);
        QFont f;
        f.setPixelSize(18);
        f.setBold(true);
        p.setFont(f);
        p.setPen(kDark);
        p.drawText(QRectF(10, 18, 40, 24), Qt::AlignCenter, "180");
    });
}
QIcon flipVertical() {
    return make([](QPainter &p) {
        p.setPen(Qt::NoPen);
        p.setBrush(kBlue);
        p.drawPolygon(QPolygonF({QPointF(10, 28), QPointF(54, 28), QPointF(32, 4)}));
        p.setBrush(kLightBlue);
        p.drawPolygon(QPolygonF({QPointF(10, 36), QPointF(54, 36), QPointF(32, 60)}));
    });
}
QIcon flipHorizontal() {
    return make([](QPainter &p) {
        p.setPen(Qt::NoPen);
        p.setBrush(kBlue);
        p.drawPolygon(QPolygonF({QPointF(28, 10), QPointF(28, 54), QPointF(4, 32)}));
        p.setBrush(kLightBlue);
        p.drawPolygon(QPolygonF({QPointF(36, 10), QPointF(36, 54), QPointF(60, 32)}));
    });
}

// ---------------------------------------------------------------- «Вид»

static void plusMinus(QPainter &p, bool plus) {
    magnifierGlass(p, QPointF(26, 26), 18);
    p.setPen(QPen(QColor("#1e6fd9"), 6, Qt::SolidLine, Qt::FlatCap));
    p.drawLine(QPointF(16, 26), QPointF(36, 26));
    if (plus) p.drawLine(QPointF(26, 16), QPointF(26, 36));
}
QIcon zoomIn()  { return make([](QPainter &p) { plusMinus(p, true); }); }
QIcon zoomOut() { return make([](QPainter &p) { plusMinus(p, false); }); }

QIcon zoom100() {
    return make([](QPainter &p) {
        paper(p, QRectF(12, 4, 40, 56));
        p.setPen(QPen(kBlue, 2));
        p.drawLine(QPointF(40, 8), QPointF(48, 8));
        p.drawLine(QPointF(48, 8), QPointF(48, 16));
        p.drawLine(QPointF(16, 48), QPointF(16, 56));
        p.drawLine(QPointF(16, 56), QPointF(24, 56));
    });
}

QIcon fullScreen() {
    return make([](QPainter &p) {
        p.setPen(QPen(QColor("#557"), 2));
        QLinearGradient g(0, 6, 0, 58);
        g.setColorAt(0, QColor("#bcd8f3"));
        g.setColorAt(1, QColor("#4f84c4"));
        p.setBrush(g);
        p.drawRect(QRectF(4, 8, 56, 48));
        p.setPen(QPen(Qt::white, 4, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(20, 44), QPointF(46, 20));
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::white);
        p.drawPolygon(QPolygonF({QPointF(34, 16), QPointF(52, 14), QPointF(50, 32)}));
    });
}

QIcon thumbnail() {
    return make([](QPainter &p) {
        p.setPen(QPen(QColor("#557"), 2));
        p.setBrush(QColor("#d6e6f6"));
        p.drawRect(QRectF(4, 20, 40, 36));
        p.setBrush(QColor("#7aa6d8"));
        p.drawRect(QRectF(26, 6, 34, 26));
        p.setBrush(QColor("#ffffff"));
        p.drawRect(QRectF(30, 10, 26, 18));
    });
}

// ---------------------------------------------------------------- меню «Файл»

QIcon fileNew() {
    return make([](QPainter &p) {
        p.setPen(QPen(QColor("#8a9aaa"), 2));
        p.setBrush(kPaper);
        p.drawPolygon(QPolygonF({QPointF(12, 4), QPointF(40, 4), QPointF(52, 16), QPointF(52, 60), QPointF(12, 60)}));
        p.drawPolyline(QPolygonF({QPointF(40, 4), QPointF(40, 16), QPointF(52, 16)}));
    });
}

QIcon fileOpen() {
    return make([](QPainter &p) {
        p.setPen(QPen(QColor("#b0841c"), 2));
        p.setBrush(QColor("#f0c24a"));
        p.drawPolygon(QPolygonF({QPointF(4, 14), QPointF(22, 14), QPointF(28, 20), QPointF(56, 20), QPointF(56, 56), QPointF(4, 56)}));
        p.setPen(QPen(kBlue, 5, Qt::SolidLine, Qt::RoundCap));
        p.drawArc(QRectF(26, 4, 30, 30), 180 * 16, -150 * 16);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#f7d77a"));
        p.drawPolygon(QPolygonF({QPointF(10, 30), QPointF(62, 30), QPointF(56, 56), QPointF(4, 56)}));
    });
}

static void floppy(QPainter &p) {
    p.setPen(QPen(QColor("#2b3f63"), 2));
    p.setBrush(QColor("#4a6fa5"));
    p.drawRoundedRect(QRectF(6, 6, 52, 52), 4, 4);
    p.setBrush(QColor("#e8eef6"));
    p.drawRect(QRectF(14, 6, 36, 20));
    p.setBrush(QColor("#2b3f63"));
    p.drawRect(QRectF(38, 9, 7, 14));
    p.setBrush(QColor("#c8d4e4"));
    p.drawRect(QRectF(14, 36, 36, 22));
}
QIcon fileSave() { return make([](QPainter &p) { floppy(p); }); }
QIcon fileSaveAs() {
    return make([](QPainter &p) {
        floppy(p);
        p.translate(46, 44);
        p.rotate(45);
        p.setPen(QPen(kDark, 1));
        p.setBrush(QColor("#f5c518"));
        p.drawRect(QRectF(-4, -24, 8, 24));
        p.setBrush(QColor("#f1d9a8"));
        p.drawPolygon(QPolygonF({QPointF(-4, 0), QPointF(4, 0), QPointF(0, 8)}));
    });
}

QIcon print() {
    return make([](QPainter &p) {
        paper(p, QRectF(16, 4, 32, 24));
        p.setPen(QPen(QColor("#444"), 2));
        p.setBrush(QColor("#9aa4ae"));
        p.drawRoundedRect(QRectF(4, 24, 56, 24), 4, 4);
        paper(p, QRectF(14, 40, 36, 18));
    });
}

QIcon scanner() {
    return make([](QPainter &p) {
        p.setPen(QPen(QColor("#555"), 2));
        p.setBrush(QColor("#c9ced4"));
        p.drawRoundedRect(QRectF(4, 30, 56, 20), 3, 3);
        p.setBrush(QColor("#8dc0ea"));
        p.drawRect(QRectF(10, 34, 44, 6));
        p.setBrush(QColor("#666"));
        p.drawRoundedRect(QRectF(34, 6, 24, 18), 3, 3);
        p.setBrush(QColor("#9ccbf0"));
        p.drawEllipse(QPointF(46, 15), 5, 5);
    });
}

QIcon email() {
    return make([](QPainter &p) {
        p.setPen(QPen(QColor("#8a6a4a"), 2));
        p.setBrush(QColor("#fdf6e3"));
        p.drawRect(QRectF(4, 14, 56, 38));
        p.drawPolyline(QPolygonF({QPointF(4, 14), QPointF(32, 36), QPointF(60, 14)}));
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#d33"));
        p.drawRect(QRectF(44, 18, 10, 10));
    });
}

QIcon desktop() {
    return make([](QPainter &p) {
        p.setPen(QPen(QColor("#444"), 2));
        p.setBrush(QColor("#6aa6e0"));
        p.drawRect(QRectF(4, 8, 56, 38));
        p.setBrush(QColor("#888"));
        p.drawRect(QRectF(24, 46, 16, 8));
        p.drawRect(QRectF(14, 54, 36, 4));
    });
}

QIcon properties() {
    return make([](QPainter &p) {
        p.setPen(QPen(QColor("#d2581a"), 9, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.drawPolyline(QPolygonF({QPointF(10, 34), QPointF(24, 50), QPointF(54, 12)}));
    });
}

QIcon about() {
    return make([](QPainter &p) {
        QRadialGradient g(QPointF(26, 22), 34);
        g.setColorAt(0, QColor("#6fb1f0"));
        g.setColorAt(1, QColor("#1c5fb0"));
        p.setPen(Qt::NoPen);
        p.setBrush(g);
        p.drawEllipse(QPointF(32, 32), 28, 28);
        QFont f("Times New Roman");
        f.setBold(true);
        f.setPixelSize(40);
        p.setFont(f);
        p.setPen(Qt::white);
        p.drawText(QRectF(0, 0, 64, 64), Qt::AlignCenter, "i");
    });
}

QIcon exit() {
    return make([](QPainter &p) {
        p.setPen(QPen(QColor("#b0841c"), 2));
        p.setBrush(QColor("#f0c24a"));
        p.drawRect(QRectF(10, 6, 40, 54));
        p.setBrush(QColor("#8a5a1a"));
        p.drawPolygon(QPolygonF({QPointF(10, 6), QPointF(34, 14), QPointF(34, 60), QPointF(10, 60)}));
        p.setPen(QPen(QColor("#2a8a2a"), 5, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(38, 34), QPointF(60, 34));
        p.drawLine(QPointF(60, 34), QPointF(52, 26));
        p.drawLine(QPointF(60, 34), QPointF(52, 42));
    });
}

QIcon undo() {
    return make([](QPainter &p) {
        p.setPen(QPen(kBlue, 7, Qt::SolidLine, Qt::FlatCap));
        p.drawArc(QRectF(14, 14, 40, 40), 150 * 16, -210 * 16);
        p.setPen(Qt::NoPen);
        p.setBrush(kBlue);
        p.drawPolygon(QPolygonF({QPointF(2, 30), QPointF(26, 30), QPointF(12, 14)}));
    });
}
QIcon redo() {
    return make([](QPainter &p) {
        p.setPen(QPen(kBlue, 7, Qt::SolidLine, Qt::FlatCap));
        p.drawArc(QRectF(10, 14, 40, 40), 30 * 16, 210 * 16);
        p.setPen(Qt::NoPen);
        p.setBrush(kBlue);
        p.drawPolygon(QPolygonF({QPointF(62, 30), QPointF(38, 30), QPointF(52, 14)}));
    });
}

QIcon paint() {
    return make([](QPainter &p) {
        p.setPen(QPen(QColor("#8a6a3a"), 2));
        p.setBrush(QColor("#f3e2c0"));
        p.drawEllipse(QRectF(4, 10, 56, 44));
        const QColor cs[] = {QColor("#ed1c24"), QColor("#22b14c"), QColor("#00a2e8"), QColor("#fff200")};
        const QPointF pts[] = {{20, 24}, {34, 20}, {46, 28}, {22, 40}};
        p.setPen(Qt::NoPen);
        for (int i = 0; i < 4; ++i) { p.setBrush(cs[i]); p.drawEllipse(pts[i], 5, 5); }
    });
}

// ---------------------------------------------------------------- фигуры, кисти

QIcon outline() {
    return make([](QPainter &p) {
        p.translate(30, 34);
        p.rotate(45);
        p.setPen(QPen(QColor("#888"), 2));
        p.setBrush(Qt::NoBrush);
        p.drawRect(QRectF(-6, -28, 12, 40));
        p.drawPolygon(QPolygonF({QPointF(-6, 12), QPointF(6, 12), QPointF(0, 24)}));
    });
}

QIcon fill() {
    return make([](QPainter &p) {
        p.translate(28, 34);
        p.rotate(-30);
        p.setPen(QPen(QColor("#888"), 2));
        p.setBrush(QColor("#e9edf2"));
        p.drawRoundedRect(QRectF(-14, -14, 28, 30), 3, 3);
        p.resetTransform();
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#7ba7d8"));
        p.drawRect(QRectF(4, 52, 56, 8));
    });
}

QIcon thickness() {
    return make([](QPainter &p) {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#1d2b4a"));
        p.drawRect(QRectF(6, 6, 52, 2));
        p.drawRect(QRectF(6, 16, 52, 4));
        p.drawRect(QRectF(6, 28, 52, 6));
        p.drawRect(QRectF(6, 44, 52, 10));
    });
}

QIcon colorEdit() {
    return make([](QPainter &p) {
        const QColor cs[] = {QColor("#ed1c24"), QColor("#ff7f27"), QColor("#fff200"),
                             QColor("#22b14c"), QColor("#00a2e8"), QColor("#a349a4")};
        for (int i = 0; i < 6; ++i) {
            p.setPen(Qt::NoPen);
            p.setBrush(cs[i]);
            p.drawPie(QRectF(6, 6, 52, 52), i * 60 * 16, 60 * 16);
        }
        p.setBrush(Qt::white);
        p.drawEllipse(QPointF(32, 32), 10, 10);
    });
}

static void textBg(QPainter &p, bool opaque) {
    p.setPen(QPen(QColor("#777"), 1.5));
    p.setBrush(opaque ? QColor("#ffffff") : Qt::NoBrush);
    if (opaque) p.drawRect(QRectF(20, 20, 40, 40));
    else {
        QPen d(QColor("#777"), 2, Qt::DotLine);
        p.setPen(d);
        p.drawRect(QRectF(20, 20, 40, 40));
    }
    QFont f("Times New Roman");
    f.setPixelSize(40);
    f.setBold(true);
    p.setFont(f);
    p.setPen(QColor("#1f3864"));
    p.drawText(QRectF(0, 0, 40, 44), Qt::AlignCenter, "A");
}
QIcon textOpaque()      { return make([](QPainter &p) { textBg(p, true); }); }
QIcon textTransparent() { return make([](QPainter &p) { textBg(p, false); }); }

QIcon galleryMore() {
    return make([](QPainter &p) {
        p.setPen(QPen(QColor("#555"), 3));
        p.drawLine(QPointF(18, 20), QPointF(46, 20));
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#555"));
        p.drawPolygon(QPolygonF({QPointF(18, 30), QPointF(46, 30), QPointF(32, 46)}));
    }, 16);
}

QIcon shape(ShapeType type) {
    return make([type](QPainter &p) {
        p.setPen(QPen(QColor("#3f7fc8"), 3.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.setBrush(Qt::NoBrush);
        QRectF r(6, 8, 52, 48);
        if (type == ShapeType::Line) r = QRectF(8, 8, 48, 48);
        p.drawPath(ShapeTool::shapePath(type, r));
    });
}

// Иконка кисти — пробный мазок самой кистью (оранжевым, как в Paint)
QIcon brush(BrushStyle style) {
    QImage img(64, 64, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    BrushTool tool(style);
    tool.setSize(2);
    const QColor c("#e8641b");
    QPoint prev(10, 48);
    tool.mousePress(img, prev, c);
    for (int i = 1; i <= 24; ++i) {
        double t = i / 24.0;
        QPoint pt(int(10 + 44 * t), int(48 - 34 * t + 10 * std::sin(t * M_PI * 2)));
        tool.mouseMove(img, prev, pt, c);
        prev = pt;
    }
    return QIcon(QPixmap::fromImage(img));
}

QPixmap thicknessLine(int width, int px, int length) {
    QPixmap pm(length, std::max(px, 16));
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.fillRect(QRect(4, (pm.height() - width) / 2, length - 8, width), QColor("#1d2b4a"));
    return pm;
}

} // namespace Icons
