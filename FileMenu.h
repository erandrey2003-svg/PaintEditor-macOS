#pragma once
#include <QFrame>
#include <QStringList>

class QStackedWidget;
class QVBoxLayout;
class QToolButton;

// Меню «Файл» как в Paint: слева команды, справа «Последние изображения»
// (или подменю «Сохранить как» / «Печать» при наведении на эти пункты)
class FileMenu : public QFrame {
    Q_OBJECT
public:
    explicit FileMenu(QWidget *parent = nullptr);
    void setRecentFiles(const QStringList &files);
    void popup(const QPoint &globalPos, int height);

signals:
    void newFile();
    void openFile();
    void save();
    void saveAs(const QString &format);   // "png", "jpg", "bmp", "gif" или "" — другие форматы
    void print();
    void pageSetup();
    void printPreview();
    void sendEmail();
    void properties();
    void about();
    void exitApp();
    void openRecent(const QString &path);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QToolButton *addItem(QVBoxLayout *lay, const QIcon &icon, const QString &text,
                         bool submenu = false, bool enabled = true);
    QWidget *makePage(const QString &title, QVBoxLayout **body);
    void addSeparator(QVBoxLayout *lay);

    QStackedWidget *m_pages = nullptr;
    QVBoxLayout *m_recentLay = nullptr;
    QToolButton *m_saveAsBtn = nullptr;
    QToolButton *m_printBtn = nullptr;
    QToolButton *m_desktopBtn = nullptr;
};
