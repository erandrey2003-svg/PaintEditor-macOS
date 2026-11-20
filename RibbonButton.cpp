#include "RibbonButton.h"
#include <QPainter>
#include <QMenu>
#include <QStyleOption>

RibbonButton::RibbonButton(Kind kind, const QString &text, const QIcon &icon, bool arrow, QWidget *parent)
    : QToolButton(parent), m_kind(kind), m_arrow(arrow) {
    setText(text);
    setIcon(icon);
    setIconSize(kind == Small ? QSize(16, 16) : QSize(32, 32));
    setAutoRaise(true);
    setToolTip(QString(text).replace('\n', ' '));
    setFocusPolicy(Qt::NoFocus);
    if (arrow) setPopupMode(QToolButton::InstantPopup);
}

QSize RibbonButton::sizeHint() const {
    const QFontMetrics fm(font());
    const QStringList lines = text().isEmpty() ? QStringList() : text().split('\n');
    int tw = 0;
    for (const QString &l : lines) tw = std::max(tw, fm.horizontalAdvance(l));

    if (m_kind == Small) {
        int w = 4 + (icon().isNull() ? 0 : iconSize().width() + 4) + tw + (m_arrow ? 12 : 0) + 4;
        if (text().isEmpty()) w = iconSize().width() + 8;
        return QSize(w, iconSize().height() + 8);
    }
    const int lineH = fm.height();
    int h = 4 + int(lines.size()) * lineH + (m_arrow ? 9 : 0) + 4;
    int w = std::max(tw + 12, 30);
    if (m_kind == Large) {
        h += iconSize().height() + 2;
        w = std::max(w, iconSize().width() + 12);
    }
    return QSize(w, h);
}

void RibbonButton::enterEvent(QEnterEvent *e) { QToolButton::enterEvent(e); update(); }
void RibbonButton::leaveEvent(QEvent *e)      { QToolButton::leaveEvent(e); update(); }

void RibbonButton::paintEvent(QPaintEvent *) {
    QPainter p(this);
    const QRect r = rect().adjusted(0, 0, -1, -1);
    const bool on = isEnabled() && (isDown() || isChecked() || (menu() && menu()->isVisible()));
    const bool hover = isEnabled() && underMouse();

    // фон как у ленты Windows 7
    if (on) {
        p.setPen(QColor("#99c9ef"));
        p.setBrush(QColor("#cce4f7"));
        p.drawRect(r);
    } else if (hover) {
        p.setPen(QColor("#cce4f7"));
        p.setBrush(QColor("#e5f3ff"));
        p.drawRect(r);
    }

    const QColor textColor = isEnabled() ? QColor("#202020") : QColor("#a0a0a0");
    const QIcon::Mode mode = isEnabled() ? QIcon::Normal : QIcon::Disabled;
    const QFontMetrics fm(font());
    const QStringList lines = text().isEmpty() ? QStringList() : text().split('\n');
    p.setPen(textColor);

    auto arrow = [&](const QPointF &c) {
        p.save();
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        p.setBrush(textColor);
        p.drawPolygon(QPolygonF({c + QPointF(-3.5, -1.5), c + QPointF(3.5, -1.5), c + QPointF(0, 2.5)}));
        p.restore();
    };

    if (m_kind == Small) {
        int x = 4;
        if (!icon().isNull()) {
            QPixmap pm = icon().pixmap(iconSize(), mode);
            int ix = text().isEmpty() ? (width() - iconSize().width()) / 2 : x;
            p.drawPixmap(ix, (height() - iconSize().height()) / 2, pm);
            x += iconSize().width() + 4;
        }
        if (!text().isEmpty())
            p.drawText(QRect(x, 0, width() - x, height()), Qt::AlignLeft | Qt::AlignVCenter, text());
        if (m_arrow) arrow(QPointF(width() - 8, height() / 2.0));
        return;
    }

    int y = 4;
    if (m_kind == Large && !icon().isNull()) {
        QPixmap pm = icon().pixmap(iconSize(), mode);
        p.drawPixmap((width() - iconSize().width()) / 2, y, pm);
        y += iconSize().height() + 2;
    }
    for (const QString &l : lines) {
        p.drawText(QRect(0, y, width(), fm.height()), Qt::AlignHCenter | Qt::AlignVCenter, l);
        y += fm.height();
    }
    if (m_arrow) arrow(QPointF(width() / 2.0, y + 4));
}
