#include "FileMenu.h"
#include "Icons.h"
#include <QToolButton>
#include <QCommandLinkButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QEvent>
#include <QFileInfo>

FileMenu::FileMenu(QWidget *parent) : QFrame(parent, Qt::Popup) {
    setObjectName("fileMenu");
    setStyleSheet(R"(
        #fileMenu { background: #ffffff; border: 1px solid #9aa9bb; }
        #fileLeft { background: #ffffff; border-right: 1px solid #d5dbe3; }
        #fileLeft QToolButton { border: 1px solid transparent; padding: 4px 6px; text-align: left; color: #1e1e1e; }
        #fileLeft QToolButton:hover { background: #e5f3ff; border-color: #cce4f7; }
        #fileLeft QToolButton:disabled { color: #9a9a9a; }
        #fileRight { background: #f4f7fb; }
        QLabel#pageTitle { font-weight: bold; color: #1e395b; padding: 6px 4px;
                           border-bottom: 1px solid #c9d3df; }
        #fileRight QToolButton { border: 1px solid transparent; padding: 3px 4px; text-align: left; color: #1e1e1e; }
        #fileRight QToolButton:hover { background: #e5f3ff; border-color: #cce4f7; }
        QCommandLinkButton { color: #1e1e1e; }
    )");

    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(1, 1, 1, 1);
    root->setSpacing(0);

    // ------------------------------------------------ левая колонка
    auto *left = new QWidget;
    left->setObjectName("fileLeft");
    left->setFixedWidth(320);
    auto *l = new QVBoxLayout(left);
    l->setContentsMargins(4, 4, 4, 4);
    l->setSpacing(0);

    connect(addItem(l, Icons::fileNew(), "Создать"), &QToolButton::clicked, this, &FileMenu::newFile);
    connect(addItem(l, Icons::fileOpen(), "Открыть"), &QToolButton::clicked, this, &FileMenu::openFile);
    connect(addItem(l, Icons::fileSave(), "Сохранить"), &QToolButton::clicked, this, &FileMenu::save);
    m_saveAsBtn = addItem(l, Icons::fileSaveAs(), "Сохранить как", true);
    connect(m_saveAsBtn, &QToolButton::clicked, this, [this]() { hide(); emit saveAs("png"); });
    addSeparator(l);
    m_printBtn = addItem(l, Icons::print(), "Печать", true);
    connect(m_printBtn, &QToolButton::clicked, this, &FileMenu::print);
    addSeparator(l);
    addItem(l, Icons::scanner(), "Со сканера или камеры", false, false);
    connect(addItem(l, Icons::email(), "Отправить по электронной почте"), &QToolButton::clicked, this, &FileMenu::sendEmail);
    m_desktopBtn = addItem(l, Icons::desktop(), "Сделать фоном рабочего стола", true, false);
    addSeparator(l);
    connect(addItem(l, Icons::properties(), "Свойства"), &QToolButton::clicked, this, &FileMenu::properties);
    addSeparator(l);
    connect(addItem(l, Icons::about(), "О программе"), &QToolButton::clicked, this, &FileMenu::about);
    addSeparator(l);
    connect(addItem(l, Icons::exit(), "Выход"), &QToolButton::clicked, this, &FileMenu::exitApp);
    l->addStretch();
    root->addWidget(left);

    // ------------------------------------------------ правая колонка (страницы)
    m_pages = new QStackedWidget;
    m_pages->setObjectName("fileRight");
    m_pages->setMinimumWidth(380);

    // 0: последние изображения
    m_pages->addWidget(makePage("Последние изображения", &m_recentLay));

    // 1: сохранить как
    {
        QVBoxLayout *b;
        m_pages->addWidget(makePage("Сохранить копию документа", &b));
        struct F { const char *fmt, *title, *desc; };
        const F formats[] = {
            {"png", "Изображение в формате PNG", "Сохранение фотографии или рисунка высокого качества для использования на компьютере или в Интернете."},
            {"jpg", "Изображение в формате JPEG", "Сохранение фотографии с хорошим качеством для использования на компьютере, в электронной почте или в Интернете."},
            {"bmp", "Изображение в формате BMP", "Сохранение любого рисунка с высоким качеством для использования на компьютере."},
            {"gif", "Изображение в формате GIF", "Сохранение простого рисунка для использования в электронной почте или в Интернете."},
            {"",    "Другие форматы", "Открытие диалогового окна «Сохранить как» для выбора всех возможных типов файлов."},
        };
        for (const F &f : formats) {
            auto *c = new QCommandLinkButton(QString::fromUtf8(f.title), QString::fromUtf8(f.desc));
            c->setIcon(Icons::fileSaveAs());
            c->setIconSize(QSize(32, 32));
            const QString fmt = QString::fromUtf8(f.fmt);
            connect(c, &QCommandLinkButton::clicked, this, [this, fmt]() { hide(); emit saveAs(fmt); });
            b->addWidget(c);
        }
        b->addStretch();
    }
    // 2: печать
    {
        QVBoxLayout *b;
        m_pages->addWidget(makePage("Предварительный просмотр и печать документа", &b));
        auto add = [&](const QString &t, const QString &d, void (FileMenu::*sig)()) {
            auto *c = new QCommandLinkButton(t, d);
            c->setIcon(Icons::print());
            c->setIconSize(QSize(32, 32));
            connect(c, &QCommandLinkButton::clicked, this, [this, sig]() { hide(); emit (this->*sig)(); });
            b->addWidget(c);
        };
        add("Печать", "Выбор принтера, числа копий и других параметров печати перед печатью.", &FileMenu::print);
        add("Параметры страницы", "Изменение макета изображения.", &FileMenu::pageSetup);
        add("Предварительный просмотр", "Предварительный просмотр и внесение изменений перед печатью.", &FileMenu::printPreview);
        b->addStretch();
    }
    root->addWidget(m_pages, 1);
}

QToolButton *FileMenu::addItem(QVBoxLayout *lay, const QIcon &icon, const QString &text,
                               bool submenu, bool enabled) {
    auto *b = new QToolButton;
    b->setIcon(icon);
    b->setIconSize(QSize(32, 32));
    b->setText(submenu ? text + QString(" ").repeated(4) + "▸" : text);
    b->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    b->setMinimumHeight(44);
    b->setAutoRaise(true);
    b->setEnabled(enabled);
    b->installEventFilter(this);       // наведение переключает правую панель
    if (!submenu) connect(b, &QToolButton::clicked, this, &QWidget::hide);
    lay->addWidget(b);
    return b;
}

void FileMenu::addSeparator(QVBoxLayout *lay) {
    auto *line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("color: #e2e6eb;");
    lay->addWidget(line);
}

QWidget *FileMenu::makePage(const QString &title, QVBoxLayout **body) {
    auto *w = new QWidget;
    auto *v = new QVBoxLayout(w);
    v->setContentsMargins(8, 4, 8, 4);
    v->setSpacing(2);
    auto *t = new QLabel(title);
    t->setObjectName("pageTitle");
    v->addWidget(t);
    *body = v;
    return w;
}

void FileMenu::setRecentFiles(const QStringList &files) {
    // удаляем старые строки (кроме заголовка)
    while (m_recentLay->count() > 1) {
        QLayoutItem *it = m_recentLay->takeAt(1);
        if (it->widget()) it->widget()->deleteLater();
        delete it;
    }
    int i = 1;
    for (const QString &path : files) {
        auto *b = new QToolButton;
        b->setText(QString("%1   %2").arg(i++).arg(QFileInfo(path).completeBaseName()));
        b->setToolTip(path);
        b->setToolButtonStyle(Qt::ToolButtonTextOnly);
        b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        b->setAutoRaise(true);
        connect(b, &QToolButton::clicked, this, [this, path]() { hide(); emit openRecent(path); });
        m_recentLay->addWidget(b);
    }
    if (files.isEmpty()) {
        auto *l = new QLabel("  (список пуст)");
        l->setStyleSheet("color: #888;");
        m_recentLay->addWidget(l);
    }
    m_recentLay->addStretch();
}

bool FileMenu::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::Enter) {
        if (obj == m_saveAsBtn) m_pages->setCurrentIndex(1);
        else if (obj == m_printBtn) m_pages->setCurrentIndex(2);
        else if (obj != m_desktopBtn) m_pages->setCurrentIndex(0);
    }
    return QFrame::eventFilter(obj, event);
}

void FileMenu::popup(const QPoint &globalPos, int height) {
    m_pages->setCurrentIndex(0);
    resize(720, std::max(height, sizeHint().height()));
    move(globalPos);
    show();
}
