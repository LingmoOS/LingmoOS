/*
 * Copyright (C) 2002  KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H
//#define FITTHEMEWINDOW "org.lingmo.style"
#define APPDATA "org.lingmo-usb-creator-data.settings"
#define LINGMO_STYLE "org.lingmo.style"
#define LINGMO_FONT_SIZE "systemFontSize"
#define WINDOWW 680 //窗口宽度
#define WINDOWH 510//窗口高度
#define TITLEH 40//标题栏高度
#define SHADOW 6//阴影宽度
#define WIDGETRADIUS 6//窗口圆角
#define SHADOWALPHA 0.16//阴影透明度

#include "page1.h"
#include "page2.h"
#include <QWidget>
#include <QTimer>
#include <QSystemTrayIcon> //状态栏
#include <QGSettings>
#include <QMenu>
#include <QMimeData>
#include <QPushButton>
#include <QWidget>
#include <QMessageBox>
#include <QStackedWidget>
#include <QGraphicsDropShadowEffect>
#include <QKeyEvent>
#include <QDesktopWidget>
#include "include/daemonipcdbus.h" //拉起帮助手册
#include "include/menumodule.h"
#include "include/GSettingSubject.h"
#include "bottompointlabel.h"
#include "include/menufactory.h"
#include "include/aboutdialog.h"
#include <QDBusConnection>
#include <QDBusInterface>
enum {
    PAGE_NONE,
    PAGE_ONE,
    PAGE_TWO,
    PAGE_THREE
}pageIndex;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    bool isWayland=false;
    bool m_isDarkTheme;
    GsettingSubject *subject;
    QString usbpath;
    QString isopath;
    MainWindow(QWidget *parent = nullptr,QString isoPath = "--");
    ~MainWindow();

signals:
    void setMakeStart();
    void dealWrongPasswd();
    void setFontSize(double);
    void setIsoPath(const QString& iso_path);
public slots:
    void makeStart(); //点击授权按钮后由页面1跳转到页面2
    void makeFinish();
    void returnMain();
    void handleIconClickedSub(const QString&);
    void slotActiveWindow(const QString&);
    void setThemeStyle(QString);
    void dealMenuModulePullupHelp();
    void doubleCheck(); //制作中途直接点×需要确认
    void dealAuthorityStatus(QString);
    void dealSetFontSize(double);
   // void settingStyle();
    void closemain();
    void setIconchange();
    void showaboutslot();
    //void uiModeChange(bool isTabletMode);


private:
    QDBusInterface *mStatusSettings = nullptr;
    bool m_isTabletMode;//判断是否是平板模式
    QToolButton *menuButton = nullptr;
    QMenu *m_menu;

    // 标题栏资源
    QWidget *title = nullptr;
    QWidget *about = nullptr;
    QLabel *titleIcon = nullptr;
   // QPushButton *titleIcon = nullptr;
    QLabel *titleText = nullptr;
    QPushButton *titleMin = nullptr;
    QPushButton *titleClose = nullptr;
    QPushButton *titleMenu = nullptr;
    QMenu *Menu = nullptr;
    QAction *actionTheme = nullptr;
    QAction *actionHelp = nullptr;
    QAction *actionAbout = nullptr;
    QAction *actionQuit = nullptr;
    MenuFactory *menufacory;
     aboutdialog * aboutWindow;

    void keyPressEvent(QKeyEvent* event);
    void dragEnterEvent(QDragEnterEvent*);
    void dragLeaveEvent(QDragLeaveEvent*);
    //void eventFilter(QObject*, QEvent *);
    void dropEvent(QDropEvent*);
    
    void init(); //初始化mainwindow相关的设置
    int changePage();
    void myStyle();//设定样式
    void createTrayActions();
    void initGsetting();
    void statusbarInit();
    void setThemeDark();
    void setThemeLight();
    void aboutClick();
    void fontSizeInit();
    QWidget *bottomWidget = nullptr;
    menuModule *menu = nullptr;
    QTimer *dbustimer = nullptr;
    QStackedWidget *stackedWidget= nullptr;
    bottompointlabel *pointLable1 = nullptr;  //页面小圆点
    bottompointlabel *pointLable2 = nullptr;
    bottompointlabel *pointLable3 = nullptr;
    Page1 *page1=nullptr;
    Page2 *page2=nullptr;
    DaemonIpcDbus *m_DaemonIpcDbus = nullptr;   //拉起帮助手册
    QSystemTrayIcon *m_mainTray = nullptr;
    QTimer *timer;
    QGSettings *m_pGsettingThemeData = nullptr;
    QGSettings *m_pGsettingAppData  = nullptr;
    QString  m_windowId = "";
};
#endif // MAINWINDOW_H
