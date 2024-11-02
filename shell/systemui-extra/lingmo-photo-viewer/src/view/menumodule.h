#ifndef MENUMODULE_H
#define MENUMODULE_H
#define DEBUG_MENUMODULE 0 // DEBUG模式开关，代码上线之前务必关掉
//#define APPDATA "org.lingmo-photo-viewer-data.settings"   0301-cancel
#include <QObject>
#include <QWidget>
#include <QMenu>
#include <QPushButton>
#include <QToolButton>
#include <QDebug>
#include <QString>
#include <QLabel>
#include <QVBoxLayout>
#include <QUrl>
#include <QHBoxLayout>
#include <QDesktopServices>
#include <QRect>
#include <QScreen>
#include <QGSettings>
#include <QMetaEnum>
#include <QApplication>
#include <QFont>
//#include "titlebar.h"
//#include "kyview.h"
#include "controller/interaction.h"
#include <user_manual.hpp>
class menuModule : public QWidget
{
    Q_OBJECT
public:
    explicit menuModule(QWidget *);
    ~menuModule();
    void setThemeLight();
    void setThemeDark();
    void setMenuBtnSize(QSize btnSize);

Q_SIGNALS:
    void menuModuleClose();      //从标题栏点击退出的信号
    void openSignal();           //从标题栏进行打开图片的信号
    void aboutShow();            //打开关于，主界面两栏的显示情况
    void setFontSizeSignal(int); //字体变化
    void exitAppFromMenu();

public:
    QToolButton *menuButton = nullptr;
    QMenu *m_menu = nullptr;

private:
    QString m_appShowingName;
    QString m_appVersion;
    QString m_appName;
    QString m_appDesc = "appDesc字段未填充!";
    QString m_iconPath;
    QString m_confPath = "org.china-weather-data.settings";
    QMenu *m_themeMenu = nullptr;
    QDialog *m_aboutWindow = nullptr;
    QGSettings *m_pGsettingThemeData = nullptr;
    QGSettings *m_pGsettingThemeStatus = nullptr;
    enum typeThemeStatus { themeAuto = 0, themeBlackOnly = 1, themeLightOnly = 2 } themeStatus;
    QFont m_ft; //固定字号
    QGSettings *m_fontSize = nullptr;

private:
    void init();
    QHBoxLayout *initTitleBar(); //关于窗口标题栏初始化
    QVBoxLayout *initBody();     // 关于窗口body初始化
    void initGsetting();
    void initAction();
    void triggerMenu(QAction *act);      //主菜单动作4
    void triggerThemeMenu(QAction *act); //主题动作
    void aboutAction();
    void initAbout(); //关于窗口初始化
    void helpAction();
    void setThemeFromLocalThemeSetting(QList<QAction *>); //获取本地主题配置
    void refreshThemeBySystemConf();                      //通过系统配置更改主题

    QLabel *m_titleText = nullptr;
    QLabel *m_bodyAppVersion = nullptr;
    QLabel *m_bodySupport = nullptr;
    QLabel *m_bodyAppName = nullptr;
    QLabel *m_bodyAppDescribe = nullptr;
};

#endif // MENUMODULE_H
