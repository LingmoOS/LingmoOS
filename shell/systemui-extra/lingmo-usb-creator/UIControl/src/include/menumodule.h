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
#ifndef MENUMODULE_H
#define MENUMODULE_H
//#define FITTHEMEWINDOW "org.lingmo.style"
#define APPDATA "org.lingmo-usb-creator-data.settings"
#define LINGMO_STYLE "org.lingmo.style"
#define LINGMO_FONT_SIZE "systemFontSize"
#include <QObject>
#include <QWidget>
#include <QMenu>
#include <QPushButton>
#include <QKeyEvent>
#include <QToolButton>
#include <QDebug>
#include <QString>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDesktopServices>
#include <QRect>
#include <QUrl>
#include <QScreen>
#include <QGSettings>
#include <QMetaEnum>
#include <QTextEdit>
#include "daemonipcdbus.h"
class menuModule : public QWidget
{
    Q_OBJECT
public:
    explicit menuModule(QWidget *);
    bool isWayland=false;
    void themeUpdate();
    void fontSizeInit();
    void initsuccessbackdilog();
    void limitTextEditTextCount(QTextEdit* text_edit,const int& limit_count);
    void setThemeDark();
    bool m_isDarktheme;
    bool isDarkTheme() {
        return m_isDarktheme;
    }

    QWidget *aboutWindow = nullptr;
    QWidget *sucbackWindow = nullptr;

signals:
    void menuModuleClose();
    void menuModuleSetThemeStyle(QString);
    void pullupHelp();
    void iconchange();
    void setFontSize(int);
    void clickabout();
public:
    QToolButton *menuButton = nullptr;

public:
//    程序在实例化的时候需要传递的信息字段,打开debug开关后这些字段会被自动填充
    QString appName = "tools/lingmo-usb-creator"; //格式lingmo-usb-creator
    QString appVersion = "1.2.0";
    QString appDesc = "appDesc字段未填充!";
    QString iconPath = ":/data/lingmo-usb-creator.png";
    QString confPath = "org.lingmo-usb-creator-data.settings";

private:
    QMenu *m_menu = nullptr;
    QMenu *themeMenu = nullptr;
    QAction *actionHelp;
    QAction *actionAbout;
    QAction *actionTheme;
    QAction *actionsetting;
    QAction *actionQuit;
    QAction *autoTheme;
    QAction *lightTheme;
    QAction *darkTheme;
    QAction *checkupdatehelp;
    QAction *successbackhelp;
    QAction *officewebsitehelp;
    QSize iconSize;
    QString appPath = "tools/lingmo-usb-creator"; //拉起帮助菜单时使用appName字段
    QGSettings *m_pGsettingThemeData = nullptr;
    QGSettings *m_pGsettingThemeStatus = nullptr;
    enum typeThemeStatus {
        themeAuto = 0,
        themeBlackOnly = 1,
        themeLightOnly = 2
        } themeStatus;

public slots:
    void dealSystemGsettingChange(const QString);
    void dealSetFontSize(double size);
    void successbackAction();
    void slot_limitTextEditTextCount();
private:
    void keyPressEvent(QKeyEvent *event);
    void init();
    QHBoxLayout* initTitleBar(); //关于窗口标题栏初始化
    QVBoxLayout* initBody();    // 关于窗口body初始化
    QHBoxLayout* initTitlesucbackBar();
    QVBoxLayout* initsucebody();
    void initGsetting();
    void initAction();
    void setStyle();
    void triggerMenu(QAction* act); //主菜单动作4
    void triggerThemeMenu(QAction* act); //主题动作
    void triggerHelpMenu(QAction* act);
    void aboutAction();
    void initAbout(); //关于窗口初始化
    void helpAction();
    void setThemeFromLocalThemeSetting(QList<QAction* >); //获取本地主题配置
    void setStyleByThemeGsetting(); //通过外部主题配置设置主题
    void setThemeStyle();
    void setThemeLight();
    void uiModeChange(bool);
    DaemonIpcDbus *ipcDbus = nullptr;
//    void setThemeDark();
    QLabel* titleText = nullptr;
    QLabel* bodyAppName = nullptr;
    QLabel* bodyAppVersion = nullptr;
    QLabel* bodyAppDesc = nullptr;
    QLabel* bodySupport = nullptr;
    QLabel* bodyIcon = nullptr;
    QLabel* titleIcon = nullptr;
    QTextEdit * textedit = nullptr;
    QLabel *toollabel = nullptr;
    QPushButton *submitbtn = nullptr;
    QWidget * body = nullptr;
    bool m_isTabletMode;
//    QPushButton* bodyIcon = nullptr;
//    QPushButton* titleIcon = nullptr;
    void refreshThemeBySystemConf();    //通过系统配置更改主题
};

#endif // MENUMODULE_H
