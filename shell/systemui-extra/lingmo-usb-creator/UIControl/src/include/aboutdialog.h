#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

///#define FITTHEMEWINDOW "org.lingmo.style"
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
#include <QDBusConnection>
#include <QDBusInterface>
#include "daemonipcdbus.h"
#include "kaboutdialog.h"
class aboutdialog : public kdk::KAboutDialog
{
    Q_OBJECT
public:
    explicit aboutdialog(QWidget * parent = nullptr);
    bool isWayland=false;
    void themeUpdate();
    void fontSizeInit();
    void setThemeDark();
    bool m_isDarktheme;
    bool isDarkTheme() {
        return m_isDarktheme;
    }

signals:
    void menuModuleClose();
    void menuModuleSetThemeStyle(QString);
    void pullupHelp();
    void iconchange();
    void setFontSize(int);

public:
    QToolButton *menuButton = nullptr;

public:
//    程序在实例化的时候需要传递的信息字段,打开debug开关后这些字段会被自动填充
    QString appName = "tools/lingmo-usb-creator"; //格式lingmo-usb-creator
    QString appVersion ;
    QString appDesc = "appDesc字段未填充!";
    QString iconPath = ":/data/lingmo-usb-creator.png";
    QString confPath = "org.lingmo-usb-creator-data.settings";

private:
    QMenu *m_menu = nullptr;
    QMenu *themeMenu = nullptr;
    QDBusInterface *mStatusSettings = nullptr;
    bool m_isTabletMode;//判断是否是平板模式
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
    void dealSetFontSize(int size);
     void uiModeChange(bool isTabletMode);
private:
    //void keyPressEvent(QKeyEvent *event);
    void init();
    QHBoxLayout* initTitleBar(); //关于窗口标题栏初始化
    QVBoxLayout* initBody();    // 关于窗口body初始化
    void initGsetting();
    void initAction();
    void setStyle();
    void aboutAction();
    void initAbout(); //关于窗口初始化
    void helpAction();
    void setThemeFromLocalThemeSetting(QList<QAction* >); //获取本地主题配置
    void setStyleByThemeGsetting(); //通过外部主题配置设置主题
    void setThemeStyle();
    void setThemeLight();
    DaemonIpcDbus *ipcDbus = nullptr;
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

    void refreshThemeBySystemConf();    //通过系统配置更改主题
private:
    QString getUSBCreatorVersion();
};

#endif // ABOUTDIALOG_H
