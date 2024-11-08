#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QPushButton>
#include <QToolButton>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QApplication>

#include "basesearchedit.h"
// 主题sdk关于,搜索框
#include "kaboutdialog.h"
#include "ksearchlineedit.h"
using namespace kdk;
class TitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit TitleBar(QWidget *parent = nullptr);
    ~TitleBar();

    // 初始化控件
    void init();
    void initMenu();
    void initLayout();
    void initAbout();
    void initHelp();
    void conn();
    bool eventFilter(QObject *watch,QEvent *e) override;
    void SearchFocusOut();
    void initWidgetStyle();
     KSearchLineEdit *m_searchArea = nullptr;   /* 搜索字体文本框 */

protected:
    void keyPressEvent(QKeyEvent *event) override;             /* 键盘响应事件 */
    void mouseDoubleClickEvent(QMouseEvent *event) override;   /* 鼠标双击标题栏 */

private:
    /* 标题栏控件 */
    QPushButton *m_addFontBtn = nullptr;      /* 添加字体按钮 */
    // BaseSearchEdit *m_searchArea = nullptr;   /* 搜索字体文本框 */
    //KSearchLineEdit *m_searchArea = nullptr;   /* 搜索字体文本框 */
    QToolButton *m_menuBtn = nullptr;         /* 菜单按钮 */
    QMenu *m_menu = nullptr;
    QList<QAction*> m_actions;
    QPushButton *m_minBtn = nullptr;          /* 最小化 */
    QPushButton *m_maxBtn = nullptr;          /* 最大化 */
    QPushButton *m_closeBtn = nullptr;        /* 关闭 */
    QHBoxLayout *m_hlayout = nullptr;         /* 布局 */
    
    kdk::KAboutDialog *m_aboutWindow = nullptr; // 关于界面

    QString appName        = "FontViewer"; //格式lingmo-usb-creator
    QString appShowingName = tr("Font Viewer"); //格式lingmo usb creator ,用来在前端展示
    QString appVersion     = qApp->applicationVersion();

signals:
    void sigTitleSearch(QString search);
    void sigAddFont();

private slots:
    void slotAddFont(void);               /* 添加字体 */
    void slotMaxWid(void);                /* 最大化 */
    void slotMinWid(void);                /* 最小化 */
    void slotCloseWid(void);              /* 关闭 */
    void slotTriggerMenu(QAction *act);   /* 菜单 */
    void slotTitleSearch(QString search);

public slots:
    void slotWindowState(QString state);   /* 窗体状态 */
    void slotChangeFontName();             /* 改变字体名称 */

    void slotChangeFaltSize();
    void slotChangePCSize();

};

#endif // TITLEBAR_H
