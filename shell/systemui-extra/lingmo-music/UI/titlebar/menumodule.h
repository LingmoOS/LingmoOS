#ifndef MENUMODULE_H
#define MENUMODULE_H
#define DEBUG_MENUMODULE 0  //DEBUG模式开关，代码上线之前务必关掉
#define FITTHEMEWINDOWS "org.lingmo.style"
#define FITCONTROLTRANS "org.lingmo.control-center.personalise"


#include <QObject>
#include <QWidget>
#include <QMenu>
#include <QPushButton>
#include <QToolButton>
#include <QDebug>
#include <QString>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRect>
#include <QScreen>
#include <QGSettings>
#include <QMetaEnum>
#include <QDesktopServices>
#include <QUrl>
#include "UIControl/base/daemonipcdbus.h"
#include "UI/base/widgetstyle.h"
class menuModule : public QWidget
{
    Q_OBJECT
public:
    explicit menuModule(QWidget *);
    void themeUpdate();

    enum typeThemeStatus {
        themeAuto = 0,
        themeBlackOnly = 1,
        themeLightOnly = 2
    } themeStatus;

Q_SIGNALS:
    void menuModuleSetThemeStyle(QString);
public:
    QToolButton *menuButton = nullptr;

public:
//    程序在实例化的时候需要传递的信息字段,打开debug开关后这些字段会被自动填充
    QString appName = "lingmo-music"; //格式lingmo-usb-creator
//    QString appShowingName = tr("lingmo music"); //格式lingmo usb creator ,用来在前端展示
    QString appVersion = qApp->applicationVersion();
    QString appDesc = "appDesc字段未填充";
    QString iconPath = ":/img/lingmo-music.png";
    QString confPath = "org.lingmo-music-data.settings";

private:
    QMenu *m_menu = nullptr;
    QMenu *themeMenu = nullptr;
    QSize iconSize;
    QString appPath = "tools/lingmo-music"; //拉起帮助菜单时使用appName字段
    QWidget *aboutWindow = nullptr;
    QGSettings *m_pGsettingThemeData = nullptr;
    QGSettings *m_pGsettingThemeStatus = nullptr;
public Q_SLOTS:
    void dealSystemGsettingChange(const QString);
    void slotLableSetFontSize(int size);
private:
    void init();
    QHBoxLayout* initTitleBar(); //关于窗口标题栏初始化
    QVBoxLayout* initBody();    // 关于窗口body初始化
    void initGsetting();
    void initAction();
    void setStyle();
    void triggerMenu(QAction* act); //主菜单动作4
    void triggerThemeMenu(QAction* act); //主题动作
    void aboutAction();
    void initAbout(); //关于窗口初始化
    void helpAction();
    void setThemeFromLocalThemeSetting(QList<QAction* >); //获取本地主题配置
    void setStyleByThemeGsetting(); //通过外部主题配置设置主题
    void setThemeStyle();
    void setThemeLight();
    DaemonIpcDbus *ipcDbus;
    void setThemeDark();
    QLabel* titleText = nullptr;
    QLabel* bodyAppName = nullptr;
    QLabel* bodyAppVersion = nullptr;
    QLabel* bodyAppDescribe = nullptr;
//    void updateTheme(); //点击菜单中的主题设置后更新一次主题
    QVBoxLayout *mainlyt = nullptr;

    void refreshThemeBySystemConf();    //通过系统配置更改主题
};

#endif // MENUMODULE_H
