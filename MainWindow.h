#pragma once
#include <QMainWindow>
#include <QColor>
#include <QIcon>
#include <QDateTime>
#include <QVector>
#include <QList>
#include <memory>
#include "ShapeTool.h"
#include "BrushTool.h"

class Canvas;
class Ruler;
class FileMenu;
class RibbonButton;
class QLabel;
class QSlider;
class QToolButton;
class QPushButton;
class QTabWidget;
class QWidget;
class QGridLayout;
class QButtonGroup;
class QAbstractButton;
class QFontComboBox;
class QComboBox;
class QScrollArea;
class QCheckBox;
class QMenu;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    // Файл
    void newFile();
    void openFile();
    bool saveFile();
    bool saveFileAs(const QString &format = QString());
    void printImage();
    void pageSetup();
    void printPreview();
    void sendEmail();
    void showProperties();
    void showAbout();
    // Изображение
    void resizeImage();
    // Масштаб
    void changeZoom(int value);
    void zoomStep(int direction);
    // Цвета
    void setColor(const QColor &color);
    void editColors();

private:
    // --- построение интерфейса
    QWidget *createQuickAccessBar();
    QTabWidget *createRibbon();
    QWidget *createHomeTab();
    QWidget *createViewTab();
    QWidget *createTextTab();
    QWidget *createWorkspace();
    void createStatusBar();
    void createShortcuts();

    QWidget *makeGroup(const QString &title, QWidget *content);
    QWidget *makeClipboardGroup();
    QWidget *makeColorsGroup();
    QWidget *makeImageGroup();
    QWidget *makeToolsGroup();
    QWidget *makeBrushesGroup();
    QWidget *makeShapesGroup();
    QWidget *makeSizeGroup();
    RibbonButton *addTool(RibbonButton *b);
    void refreshGallery();

    // --- логика
    void applyTool(std::unique_ptr<Tool> tool);
    void selectShape(ShapeType type);
    void selectBrush(BrushStyle style);
    void setToolSize(int size);
    void updateColorButtons();
    void refreshCustomColors();
    void setActiveColorSlot(int slot);
    void updateTextFont();
    void showTextTab(bool show);
    void positionContextLabel();
    void onSelectionChanged(const QRect &rect);
    void updateTitle();
    bool maybeSave();
    bool loadFile(const QString &path);
    bool writeFile(const QString &path);
    void addRecent(const QString &path);
    QStringList recentFiles() const;
    void showFileMenu();
    void openFullScreen();
    void toggleThumbnail(bool on);
    void updateThumbnail();
    void updateRulers();

    Canvas *m_canvas = nullptr;
    QTabWidget *m_ribbon = nullptr;
    QWidget *m_textTab = nullptr;
    QLabel *m_contextLabel = nullptr;
    QLabel *m_titleLabel = nullptr;
    QWidget *m_header = nullptr;
    FileMenu *m_fileMenu = nullptr;
    QToolButton *m_fileBtn = nullptr;
    QScrollArea *m_scroll = nullptr;
    Ruler *m_hRuler = nullptr;
    Ruler *m_vRuler = nullptr;
    QWidget *m_rulerCorner = nullptr;
    QLabel *m_thumb = nullptr;
    QAbstractButton *m_thumbBtn = nullptr;

    // инструменты
    QButtonGroup *m_toolGroup = nullptr;
    QAbstractButton *m_prevToolBtn = nullptr;   // для возврата после «Палитры»
    RibbonButton *m_brushBtn = nullptr;
    RibbonButton *m_selectBtn = nullptr;
    RibbonButton *m_textBtn = nullptr;
    QList<QAbstractButton *> m_brushChoices;
    QList<QAbstractButton *> m_sizeChoices;
    BrushStyle m_brushStyle = BrushStyle::Brush;
    int m_toolSize = 3;

    // галерея фигур
    QVector<RibbonButton *> m_shapeBtns;
    QGridLayout *m_galleryGrid = nullptr;
    int m_galleryRow = 0;
    QToolButton *m_galleryUp = nullptr;
    QToolButton *m_galleryDown = nullptr;
    RibbonButton *m_outlineBtn = nullptr;
    RibbonButton *m_fillBtn = nullptr;

    // кнопки, состояние которых зависит от выделения / буфера / истории
    QList<QWidget *> m_selectionDependent;
    QList<QWidget *> m_pasteButtons;
    QToolButton *m_undoBtn = nullptr;
    QToolButton *m_redoBtn = nullptr;

    // цвета (группа «Цвета» есть на двух вкладках)
    QList<RibbonButton *> m_color1Btns, m_color2Btns;
    QList<QPushButton *> m_customCells;
    QVector<QColor> m_customColors;
    int m_activeColor = 1;

    // текст
    QFontComboBox *m_fontCombo = nullptr;
    QComboBox *m_fontSize = nullptr;
    QToolButton *m_boldBtn = nullptr, *m_italicBtn = nullptr, *m_underlineBtn = nullptr, *m_strikeBtn = nullptr;

    // строка состояния
    QLabel *m_coordLabel = nullptr;
    QLabel *m_selLabel = nullptr;
    QLabel *m_sizeLabel = nullptr;
    QLabel *m_fileSizeLabel = nullptr;
    QLabel *m_zoomLabel = nullptr;
    QSlider *m_zoomSlider = nullptr;
    QCheckBox *m_statusCheck = nullptr;

    // документ
    QString m_fileName;
    bool m_modified = false;
    QDateTime m_savedAt;
};
