#include "MainWindow.h"
#include "Canvas.h"
#include "BrushTool.h"
#include "PencilTool.h"
#include "FillTool.h"
#include "EraserTool.h"
#include "ShapeTool.h"
#include "Icons.h"
#include "RibbonButton.h"
#include "FileMenu.h"
#include "Dialogs.h"
#include "Ruler.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QTabWidget>
#include <QTabBar>
#include <QToolButton>
#include <QPushButton>
#include <QButtonGroup>
#include <QActionGroup>
#include <QWidgetAction>
#include <QScrollArea>
#include <QScrollBar>
#include <QSlider>
#include <QLabel>
#include <QFrame>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QFileDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPainter>
#include <QPixmap>
#include <QCheckBox>
#include <QFontComboBox>
#include <QComboBox>
#include <QCloseEvent>
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>
#include <QLocale>
#include <QScreen>
#include <QPrinter>
#include <QPrintDialog>
#include <QPageSetupDialog>
#include <QPrintPreviewDialog>
#include <QRegularExpression>
#include <functional>

// ================================================================ вспомогательное

namespace {

QIcon colorIcon(const QColor &c, int s = 32) {
    QPixmap pm(s, s);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setPen(QColor("#8a8a8a"));
    p.setBrush(Qt::white);
    p.drawRect(0, 0, s - 1, s - 1);
    p.fillRect(QRect(3, 3, s - 6, s - 6), c);
    return QIcon(pm);
}

const char *kStyle = R"(
QMainWindow { background: #ffffff; }
#header { background: #ffffff; }
#header QLabel#title { color: #1e1e1e; padding-left: 6px; }
#header QToolButton { border: 1px solid transparent; padding: 1px; }
#header QToolButton:hover { background: #e5f3ff; border-color: #cce4f7; }
QLabel#contextLabel { background: #d7ecc0; color: #3b5d22; border-left: 1px solid #b8d69a;
                      border-right: 1px solid #b8d69a; padding: 0 10px; }
QTabWidget::pane { border: none; border-top: 1px solid #d6d6d6; border-bottom: 1px solid #c5cdd6; background: #f5f6f7; }
QTabBar { background: #ffffff; }
QTabBar::tab { padding: 5px 16px; background: transparent; color: #202020; border: 1px solid transparent; }
QTabBar::tab:hover { color: #1979ca; }
QTabBar::tab:selected { background: #f5f6f7; border: 1px solid #d6d6d6; border-bottom: none; }
#ribbonPage { background: #f5f6f7; }
#ribbonPage QLabel { color: #202020; }
QLabel#groupTitle { color: #5c5c5c; }
QFrame#gallery { background: #ffffff; border: 1px solid #dadada; }
#ribbonPage QToolButton#galleryArrow { border: 1px solid #dadada; background: #f8f8f8; }
#ribbonPage QToolButton#galleryArrow:hover { background: #e5f3ff; border-color: #99c9ef; }
#ribbonPage QToolButton#fmt { border: 1px solid transparent; min-width: 22px; min-height: 22px; }
#ribbonPage QToolButton#fmt:hover { background: #e5f3ff; border-color: #cce4f7; }
#ribbonPage QToolButton#fmt:checked { background: #cce4f7; border-color: #99c9ef; }
QWidget#popupGrid { background: #ffffff; }
QWidget#popupGrid QToolButton { border: 1px solid transparent; background: #ffffff; }
QWidget#popupGrid QToolButton:hover { background: #e5f3ff; border-color: #cce4f7; }
QWidget#popupGrid QToolButton:checked { background: #cce4f7; border-color: #99c9ef; }
QToolButton#fileBtn { background: #1979ca; color: white; padding: 5px 22px; border: none; border-radius: 0; }
QToolButton#fileBtn:hover { background: #2a8ad4; }
QToolButton#fileBtn::menu-indicator { image: none; }
QStatusBar { background: #f0f0f0; color: #202020; border-top: 1px solid #d6d6d6; }
QStatusBar::item { border: none; }
QStatusBar QLabel { color: #202020; }
QStatusBar QToolButton { color: #202020; border: none; font-size: 15px; padding: 0 4px; }
)";

// Значок для строки состояния
QPixmap statusIcon(int kind) {
    QPixmap pm(16, 16);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor("#3a3a3a"), 1.2));
    switch (kind) {
    case 0:  // курсор
        p.drawLine(QPointF(8, 1), QPointF(8, 15));
        p.drawLine(QPointF(1, 8), QPointF(15, 8));
        p.drawRect(QRectF(5.5, 5.5, 5, 5));
        break;
    case 1: {  // выделение
        QPen d(QColor("#3a3a3a"), 1, Qt::DotLine);
        p.setPen(d);
        p.drawRect(QRectF(1.5, 3.5, 13, 9));
        break;
    }
    case 2:  // размер листа
        p.drawRect(QRectF(1.5, 3.5, 11, 9));
        p.setBrush(QColor("#3a3a3a"));
        p.drawRect(QRectF(11.5, 11.5, 3, 3));
        break;
    case 3:  // размер файла
        p.setBrush(QColor("#4a6fa5"));
        p.drawRect(QRectF(2.5, 2.5, 11, 11));
        p.fillRect(QRectF(5, 3, 6, 4), Qt::white);
        break;
    }
    return pm;
}

// Нижняя часть «разделённой» кнопки получает ту же ширину, что и верхняя
QWidget *makeSplit(RibbonButton *top, RibbonButton *bottom) {
    auto *w = new QWidget;
    auto *v = new QVBoxLayout(w);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(0);
    const int width = std::max(top->sizeHint().width(), bottom->sizeHint().width());
    top->setFixedWidth(width);
    bottom->setFixedWidth(width);
    v->addWidget(top);
    v->addWidget(bottom);
    v->addStretch();
    return w;
}

// Окно «Во весь экран»: картинка на тёмном фоне, любой щелчок/клавиша — выход
class FullScreenView : public QWidget {
public:
    explicit FullScreenView(const QImage &img) : m_img(img) {
        setAttribute(Qt::WA_DeleteOnClose);
        setCursor(Qt::ArrowCursor);
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.fillRect(rect(), QColor("#1f1f1f"));
        QSize s = m_img.size();
        if (s.width() > width() || s.height() > height()) s.scale(size(), Qt::KeepAspectRatio);
        QRect r(QPoint(), s);
        r.moveCenter(rect().center());
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        p.drawImage(r, m_img);
    }
    void mousePressEvent(QMouseEvent *) override { close(); }
    void keyPressEvent(QKeyEvent *) override { close(); }
private:
    QImage m_img;
};

// Плавающее окно «Эскиз»
class ThumbWindow : public QLabel {
public:
    std::function<void()> onClose;
    explicit ThumbWindow(QWidget *parent) : QLabel(parent, Qt::Tool) {
        setWindowTitle("Эскиз");
        setAlignment(Qt::AlignCenter);
        setStyleSheet("background: #cfd8e4;");
        resize(260, 180);
    }
protected:
    void closeEvent(QCloseEvent *e) override {
        if (onClose) onClose();
        QLabel::closeEvent(e);
    }
};

QPrinter &sharedPrinter() {
    static QPrinter printer(QPrinter::HighResolution);
    return printer;
}

const double kZoomLevels[] = {12.5, 25, 50, 100, 200, 300, 400, 500, 600, 700, 800};

} // namespace

// ================================================================ конструктор

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    m_canvas = new Canvas(this);
    m_customColors.fill(QColor(), 10);
    m_toolGroup = new QButtonGroup(this);
    m_toolGroup->setExclusive(true);

    auto *central = new QWidget(this);
    auto *lay = new QVBoxLayout(central);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);
    lay->addWidget(createQuickAccessBar());
    lay->addWidget(createRibbon());
    lay->addWidget(createWorkspace(), 1);
    setCentralWidget(central);
    setStyleSheet(kStyle);
    createStatusBar();
    createShortcuts();

    m_fileMenu = new FileMenu(this);
    connect(m_fileMenu, &FileMenu::newFile, this, &MainWindow::newFile);
    connect(m_fileMenu, &FileMenu::openFile, this, &MainWindow::openFile);
    connect(m_fileMenu, &FileMenu::save, this, &MainWindow::saveFile);
    connect(m_fileMenu, &FileMenu::saveAs, this, [this](const QString &f) { saveFileAs(f); });
    connect(m_fileMenu, &FileMenu::print, this, &MainWindow::printImage);
    connect(m_fileMenu, &FileMenu::pageSetup, this, &MainWindow::pageSetup);
    connect(m_fileMenu, &FileMenu::printPreview, this, &MainWindow::printPreview);
    connect(m_fileMenu, &FileMenu::sendEmail, this, &MainWindow::sendEmail);
    connect(m_fileMenu, &FileMenu::properties, this, &MainWindow::showProperties);
    connect(m_fileMenu, &FileMenu::about, this, &MainWindow::showAbout);
    connect(m_fileMenu, &FileMenu::exitApp, this, &QWidget::close);
    connect(m_fileMenu, &FileMenu::openRecent, this, [this](const QString &path) {
        if (maybeSave()) loadFile(path);
    });

    // --- сигналы холста
    connect(m_canvas, &Canvas::colorPicked, this, [this](const QColor &c, bool primary) {
        if (primary) m_canvas->setPrimaryColor(c); else m_canvas->setSecondaryColor(c);
        updateColorButtons();
        if (m_prevToolBtn) m_prevToolBtn->click();   // как в Paint — возврат к прежнему инструменту
    });
    connect(m_canvas, &Canvas::zoomRequested, this, &MainWindow::zoomStep);
    connect(m_canvas, &Canvas::selectionChanged, this, &MainWindow::onSelectionChanged);
    connect(m_canvas, &Canvas::imageSizeChanged, this, [this](const QSize &s) {
        m_sizeLabel->setText(QString("%1 × %2 пкс").arg(s.width()).arg(s.height()));
        updateRulers();
    });
    connect(m_canvas, &Canvas::imageChanged, this, [this]() {
        if (!m_modified) { m_modified = true; updateTitle(); }
        updateThumbnail();
    });
    connect(m_canvas, &Canvas::historyChanged, this, [this](bool u, bool r) {
        m_undoBtn->setEnabled(u);
        m_redoBtn->setEnabled(r);
    });
    connect(m_canvas, &Canvas::textEditingChanged, this, &MainWindow::showTextTab);

    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, [this]() {
        const bool has = QApplication::clipboard()->mimeData() && QApplication::clipboard()->mimeData()->hasImage();
        for (QWidget *w : m_pasteButtons) w->setEnabled(has);
    });
    emit QApplication::clipboard()->dataChanged();

    resize(1200, 800);
    m_canvas->setPrimaryColor(Qt::black);
    m_canvas->setSecondaryColor(Qt::white);
    updateColorButtons();
    refreshCustomColors();
    setActiveColorSlot(1);
    updateTextFont();
    onSelectionChanged(QRect());
    m_undoBtn->setEnabled(false);
    m_redoBtn->setEnabled(false);
    m_brushBtn->click();      // по умолчанию — кисть
    updateTitle();
}

// ================================================================ панель быстрого доступа

QWidget *MainWindow::createQuickAccessBar() {
    m_header = new QWidget;
    m_header->setObjectName("header");
    m_header->setFixedHeight(28);
    auto *h = new QHBoxLayout(m_header);
    h->setContentsMargins(6, 0, 6, 0);
    h->setSpacing(2);

    auto *logo = new QLabel;
    logo->setPixmap(Icons::paint().pixmap(18, 18));
    h->addWidget(logo);

    auto mk = [&](const QIcon &icon, const QString &tip) {
        auto *b = new QToolButton;
        b->setIcon(icon);
        b->setIconSize(QSize(16, 16));
        b->setToolTip(tip);
        b->setAutoRaise(true);
        b->setFocusPolicy(Qt::NoFocus);
        h->addWidget(b);
        return b;
    };
    connect(mk(Icons::fileSave(), "Сохранить (Ctrl+S)"), &QToolButton::clicked, this, &MainWindow::saveFile);
    m_undoBtn = mk(Icons::undo(), "Отменить (Ctrl+Z)");
    connect(m_undoBtn, &QToolButton::clicked, m_canvas, &Canvas::undo);
    m_redoBtn = mk(Icons::redo(), "Вернуть (Ctrl+Y)");
    connect(m_redoBtn, &QToolButton::clicked, m_canvas, &Canvas::redo);

    auto *sep = new QFrame;
    sep->setFrameShape(QFrame::VLine);
    sep->setStyleSheet("color: #d0d0d0;");
    h->addSpacing(4);
    h->addWidget(sep);

    m_titleLabel = new QLabel;
    m_titleLabel->setObjectName("title");
    h->addWidget(m_titleLabel);
    h->addStretch();

    // зелёная надпись над контекстной вкладкой «Текст»
    m_contextLabel = new QLabel("Инструменты работы с текстом", m_header);
    m_contextLabel->setObjectName("contextLabel");
    m_contextLabel->setAlignment(Qt::AlignCenter);
    m_contextLabel->hide();
    return m_header;
}

// ================================================================ лента

QWidget *MainWindow::makeGroup(const QString &title, QWidget *content) {
    auto *w = new QWidget;
    auto *v = new QVBoxLayout(w);
    v->setContentsMargins(4, 3, 4, 1);
    v->setSpacing(1);
    v->addWidget(content, 1, Qt::AlignTop | Qt::AlignHCenter);
    auto *l = new QLabel(title);
    l->setObjectName("groupTitle");
    l->setAlignment(Qt::AlignCenter);
    v->addWidget(l);
    return w;
}

static QFrame *groupSeparator() {
    auto *line = new QFrame;
    line->setFrameShape(QFrame::VLine);
    line->setStyleSheet("color: #dcdfe3;");
    return line;
}

static QWidget *ribbonPage(QHBoxLayout **lay) {
    auto *page = new QWidget;
    page->setObjectName("ribbonPage");
    auto *h = new QHBoxLayout(page);
    h->setContentsMargins(2, 0, 2, 0);
    h->setSpacing(0);
    *lay = h;
    return page;
}

QTabWidget *MainWindow::createRibbon() {
    m_ribbon = new QTabWidget;
    m_ribbon->setDocumentMode(true);
    m_ribbon->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // синяя кнопка «Файл» слева от вкладок
    m_fileBtn = new QToolButton;
    m_fileBtn->setObjectName("fileBtn");
    m_fileBtn->setText("Файл");
    m_fileBtn->setFocusPolicy(Qt::NoFocus);
    connect(m_fileBtn, &QToolButton::clicked, this, &MainWindow::showFileMenu);
    m_ribbon->setCornerWidget(m_fileBtn, Qt::TopLeftCorner);

    m_ribbon->addTab(createHomeTab(), "Главная");
    m_ribbon->addTab(createViewTab(), "Вид");
    m_textTab = createTextTab();    // добавляется только при вводе текста

    connect(m_ribbon, &QTabWidget::currentChanged, this, [this]() { positionContextLabel(); });
    return m_ribbon;
}

RibbonButton *MainWindow::addTool(RibbonButton *b) {
    b->setCheckable(true);
    m_toolGroup->addButton(b);
    return b;
}

// ------------------------------------------------ «Главная»

QWidget *MainWindow::createHomeTab() {
    QHBoxLayout *home;
    QWidget *page = ribbonPage(&home);

    home->addWidget(makeClipboardGroup());
    home->addWidget(groupSeparator());
    home->addWidget(makeImageGroup());
    home->addWidget(groupSeparator());
    home->addWidget(makeToolsGroup());
    home->addWidget(groupSeparator());
    home->addWidget(makeBrushesGroup());
    home->addWidget(groupSeparator());
    home->addWidget(makeShapesGroup());
    home->addWidget(groupSeparator());
    home->addWidget(makeSizeGroup());
    home->addWidget(makeColorsGroup());
    home->addStretch();

    // отслеживаем предыдущий инструмент (для «Палитры») и доступность «Контур»/«Заливка»
    connect(m_toolGroup, &QButtonGroup::buttonToggled, this, [this](QAbstractButton *b, bool checked) {
        if (!checked) { m_prevToolBtn = b; return; }
        const bool isShape = m_shapeBtns.contains(static_cast<RibbonButton *>(b));
        m_outlineBtn->setEnabled(isShape);
        m_fillBtn->setEnabled(isShape);
    });
    return page;
}

// «Буфер обмена» (есть и на вкладке «Главная», и на вкладке «Текст»)
QWidget *MainWindow::makeClipboardGroup() {
    auto *box = new QWidget;
    auto *h = new QHBoxLayout(box);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(2);

    auto pasteClipboard = [this]() {
        QImage img = QApplication::clipboard()->image();
        if (img.isNull()) return;
        m_selectBtn->setChecked(true);
        m_canvas->paste(img);
    };

    auto *top = new RibbonButton(RibbonButton::Large, "", Icons::paste());
    top->setToolTip("Вставить (Ctrl+V)");
    connect(top, &QToolButton::clicked, this, pasteClipboard);
    auto *bottom = new RibbonButton(RibbonButton::LargeBottom, "Вставить", QIcon(), true);
    auto *menu = new QMenu(bottom);
    connect(menu->addAction(Icons::paste(), "Вставить"), &QAction::triggered, this, pasteClipboard);
    connect(menu->addAction(Icons::fileOpen(), "Вставить из"), &QAction::triggered, this, [this]() {
        QString f = QFileDialog::getOpenFileName(this, "Вставить из", QString(),
                        "Все файлы изображений (*.png *.jpg *.jpeg *.bmp *.gif *.tif *.tiff *.ico)");
        if (f.isEmpty()) return;
        QImage img(f);
        if (img.isNull()) { QMessageBox::warning(this, "Вставить из", "Не удалось открыть файл."); return; }
        m_selectBtn->setChecked(true);
        m_canvas->paste(img);
    });
    bottom->setMenu(menu);
    h->addWidget(makeSplit(top, bottom));
    m_pasteButtons << top << bottom;

    auto *col = new QVBoxLayout;
    col->setSpacing(1);
    auto *cut = new RibbonButton(RibbonButton::Small, "Вырезать", Icons::cut());
    cut->setToolTip("Вырезать (Ctrl+X)");
    connect(cut, &QToolButton::clicked, m_canvas, &Canvas::cutSelection);
    auto *copy = new RibbonButton(RibbonButton::Small, "Копировать", Icons::copy());
    copy->setToolTip("Копировать (Ctrl+C)");
    connect(copy, &QToolButton::clicked, m_canvas, &Canvas::copySelection);
    col->addWidget(cut);
    col->addWidget(copy);
    col->addStretch();
    h->addLayout(col);
    m_selectionDependent << cut << copy;

    return makeGroup("Буфер обмена", box);
}

// «Изображение»: Выделить ▾, Обрезать, Изменить размер, Повернуть ▾
QWidget *MainWindow::makeImageGroup() {
    auto *box = new QWidget;
    auto *h = new QHBoxLayout(box);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(2);

    m_selectBtn = addTool(new RibbonButton(RibbonButton::Large, "", Icons::select()));
    m_selectBtn->setToolTip("Выделить");
    connect(m_selectBtn, &QToolButton::clicked, this, [this]() { m_canvas->setMode(CanvasMode::Select); });

    auto *bottom = new RibbonButton(RibbonButton::LargeBottom, "Выделить", QIcon(), true);
    auto *menu = new QMenu(bottom);
    menu->addSection("Параметры выделения");
    auto *rectAct = menu->addAction(Icons::select(), "Прямоугольная область");
    rectAct->setCheckable(true);
    rectAct->setChecked(true);
    connect(rectAct, &QAction::triggered, this, [this, rectAct]() { rectAct->setChecked(true); m_selectBtn->click(); });
    menu->addSeparator();
    connect(menu->addAction("Выделить все"), &QAction::triggered, this, [this]() {
        m_selectBtn->click();
        m_canvas->selectAll();
    });
    auto *delAct = menu->addAction("Удалить");
    connect(delAct, &QAction::triggered, m_canvas, &Canvas::deleteSelection);
    menu->addSeparator();
    auto *transp = menu->addAction("Прозрачное выделение");
    transp->setCheckable(true);
    connect(transp, &QAction::toggled, m_canvas, &Canvas::setTransparentSelection);
    bottom->setMenu(menu);
    h->addWidget(makeSplit(m_selectBtn, bottom));

    auto *col = new QVBoxLayout;
    col->setSpacing(1);
    auto *crop = new RibbonButton(RibbonButton::Small, "Обрезать", Icons::crop());
    crop->setToolTip("Обрезать (Ctrl+Shift+X)");
    connect(crop, &QToolButton::clicked, m_canvas, &Canvas::crop);
    m_selectionDependent << crop;

    auto *resize = new RibbonButton(RibbonButton::Small, "Изменить размер", Icons::resize());
    resize->setToolTip("Изменить размер и наклонить (Ctrl+W)");
    connect(resize, &QToolButton::clicked, this, &MainWindow::resizeImage);

    auto *rotate = new RibbonButton(RibbonButton::Small, "Повернуть", Icons::rotate(), true);
    auto *rm = new QMenu(rotate);
    connect(rm->addAction(Icons::rotateRight(), "Повернуть на 90° вправо"), &QAction::triggered, this, [this]() { m_canvas->rotate(90); });
    connect(rm->addAction(Icons::rotateLeft(), "Повернуть на 90° влево"), &QAction::triggered, this, [this]() { m_canvas->rotate(-90); });
    connect(rm->addAction(Icons::rotate180(), "Повернуть на 180°"), &QAction::triggered, this, [this]() { m_canvas->rotate(180); });
    connect(rm->addAction(Icons::flipVertical(), "Отразить по вертикали"), &QAction::triggered, this, [this]() { m_canvas->flip(false); });
    connect(rm->addAction(Icons::flipHorizontal(), "Отразить по горизонтали"), &QAction::triggered, this, [this]() { m_canvas->flip(true); });
    rotate->setMenu(rm);

    col->addWidget(crop);
    col->addWidget(resize);
    col->addWidget(rotate);
    col->addStretch();
    h->addLayout(col);
    return makeGroup("Изображение", box);
}

// «Инструменты»: карандаш, заливка, текст / ластик, палитра, масштаб
QWidget *MainWindow::makeToolsGroup() {
    auto *box = new QWidget;
    auto *g = new QGridLayout(box);
    g->setContentsMargins(0, 4, 0, 0);
    g->setSpacing(2);

    auto mk = [&](const QIcon &icon, const QString &tip, int r, int c) {
        auto *b = addTool(new RibbonButton(RibbonButton::Small, "", icon));
        b->setIconSize(QSize(22, 22));
        b->setToolTip(tip);
        g->addWidget(b, r, c);
        return b;
    };
    auto *pencil = mk(Icons::pencil(), "Карандаш", 0, 0);
    connect(pencil, &QToolButton::clicked, this, [this]() { applyTool(std::make_unique<PencilTool>()); });
    auto *fill = mk(Icons::bucket(), "Заливка цветом", 0, 1);
    connect(fill, &QToolButton::clicked, this, [this]() { applyTool(std::make_unique<FillTool>()); });
    m_textBtn = mk(Icons::text(), "Текст", 0, 2);
    connect(m_textBtn, &QToolButton::clicked, this, [this]() { m_canvas->setMode(CanvasMode::Text); });
    auto *eraser = mk(Icons::eraser(), "Ластик", 1, 0);
    connect(eraser, &QToolButton::clicked, this, [this]() { applyTool(std::make_unique<EraserTool>()); });
    auto *picker = mk(Icons::picker(), "Палитра", 1, 1);
    connect(picker, &QToolButton::clicked, this, [this]() { m_canvas->setMode(CanvasMode::Picker); });
    auto *zoom = mk(Icons::magnifier(), "Масштаб", 1, 2);
    connect(zoom, &QToolButton::clicked, this, [this]() { m_canvas->setMode(CanvasMode::Magnifier); });

    return makeGroup("Инструменты", box);
}

// «Кисти»: кнопка + стрелочка с меню выбора из 9 кистей
QWidget *MainWindow::makeBrushesGroup() {
    m_brushBtn = addTool(new RibbonButton(RibbonButton::Large, "", Icons::brush(m_brushStyle)));
    m_brushBtn->setIconSize(QSize(40, 40));
    m_brushBtn->setToolTip(BrushTool::name(m_brushStyle));
    connect(m_brushBtn, &QToolButton::clicked, this, [this]() { selectBrush(m_brushStyle); });

    auto *bottom = new RibbonButton(RibbonButton::LargeBottom, "Кисти", QIcon(), true);
    auto *menu = new QMenu(bottom);
    auto *wa = new QWidgetAction(menu);
    auto *grid = new QWidget;
    grid->setObjectName("popupGrid");
    auto *g = new QGridLayout(grid);
    g->setContentsMargins(4, 4, 4, 4);
    g->setSpacing(2);
    auto *excl = new QButtonGroup(grid);
    const BrushStyle styles[] = {
        BrushStyle::Brush, BrushStyle::Calligraphy1, BrushStyle::Calligraphy2, BrushStyle::Airbrush,
        BrushStyle::Oil, BrushStyle::Crayon, BrushStyle::Marker, BrushStyle::NaturalPencil,
        BrushStyle::Watercolor};
    int i = 0;
    for (BrushStyle s : styles) {
        auto *b = new QToolButton;
        b->setIcon(Icons::brush(s));
        b->setIconSize(QSize(46, 46));
        b->setFixedSize(54, 54);
        b->setToolTip(BrushTool::name(s));
        b->setCheckable(true);
        b->setChecked(s == m_brushStyle);
        excl->addButton(b);
        connect(b, &QToolButton::clicked, this, [this, s, menu]() {
            menu->hide();
            m_brushStyle = s;
            m_brushBtn->setIcon(Icons::brush(s));
            m_brushBtn->setToolTip(BrushTool::name(s));
            m_brushBtn->click();
        });
        g->addWidget(b, i / 4, i % 4);
        ++i;
    }
    wa->setDefaultWidget(grid);
    menu->addAction(wa);
    bottom->setMenu(menu);

    auto *w = makeSplit(m_brushBtn, bottom);
    return makeGroup("", w);
}

// «Фигуры»: 3 видимых ряда, стрелки ▲▼ и кнопка раскрытия всей галереи + «Контур»/«Заливка»
QWidget *MainWindow::makeShapesGroup() {
    auto *box = new QWidget;
    auto *h = new QHBoxLayout(box);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(0);

    auto *gallery = new QFrame;
    gallery->setObjectName("gallery");
    m_galleryGrid = new QGridLayout(gallery);
    m_galleryGrid->setContentsMargins(2, 2, 2, 2);
    m_galleryGrid->setSpacing(1);
    gallery->setFixedSize(2 * 2 + 7 * 22 + 6 + 2, 2 * 2 + 3 * 22 + 2 + 2);

    for (int i = 0; i < ShapeTool::count; ++i) {
        const ShapeType t = ShapeType(i);
        auto *b = addTool(new RibbonButton(RibbonButton::Small, "", Icons::shape(t)));
        b->setIconSize(QSize(18, 18));
        b->setFixedSize(22, 22);
        b->setToolTip(ShapeTool::name(t));
        connect(b, &QToolButton::clicked, this, [this, t]() { selectShape(t); });
        m_shapeBtns << b;
    }

    // столбец со стрелками справа от галереи
    auto *arrows = new QVBoxLayout;
    arrows->setSpacing(0);
    m_galleryUp = new QToolButton;
    m_galleryUp->setArrowType(Qt::UpArrow);
    m_galleryDown = new QToolButton;
    m_galleryDown->setArrowType(Qt::DownArrow);
    auto *more = new QToolButton;
    more->setIcon(Icons::galleryMore());
    more->setToolTip("Все фигуры");
    for (auto *b : {m_galleryUp, m_galleryDown, more}) {
        b->setObjectName("galleryArrow");
        b->setFixedSize(16, 25);
        b->setIconSize(QSize(10, 10));
        b->setFocusPolicy(Qt::NoFocus);
        arrows->addWidget(b);
    }
    arrows->addStretch();
    connect(m_galleryUp, &QToolButton::clicked, this, [this]() { if (m_galleryRow > 0) { --m_galleryRow; refreshGallery(); } });
    connect(m_galleryDown, &QToolButton::clicked, this, [this]() { if (m_galleryRow < 1) { ++m_galleryRow; refreshGallery(); } });

    // раскрывающаяся панель со ВСЕМИ фигурами
    auto *allMenu = new QMenu(more);
    auto *wa = new QWidgetAction(allMenu);
    auto *grid = new QWidget;
    grid->setObjectName("popupGrid");
    auto *g = new QGridLayout(grid);
    g->setContentsMargins(3, 3, 3, 3);
    g->setSpacing(1);
    for (int i = 0; i < ShapeTool::count; ++i) {
        auto *b = new QToolButton;
        b->setIcon(Icons::shape(ShapeType(i)));
        b->setIconSize(QSize(18, 18));
        b->setFixedSize(24, 24);
        b->setToolTip(ShapeTool::name(ShapeType(i)));
        connect(b, &QToolButton::clicked, this, [this, i, allMenu]() {
            allMenu->hide();
            m_galleryRow = std::min(i / 7, 1);
            refreshGallery();
            m_shapeBtns[i]->click();
        });
        g->addWidget(b, i / 7, i % 7);
    }
    wa->setDefaultWidget(grid);
    allMenu->addAction(wa);
    connect(more, &QToolButton::clicked, this, [more, allMenu, gallery]() {
        allMenu->popup(gallery->mapToGlobal(QPoint(0, 0)));
        Q_UNUSED(more);
    });

    h->addWidget(gallery, 0, Qt::AlignTop);
    h->addLayout(arrows);
    h->addSpacing(6);

    // «Контур» и «Заливка»
    auto *opts = new QVBoxLayout;
    opts->setSpacing(1);
    m_outlineBtn = new RibbonButton(RibbonButton::Small, "Контур", Icons::outline(), true);
    auto *om = new QMenu(m_outlineBtn);
    auto *og = new QActionGroup(om);
    struct O { OutlineMode m; const char *t; };
    for (const O &o : {O{OutlineMode::None, "Без контура"}, O{OutlineMode::Solid, "Сплошной цвет"},
                       O{OutlineMode::Marker, "Маркер"}, O{OutlineMode::Watercolor, "Акварель"}}) {
        auto *a = om->addAction(QString::fromUtf8(o.t));
        a->setCheckable(true);
        a->setChecked(o.m == ShapeTool::outline);
        og->addAction(a);
        const OutlineMode m = o.m;
        connect(a, &QAction::triggered, this, [m]() { ShapeTool::outline = m; });
    }
    m_outlineBtn->setMenu(om);

    m_fillBtn = new RibbonButton(RibbonButton::Small, "Заливка", Icons::fill(), true);
    auto *fm = new QMenu(m_fillBtn);
    auto *fg = new QActionGroup(fm);
    struct F { FillMode m; const char *t; };
    for (const F &f : {F{FillMode::None, "Без заливки"}, F{FillMode::Solid, "Сплошной цвет"},
                       F{FillMode::Marker, "Маркер"}, F{FillMode::Watercolor, "Акварель"},
                       F{FillMode::Gradient, "Градиент (цвет 1 → цвет 2)"}}) {
        auto *a = fm->addAction(QString::fromUtf8(f.t));
        a->setCheckable(true);
        a->setChecked(f.m == ShapeTool::fill);
        fg->addAction(a);
        const FillMode m = f.m;
        connect(a, &QAction::triggered, this, [m]() { ShapeTool::fill = m; });
    }
    m_fillBtn->setMenu(fm);

    opts->addWidget(m_outlineBtn);
    opts->addWidget(m_fillBtn);
    opts->addStretch();
    h->addLayout(opts);

    refreshGallery();
    return makeGroup("Фигуры", box);
}

void MainWindow::refreshGallery() {
    for (auto *b : m_shapeBtns) {
        m_galleryGrid->removeWidget(b);
        b->hide();
    }
    const int first = m_galleryRow * 7;
    const int last = std::min(ShapeTool::count, (m_galleryRow + 3) * 7);
    for (int i = first; i < last; ++i) {
        m_galleryGrid->addWidget(m_shapeBtns[i], i / 7 - m_galleryRow, i % 7);
        m_shapeBtns[i]->show();
    }
    m_galleryUp->setEnabled(m_galleryRow > 0);
    m_galleryDown->setEnabled(m_galleryRow < 1);
}

// «Толщина»: кнопка со стрелочкой и меню из 4 линий
QWidget *MainWindow::makeSizeGroup() {
    auto *b = new RibbonButton(RibbonButton::Large, "Толщина", Icons::thickness(), true);
    b->setToolTip("Толщина");
    auto *menu = new QMenu(b);
    auto *wa = new QWidgetAction(menu);
    auto *list = new QWidget;
    list->setObjectName("popupGrid");
    auto *v = new QVBoxLayout(list);
    v->setContentsMargins(3, 3, 3, 3);
    v->setSpacing(2);
    auto *excl = new QButtonGroup(list);
    for (int s : {1, 3, 5, 8}) {
        auto *line = new QToolButton;
        line->setIcon(QIcon(Icons::thicknessLine(s, 24, 170)));
        line->setIconSize(QSize(170, 24));
        line->setFixedSize(180, 40);
        line->setCheckable(true);
        line->setChecked(s == m_toolSize);
        line->setToolTip(QString("%1 пкс").arg(s));
        excl->addButton(line);
        connect(line, &QToolButton::clicked, this, [this, s, menu]() {
            menu->hide();
            setToolSize(s);
        });
        v->addWidget(line);
        m_sizeChoices << line;
    }
    wa->setDefaultWidget(list);
    menu->addAction(wa);
    b->setMenu(menu);
    return makeGroup("", b);
}

// «Цвета»: Цвет 1, Цвет 2, палитра 10x3, «Изменение цветов»
QWidget *MainWindow::makeColorsGroup() {
    auto *box = new QWidget;
    auto *h = new QHBoxLayout(box);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(2);

    auto *c1 = new RibbonButton(RibbonButton::Large, "Цвет\n1", colorIcon(Qt::black));
    c1->setToolTip("Цвет 1 (основной): рисование левой кнопкой мыши");
    auto *c2 = new RibbonButton(RibbonButton::Large, "Цвет\n2", colorIcon(Qt::white));
    c2->setToolTip("Цвет 2 (фон): ластик, заливка фигур, правая кнопка мыши");
    for (auto *c : {c1, c2}) { c->setCheckable(true); c->setFixedWidth(46); }
    connect(c1, &QToolButton::clicked, this, [this]() { setActiveColorSlot(1); });
    connect(c2, &QToolButton::clicked, this, [this]() { setActiveColorSlot(2); });
    m_color1Btns << c1;
    m_color2Btns << c2;
    h->addWidget(c1, 0, Qt::AlignTop);
    h->addWidget(c2, 0, Qt::AlignTop);

    const QStringList colors = {
        "#000000", "#7f7f7f", "#880015", "#ed1c24", "#ff7f27",
        "#fff200", "#22b14c", "#00a2e8", "#3f48cc", "#a349a4",
        "#ffffff", "#c3c3c3", "#b97a57", "#ffaec9", "#ffc90e",
        "#efe4b0", "#b5e61d", "#99d9ea", "#7092be", "#c8bfe7"};

    auto *gridHolder = new QWidget;
    auto *grid = new QGridLayout(gridHolder);
    grid->setContentsMargins(4, 4, 4, 0);
    grid->setSpacing(2);
    for (int i = 0; i < 30; ++i) {
        auto *btn = new QPushButton;
        btn->setFixedSize(20, 20);
        btn->setFocusPolicy(Qt::NoFocus);
        btn->setContextMenuPolicy(Qt::CustomContextMenu);
        if (i < colors.size()) {
            const QColor c(colors[i]);
            btn->setToolTip(c.name());
            btn->setStyleSheet(QString("QPushButton{background:%1; border:1px solid #a0a0a0;}"
                                       "QPushButton:hover{border:1px solid #3c7fb1;}").arg(c.name()));
            connect(btn, &QPushButton::clicked, this, [this, c]() { setColor(c); });
            connect(btn, &QPushButton::customContextMenuRequested, this, [this, c]() {
                m_canvas->setSecondaryColor(c);
                updateColorButtons();
            });
        } else {
            const int idx = i - 20;      // ячейки для своих цветов
            m_customCells << btn;
            connect(btn, &QPushButton::clicked, this, [this, idx]() {
                if (m_customColors[idx].isValid()) setColor(m_customColors[idx]);
            });
            connect(btn, &QPushButton::customContextMenuRequested, this, [this, idx]() {
                if (m_customColors[idx].isValid()) {
                    m_canvas->setSecondaryColor(m_customColors[idx]);
                    updateColorButtons();
                }
            });
        }
        grid->addWidget(btn, i / 10, i % 10);
    }
    h->addWidget(gridHolder, 0, Qt::AlignTop);

    auto *edit = new RibbonButton(RibbonButton::Large, "Изменение\nцветов", Icons::colorEdit());
    edit->setToolTip("Изменение цветов: выбор цвета из палитры");
    connect(edit, &QToolButton::clicked, this, &MainWindow::editColors);
    h->addWidget(edit, 0, Qt::AlignTop);

    refreshCustomColors();
    updateColorButtons();
    return makeGroup("Цвета", box);
}

// ------------------------------------------------ «Вид»

QWidget *MainWindow::createViewTab() {
    QHBoxLayout *view;
    QWidget *page = ribbonPage(&view);

    // Масштаб
    {
        auto *box = new QWidget;
        auto *h = new QHBoxLayout(box);
        h->setContentsMargins(0, 0, 0, 0);
        h->setSpacing(2);
        auto *zin = new RibbonButton(RibbonButton::Large, "Увеличить", Icons::zoomIn());
        connect(zin, &QToolButton::clicked, this, [this]() { zoomStep(+1); });
        auto *zout = new RibbonButton(RibbonButton::Large, "Уменьшить", Icons::zoomOut());
        connect(zout, &QToolButton::clicked, this, [this]() { zoomStep(-1); });
        auto *z100 = new RibbonButton(RibbonButton::Large, "100\n%", Icons::zoom100());
        z100->setToolTip("100%");
        connect(z100, &QToolButton::clicked, this, [this]() { m_zoomSlider->setValue(100); });
        for (auto *b : {zin, zout, z100}) h->addWidget(b, 0, Qt::AlignTop);
        view->addWidget(makeGroup("Масштаб", box));
    }
    view->addWidget(groupSeparator());

    // Показать или скрыть
    {
        auto *box = new QWidget;
        auto *v = new QVBoxLayout(box);
        v->setContentsMargins(6, 2, 6, 0);
        v->setSpacing(4);
        auto *rulers = new QCheckBox("Линейки");
        auto *grid = new QCheckBox("Линии сетки");
        m_statusCheck = new QCheckBox("Строка состояния");
        m_statusCheck->setChecked(true);
        connect(rulers, &QCheckBox::toggled, this, [this](bool on) {
            m_hRuler->setVisible(on);
            m_vRuler->setVisible(on);
            m_rulerCorner->setVisible(on);
            updateRulers();
        });
        connect(grid, &QCheckBox::toggled, m_canvas, &Canvas::setShowGrid);
        connect(m_statusCheck, &QCheckBox::toggled, this, [this](bool on) { statusBar()->setVisible(on); });
        v->addWidget(rulers);
        v->addWidget(grid);
        v->addWidget(m_statusCheck);
        v->addStretch();
        view->addWidget(makeGroup("Показать или скрыть", box));
    }
    view->addWidget(groupSeparator());

    // Показать
    {
        auto *box = new QWidget;
        auto *h = new QHBoxLayout(box);
        h->setContentsMargins(0, 0, 0, 0);
        h->setSpacing(2);
        auto *full = new RibbonButton(RibbonButton::Large, "Во весь\nэкран", Icons::fullScreen());
        full->setToolTip("Во весь экран (F11)");
        connect(full, &QToolButton::clicked, this, &MainWindow::openFullScreen);
        auto *thumb = new RibbonButton(RibbonButton::Large, "Эскиз", Icons::thumbnail());
        thumb->setCheckable(true);
        connect(thumb, &QToolButton::toggled, this, &MainWindow::toggleThumbnail);
        m_thumbBtn = thumb;
        h->addWidget(full, 0, Qt::AlignTop);
        h->addWidget(thumb, 0, Qt::AlignTop);
        view->addWidget(makeGroup("Показать", box));
    }
    view->addStretch();
    return page;
}

// ------------------------------------------------ контекстная вкладка «Текст»

QWidget *MainWindow::createTextTab() {
    QHBoxLayout *lay;
    QWidget *page = ribbonPage(&lay);

    lay->addWidget(makeClipboardGroup());
    lay->addWidget(groupSeparator());

    // Шрифт
    {
        auto *box = new QWidget;
        auto *v = new QVBoxLayout(box);
        v->setContentsMargins(0, 4, 0, 0);
        v->setSpacing(4);
        m_fontCombo = new QFontComboBox;
        m_fontCombo->setFixedWidth(170);
        m_fontCombo->setCurrentFont(QFont("Calibri"));
        v->addWidget(m_fontCombo);

        auto *row = new QHBoxLayout;
        row->setSpacing(2);
        m_fontSize = new QComboBox;
        m_fontSize->setEditable(true);
        for (int s : {8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72})
            m_fontSize->addItem(QString::number(s));
        m_fontSize->setCurrentText("11");
        m_fontSize->setFixedWidth(56);
        row->addWidget(m_fontSize);

        auto fmt = [&](const QString &text, const QString &tip, std::function<void(QFont &)> style) {
            auto *b = new QToolButton;
            b->setObjectName("fmt");
            b->setText(text);
            b->setToolTip(tip);
            b->setCheckable(true);
            b->setFocusPolicy(Qt::NoFocus);
            QFont f = b->font();
            style(f);
            b->setFont(f);
            row->addWidget(b);
            connect(b, &QToolButton::toggled, this, &MainWindow::updateTextFont);
            return b;
        };
        m_boldBtn = fmt("Ж", "Полужирный (Ctrl+B)", [](QFont &f) { f.setBold(true); });
        m_italicBtn = fmt("К", "Курсив (Ctrl+I)", [](QFont &f) { f.setItalic(true); });
        m_underlineBtn = fmt("Ч", "Подчеркнутый (Ctrl+U)", [](QFont &f) { f.setUnderline(true); });
        m_strikeBtn = fmt("abc", "Зачеркнутый", [](QFont &f) { f.setStrikeOut(true); });
        m_boldBtn->setShortcut(QKeySequence("Ctrl+B"));
        m_italicBtn->setShortcut(QKeySequence("Ctrl+I"));
        m_underlineBtn->setShortcut(QKeySequence("Ctrl+U"));
        row->addStretch();
        v->addLayout(row);
        v->addStretch();

        connect(m_fontCombo, &QFontComboBox::currentFontChanged, this, &MainWindow::updateTextFont);
        connect(m_fontSize, &QComboBox::currentTextChanged, this, &MainWindow::updateTextFont);
        lay->addWidget(makeGroup("Шрифт", box));
    }
    lay->addWidget(groupSeparator());

    // Фон
    {
        auto *box = new QWidget;
        auto *v = new QVBoxLayout(box);
        v->setContentsMargins(0, 2, 0, 0);
        v->setSpacing(1);
        auto *opaque = new RibbonButton(RibbonButton::Small, "Непрозрачный", Icons::textOpaque());
        auto *transparent = new RibbonButton(RibbonButton::Small, "Прозрачный", Icons::textTransparent());
        auto *g = new QButtonGroup(box);
        for (auto *b : {opaque, transparent}) {
            b->setCheckable(true);
            g->addButton(b);
            v->addWidget(b);
        }
        transparent->setChecked(true);
        connect(opaque, &QToolButton::toggled, m_canvas, &Canvas::setTextOpaque);
        v->addStretch();
        lay->addWidget(makeGroup("Фон", box));
    }
    lay->addWidget(groupSeparator());
    lay->addWidget(makeColorsGroup());
    lay->addStretch();
    return page;
}

void MainWindow::updateTextFont() {
    if (!m_fontCombo) return;
    QFont f = m_fontCombo->currentFont();
    bool ok = false;
    int pt = m_fontSize->currentText().toInt(&ok);
    f.setPointSize(ok && pt > 0 ? std::min(pt, 500) : 11);
    f.setBold(m_boldBtn->isChecked());
    f.setItalic(m_italicBtn->isChecked());
    f.setUnderline(m_underlineBtn->isChecked());
    f.setStrikeOut(m_strikeBtn->isChecked());
    m_canvas->setTextFont(f);
}

void MainWindow::showTextTab(bool show) {
    const int idx = m_ribbon->indexOf(m_textTab);
    if (show) {
        if (idx < 0) m_ribbon->addTab(m_textTab, "Текст");
        m_ribbon->setCurrentWidget(m_textTab);
        m_contextLabel->show();
        QTimer::singleShot(0, this, &MainWindow::positionContextLabel);
    } else {
        if (idx >= 0) {
            const bool wasCurrent = m_ribbon->currentIndex() == idx;
            m_ribbon->removeTab(idx);
            if (wasCurrent) m_ribbon->setCurrentIndex(0);
        }
        m_contextLabel->hide();
        m_titleLabel->setContentsMargins(0, 0, 0, 0);
    }
}

// зелёная надпись «Инструменты работы с текстом» — точно над вкладкой «Текст»
void MainWindow::positionContextLabel() {
    const int idx = m_ribbon->indexOf(m_textTab);
    if (idx < 0) { m_contextLabel->hide(); m_titleLabel->setContentsMargins(0, 0, 0, 0); return; }
    QTabBar *bar = m_ribbon->tabBar();
    const QRect tr = bar->tabRect(idx);
    const QPoint p = m_header->mapFromGlobal(bar->mapToGlobal(tr.topLeft()));
    const int w = std::max(tr.width(), m_contextLabel->sizeHint().width());
    m_contextLabel->setGeometry(p.x() + tr.width() / 2 - w / 2, 0, w, m_header->height());
    m_contextLabel->show();
    m_contextLabel->raise();
    // заголовок окна сдвигается правее зелёной надписи, как в Paint
    m_titleLabel->setContentsMargins(0, 0, 0, 0);
    const int overlap = m_contextLabel->geometry().right() + 10 - m_titleLabel->x();
    m_titleLabel->setContentsMargins(std::max(0, overlap), 0, 0, 0);
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    if (m_contextLabel && m_contextLabel->isVisible()) positionContextLabel();
}

// ================================================================ рабочая область

QWidget *MainWindow::createWorkspace() {
    auto *w = new QWidget;
    auto *g = new QGridLayout(w);
    g->setContentsMargins(0, 0, 0, 0);
    g->setSpacing(0);

    m_hRuler = new Ruler(Qt::Horizontal);
    m_vRuler = new Ruler(Qt::Vertical);
    m_rulerCorner = new QWidget;
    m_rulerCorner->setFixedSize(20, 20);
    m_rulerCorner->setStyleSheet("background: #f5f6f7;");
    for (QWidget *r : {static_cast<QWidget *>(m_hRuler), static_cast<QWidget *>(m_vRuler), m_rulerCorner})
        r->hide();

    // холст с отступом 5 пикселей, как в Paint
    auto *holder = new QWidget;
    holder->setStyleSheet("background: transparent;");
    auto *hl = new QVBoxLayout(holder);
    hl->setContentsMargins(5, 5, 5, 5);
    hl->setSizeConstraint(QLayout::SetFixedSize);
    hl->addWidget(m_canvas);

    m_scroll = new QScrollArea;
    m_scroll->setWidget(holder);
    m_scroll->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setStyleSheet("QScrollArea, QScrollArea > QWidget > QWidget { background: #cfd8e4; }");

    g->addWidget(m_rulerCorner, 0, 0);
    g->addWidget(m_hRuler, 0, 1);
    g->addWidget(m_vRuler, 1, 0);
    g->addWidget(m_scroll, 1, 1);

    connect(m_scroll->horizontalScrollBar(), &QScrollBar::valueChanged, this, &MainWindow::updateRulers);
    connect(m_scroll->verticalScrollBar(), &QScrollBar::valueChanged, this, &MainWindow::updateRulers);
    return w;
}

void MainWindow::updateRulers() {
    if (!m_hRuler || !m_hRuler->isVisible()) return;
    m_hRuler->setZoom(m_canvas->zoom());
    m_vRuler->setZoom(m_canvas->zoom());
    m_hRuler->setOffset(m_scroll->horizontalScrollBar()->value() - 5);
    m_vRuler->setOffset(m_scroll->verticalScrollBar()->value() - 5);
}

// ================================================================ строка состояния

void MainWindow::createStatusBar() {
    QStatusBar *sb = statusBar();
    sb->setSizeGripEnabled(false);

    auto item = [&](int icon, QLabel *&label, int width) {
        auto *w = new QWidget;
        auto *h = new QHBoxLayout(w);
        h->setContentsMargins(6, 0, 6, 0);
        h->setSpacing(4);
        auto *ic = new QLabel;
        ic->setPixmap(statusIcon(icon));
        label = new QLabel;
        label->setMinimumWidth(width);
        h->addWidget(ic);
        h->addWidget(label);
        sb->addWidget(w);
        auto *sep = new QFrame;
        sep->setFrameShape(QFrame::VLine);
        sep->setStyleSheet("color: #d0d0d0;");
        sb->addWidget(sep);
    };
    item(0, m_coordLabel, 110);
    item(1, m_selLabel, 110);
    item(2, m_sizeLabel, 120);
    item(3, m_fileSizeLabel, 110);
    m_sizeLabel->setText(QString("%1 × %2 пкс").arg(m_canvas->image().width()).arg(m_canvas->image().height()));

    // координаты меняются при КАЖДОМ движении курсора над холстом
    connect(m_canvas, &Canvas::mouseMoved, this, [this](int x, int y) {
        m_coordLabel->setText(QString("%1, %2 пкс").arg(x).arg(y));
        if (m_hRuler->isVisible()) { m_hRuler->setCursorPos(x); m_vRuler->setCursorPos(y); }
    });
    connect(m_canvas, &Canvas::mouseLeft, this, [this]() {
        m_coordLabel->clear();
        m_hRuler->setCursorPos(-1);
        m_vRuler->setCursorPos(-1);
    });

    m_zoomLabel = new QLabel("100%");
    m_zoomLabel->setMinimumWidth(44);
    m_zoomLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    auto *minus = new QToolButton;
    minus->setText("−");
    auto *plus = new QToolButton;
    plus->setText("+");
    m_zoomSlider = new QSlider(Qt::Horizontal);
    m_zoomSlider->setRange(12, 800);
    m_zoomSlider->setValue(100);
    m_zoomSlider->setFixedWidth(140);
    connect(minus, &QToolButton::clicked, this, [this]() { zoomStep(-1); });
    connect(plus, &QToolButton::clicked, this, [this]() { zoomStep(+1); });
    connect(m_zoomSlider, &QSlider::valueChanged, this, &MainWindow::changeZoom);

    sb->addPermanentWidget(m_zoomLabel);
    sb->addPermanentWidget(minus);
    sb->addPermanentWidget(m_zoomSlider);
    sb->addPermanentWidget(plus);
}

void MainWindow::onSelectionChanged(const QRect &rect) {
    const bool has = !rect.isNull();
    for (QWidget *w : m_selectionDependent) w->setEnabled(has);
    m_selLabel->setText(has ? QString("%1 × %2 пкс").arg(rect.width()).arg(rect.height()) : QString());
}

// ================================================================ горячие клавиши

void MainWindow::createShortcuts() {
    auto add = [this](const QKeySequence &key, std::function<void()> fn) {
        auto *a = new QAction(this);
        a->setShortcut(key);
        connect(a, &QAction::triggered, this, fn);
        addAction(a);
    };
    add(QKeySequence::New, [this]() { newFile(); });
    add(QKeySequence::Open, [this]() { openFile(); });
    add(QKeySequence::Save, [this]() { saveFile(); });
    add(QKeySequence(Qt::Key_F12), [this]() { saveFileAs(); });
    add(QKeySequence::Print, [this]() { printImage(); });
    add(QKeySequence::Undo, [this]() { m_canvas->undo(); });
    add(QKeySequence("Ctrl+Y"), [this]() { m_canvas->redo(); });
    add(QKeySequence::Copy, [this]() { m_canvas->copySelection(); });
    add(QKeySequence::Cut, [this]() { m_canvas->cutSelection(); });
    add(QKeySequence::Paste, [this]() {
        QImage img = QApplication::clipboard()->image();
        if (!img.isNull()) { m_selectBtn->setChecked(true); m_canvas->paste(img); }
    });
    add(QKeySequence::SelectAll, [this]() { m_selectBtn->click(); m_canvas->selectAll(); });
    add(QKeySequence::Delete, [this]() { m_canvas->deleteSelection(); });
    add(QKeySequence("Ctrl+Shift+X"), [this]() { m_canvas->crop(); });
    add(QKeySequence("Ctrl+W"), [this]() { resizeImage(); });
    add(QKeySequence("Ctrl+E"), [this]() { showProperties(); });
    add(QKeySequence(Qt::Key_F11), [this]() { openFullScreen(); });
    add(QKeySequence("Ctrl+PgUp"), [this]() { zoomStep(+1); });
    add(QKeySequence("Ctrl+PgDown"), [this]() { zoomStep(-1); });
    add(QKeySequence(Qt::Key_Escape), [this]() { m_canvas->commitAll(); });
}

// ================================================================ инструменты

void MainWindow::applyTool(std::unique_ptr<Tool> tool) {
    tool->setSize(m_toolSize);
    m_canvas->setTool(std::move(tool));
}

void MainWindow::selectBrush(BrushStyle style) { applyTool(std::make_unique<BrushTool>(style)); }
void MainWindow::selectShape(ShapeType type)   { applyTool(std::make_unique<ShapeTool>(type)); }

void MainWindow::setToolSize(int size) {
    m_toolSize = size;
    m_canvas->setToolSize(size);
}

// ================================================================ цвета

void MainWindow::updateColorButtons() {
    for (auto *b : m_color1Btns) b->setIcon(colorIcon(m_canvas->primaryColor()));
    for (auto *b : m_color2Btns) b->setIcon(colorIcon(m_canvas->secondaryColor()));
}

void MainWindow::setActiveColorSlot(int slot) {
    m_activeColor = slot;
    for (auto *b : m_color1Btns) b->setChecked(slot == 1);
    for (auto *b : m_color2Btns) b->setChecked(slot == 2);
}

void MainWindow::refreshCustomColors() {
    for (int i = 0; i < m_customCells.size(); ++i) {
        const QColor c = m_customColors[i % 10];
        m_customCells[i]->setStyleSheet(c.isValid()
            ? QString("QPushButton{background:%1; border:1px solid #a0a0a0;}"
                      "QPushButton:hover{border:1px solid #3c7fb1;}").arg(c.name())
            : QString("QPushButton{background:#ffffff; border:1px solid #d4d4d4;}"));
        m_customCells[i]->setToolTip(c.isValid() ? c.name() : QString());
    }
}

void MainWindow::setColor(const QColor &color) {
    if (m_activeColor == 1) m_canvas->setPrimaryColor(color);
    else m_canvas->setSecondaryColor(color);
    updateColorButtons();
}

void MainWindow::editColors() {
    const QColor start = m_activeColor == 1 ? m_canvas->primaryColor() : m_canvas->secondaryColor();
    const QColor c = QColorDialog::getColor(start, this, "Изменение палитры");
    if (!c.isValid()) return;
    // новый цвет попадает в нижний ряд палитры, как в Paint
    m_customColors.prepend(c);
    m_customColors.resize(10);
    refreshCustomColors();
    setColor(c);
}

// ================================================================ масштаб

void MainWindow::changeZoom(int value) {
    m_canvas->setZoom(value / 100.0f);
    m_zoomLabel->setText(QString("%1%").arg(value));
    updateRulers();
}

void MainWindow::zoomStep(int direction) {
    const int cur = m_zoomSlider->value();
    if (direction > 0) {
        for (double z : kZoomLevels) if (z > cur + 0.5) { m_zoomSlider->setValue(int(z)); return; }
    } else {
        for (int i = int(std::size(kZoomLevels)) - 1; i >= 0; --i)
            if (kZoomLevels[i] < cur - 0.5) { m_zoomSlider->setValue(int(kZoomLevels[i])); return; }
    }
}

void MainWindow::openFullScreen() {
    m_canvas->commitAll();
    auto *v = new FullScreenView(m_canvas->image());
    v->showFullScreen();
}

void MainWindow::toggleThumbnail(bool on) {
    if (!m_thumb) {
        auto *t = new ThumbWindow(this);
        t->onClose = [this]() { m_thumbBtn->setChecked(false); };
        m_thumb = t;
    }
    if (on) {
        updateThumbnail();
        m_thumb->move(mapToGlobal(QPoint(width() - 300, height() - 260)));
        m_thumb->show();
    } else {
        m_thumb->hide();
    }
}

void MainWindow::updateThumbnail() {
    if (!m_thumb || !m_thumb->isVisible()) return;
    m_thumb->setPixmap(QPixmap::fromImage(m_canvas->image().scaled(
        m_thumb->size() - QSize(10, 10), Qt::KeepAspectRatio, Qt::SmoothTransformation)));
}

// ================================================================ изображение

void MainWindow::resizeImage() {
    const QSize base = m_canvas->hasSelection() ? m_canvas->selectionRect().size() : m_canvas->image().size();
    ResizeDialog dlg(base, this);
    if (dlg.exec() != QDialog::Accepted) return;
    m_canvas->scaleAndSkew(dlg.newSize(), dlg.skewX(), dlg.skewY());
}

// ================================================================ файл

void MainWindow::showFileMenu() {
    m_fileMenu->setRecentFiles(recentFiles());
    const QPoint pos = m_fileBtn->mapToGlobal(QPoint(0, m_fileBtn->height()));
    m_fileMenu->popup(pos, std::min(560, height() - 60));
}

void MainWindow::updateTitle() {
    const QString name = m_fileName.isEmpty() ? "Безымянный" : QFileInfo(m_fileName).fileName();
    const QString title = name + " - Paint";
    setWindowTitle(title);
    m_titleLabel->setText(title);
    if (!m_fileName.isEmpty() && QFileInfo::exists(m_fileName))
        m_fileSizeLabel->setText("Размер: " + QLocale().formattedDataSize(QFileInfo(m_fileName).size()));
    else
        m_fileSizeLabel->clear();
}

bool MainWindow::maybeSave() {
    m_canvas->commitAll();
    if (!m_modified) return true;
    const QString name = m_fileName.isEmpty() ? "Безымянный" : QFileInfo(m_fileName).fileName();
    QMessageBox box(QMessageBox::Question, "Paint",
                    QString("Сохранить изменения в \"%1\"?").arg(name), QMessageBox::NoButton, this);
    auto *save = box.addButton("Сохранить", QMessageBox::AcceptRole);
    auto *discard = box.addButton("Не сохранять", QMessageBox::DestructiveRole);
    box.addButton("Отмена", QMessageBox::RejectRole);
    box.setDefaultButton(save);
    box.exec();
    if (box.clickedButton() == save) return saveFile();
    return box.clickedButton() == discard;
}

void MainWindow::newFile() {
    if (!maybeSave()) return;
    m_canvas->newImage(QSize(1920, 1080));
    m_fileName.clear();
    m_modified = false;
    m_savedAt = QDateTime();
    updateTitle();
}

void MainWindow::openFile() {
    if (!maybeSave()) return;
    const QString f = QFileDialog::getOpenFileName(this, "Открыть", QFileInfo(m_fileName).path(),
        "Все файлы изображений (*.png *.jpg *.jpeg *.bmp *.gif *.tif *.tiff *.ico);;"
        "PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp);;GIF (*.gif);;Все файлы (*)");
    if (!f.isEmpty()) loadFile(f);
}

bool MainWindow::loadFile(const QString &path) {
    QImage img(path);
    if (img.isNull()) {
        QMessageBox::warning(this, "Paint", QString("Не удалось открыть файл \"%1\".").arg(path));
        return false;
    }
    m_canvas->loadImage(img);
    m_fileName = path;
    m_modified = false;
    m_savedAt = QFileInfo(path).lastModified();
    addRecent(path);
    updateTitle();
    return true;
}

bool MainWindow::writeFile(const QString &path) {
    if (!m_canvas->saveImage(path)) {
        QMessageBox::warning(this, "Paint", QString("Не удалось сохранить файл \"%1\".").arg(path));
        return false;
    }
    m_fileName = path;
    m_modified = false;
    m_savedAt = QDateTime::currentDateTime();
    addRecent(path);
    updateTitle();
    return true;
}

bool MainWindow::saveFile() {
    if (m_fileName.isEmpty()) return saveFileAs();
    return writeFile(m_fileName);
}

bool MainWindow::saveFileAs(const QString &format) {
    struct Fmt { const char *ext, *filter; };
    const Fmt fmts[] = {{"png", "PNG (*.png)"}, {"jpg", "JPEG (*.jpg *.jpeg)"},
                        {"bmp", "Точечный рисунок (*.bmp)"}, {"gif", "GIF (*.gif)"},
                        {"tif", "TIFF (*.tif *.tiff)"}};
    QStringList filters;
    QString selected = "PNG (*.png)";
    for (const Fmt &f : fmts) {
        filters << QString::fromUtf8(f.filter);
        if (format == f.ext) selected = QString::fromUtf8(f.filter);
    }
    const QString base = m_fileName.isEmpty()
        ? QDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).filePath("Безымянный")
        : QFileInfo(m_fileName).path() + "/" + QFileInfo(m_fileName).completeBaseName();
    const QString ext = format.isEmpty() ? "png" : format;
    QString f = QFileDialog::getSaveFileName(this, "Сохранить как", base + "." + ext,
                                             filters.join(";;"), &selected);
    if (f.isEmpty()) return false;
    if (QFileInfo(f).suffix().isEmpty()) {
        const int a = selected.indexOf("*.") + 2;
        f += "." + selected.mid(a, selected.indexOf(QRegularExpression("[ )]"), a) - a);
    }
    return writeFile(f);
}

void MainWindow::addRecent(const QString &path) {
    QSettings s("PaintEditor", "PaintEditor");
    QStringList list = s.value("recentFiles").toStringList();
    list.removeAll(path);
    list.prepend(path);
    while (list.size() > 8) list.removeLast();
    s.setValue("recentFiles", list);
}

QStringList MainWindow::recentFiles() const {
    QStringList list = QSettings("PaintEditor", "PaintEditor").value("recentFiles").toStringList();
    list.erase(std::remove_if(list.begin(), list.end(),
                              [](const QString &p) { return !QFileInfo::exists(p); }), list.end());
    return list;
}

static void paintToPrinter(QPrinter *printer, const QImage &img) {
    QPainter p(printer);
    const QRect page = p.viewport();
    QSize s = img.size();
    // печатаем в «естественном» размере (96 т/д), уменьшая, если не помещается
    const double scale = printer->resolution() / 96.0;
    s = QSize(int(s.width() * scale), int(s.height() * scale));
    if (s.width() > page.width() || s.height() > page.height()) s.scale(page.size(), Qt::KeepAspectRatio);
    p.drawImage(QRect(page.topLeft(), s), img);
}

void MainWindow::printImage() {
    m_canvas->commitAll();
    QPrintDialog dlg(&sharedPrinter(), this);
    dlg.setWindowTitle("Печать");
    if (dlg.exec() == QDialog::Accepted) paintToPrinter(&sharedPrinter(), m_canvas->image());
}

void MainWindow::pageSetup() {
    QPageSetupDialog dlg(&sharedPrinter(), this);
    dlg.exec();
}

void MainWindow::printPreview() {
    m_canvas->commitAll();
    QPrintPreviewDialog dlg(&sharedPrinter(), this);
    dlg.setWindowTitle("Предварительный просмотр");
    connect(&dlg, &QPrintPreviewDialog::paintRequested, this, [this](QPrinter *pr) {
        paintToPrinter(pr, m_canvas->image());
    });
    dlg.exec();
}

void MainWindow::sendEmail() {
    m_canvas->commitAll();
    // почтовая ссылка не умеет прикладывать файлы, поэтому сохраняем копию и показываем путь
    const QString path = QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
                             .filePath((m_fileName.isEmpty() ? "Безымянный" : QFileInfo(m_fileName).completeBaseName()) + ".png");
    m_canvas->image().save(path);
    QDesktopServices::openUrl(QUrl("mailto:?subject=" + QUrl::toPercentEncoding("Рисунок")));
    QMessageBox::information(this, "Отправить по электронной почте",
                             "Изображение сохранено для вложения в письмо:\n" + path);
}

void MainWindow::showProperties() {
    PropertiesDialog dlg(m_canvas->image().size(), m_fileName, m_savedAt, this);
    if (dlg.exec() != QDialog::Accepted) return;
    if (dlg.newSize() != m_canvas->image().size()) m_canvas->resizeCanvas(dlg.newSize());
    if (dlg.blackAndWhite()) m_canvas->toBlackAndWhite();
}

void MainWindow::showAbout() {
    QMessageBox::about(this, "О программе",
        "<b>Paint</b> — растровый графический редактор<br>"
        "Курсовая работа, C++ / Qt " QT_VERSION_STR "<br><br>"
        "Кисти, фигуры, заливка, текст, выделение, буфер обмена,<br>"
        "масштаб, линейки, сетка, печать.");
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (maybeSave()) event->accept();
    else event->ignore();
}
