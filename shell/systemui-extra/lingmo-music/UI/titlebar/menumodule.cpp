#include "menumodule.h"
#include "UI/mainwidget.h"
#include "UI/base/xatom-helper.h"
#include "UIControl/global/global.h"
#include "UI/globalsignal.h"
#include "lingmostylehelper/lingmostylehelper.h"
#include "windowmanager/windowmanager.h"

#include <kaboutdialog.h>

#define PT_14 14

menuModule::menuModule(QWidget *parent = nullptr) : QWidget(parent)
{
    init();
}

void menuModule::init(){
    initAction();
    setStyle();
}

void menuModule::initAction(){
    aboutWindow = new QWidget();
    aboutWindow->setAutoFillBackground(true);
    aboutWindow->setBackgroundRole(QPalette::Base);
    titleText = new QLabel();
    bodyAppName = new QLabel();
    bodyAppVersion = new QLabel();
    bodyAppDescribe = new QLabel();

    menuButton = new QToolButton;
    menuButton->setToolTip(tr("Menu"));
    menuButton->setProperty("isWindowButton", 0x1);
    menuButton->setProperty("useIconHighlightEffect", 0x2);
    menuButton->setPopupMode(QToolButton::InstantPopup);
    menuButton->setFixedSize(30,30);
    menuButton->setIconSize(QSize(16, 16));
    menuButton->setAutoRaise(true);
    menuButton->setIcon(QIcon::fromTheme("open-menu-symbolic"));

    m_menu = new QMenu();
    QList<QAction *> actions ;
    QAction *actionTheme = new QAction(m_menu);
    actionTheme->setText(tr("Theme"));
    QAction *actionHelp = new QAction(m_menu);
    actionHelp->setText(tr("Help"));
    QAction *actionAbout = new QAction(m_menu);
    actionAbout->setText(tr("About"));
    QAction *actionExit = new QAction(m_menu);
    actionExit->setText(tr("Exit"));
//    actions<<actionTheme<<actionHelp<<actionAbout;    //隐藏关于主题颜色的设置，默认跟随主题，不随着用户操作更改。
    actions<<actionHelp<< actionAbout << actionExit;
    m_menu->addActions(actions);
//    互斥按钮组
    QMenu *themeMenu = new QMenu;
    QActionGroup *themeMenuGroup = new QActionGroup(this);
    QAction *autoTheme = new QAction(tr("Auto"),this);
    autoTheme->setObjectName("Auto");//用TEXT判断有风险
    themeMenuGroup->addAction(autoTheme);
    themeMenu->addAction(autoTheme);
    autoTheme->setCheckable(true);
    QAction *lightTheme = new QAction(tr("Light"),this);
    lightTheme->setObjectName("Light");//用TEXT判断有风险
//    themeMenuGroup->addAction(lightTheme);
//    themeMenu->addAction(lightTheme);
    lightTheme->setCheckable(true);
    QAction *darkTheme = new QAction(tr("Dark"),this);
    darkTheme->setObjectName("Dark");//用TEXT判断有风险
//    themeMenuGroup->addAction(darkTheme);
//    themeMenu->addAction(darkTheme);
    darkTheme->setCheckable(true);
    QList<QAction* > themeActions;
    themeActions<<autoTheme<<lightTheme<<darkTheme;
//    autoTheme->setChecked(true);
    actionTheme->setMenu(themeMenu);
    menuButton->setMenu(m_menu);
    connect(m_menu,&QMenu::triggered,this,&menuModule::triggerMenu);
    initGsetting();
    setThemeFromLocalThemeSetting(themeActions);
    themeUpdate();
    connect(themeMenu,&QMenu::triggered,this,&menuModule::triggerThemeMenu);

    //键盘F1响应唤出用户手册绑定
    connect(Widget::mutual,&Widget::signalShowGuide,this,&menuModule::helpAction);
    //限制应用字体不随着主题变化
//    QFont sizeFont;
//    sizeFont.setPixelSize(14);
//    m_menu->setFont(sizeFont);
//    bodyAppName->setFont(sizeFont);
//    titleText->setFont(sizeFont);
//    bodyAppVersion->setFont(sizeFont);
//    bodyAppDescribe->setFont(sizeFont);

}

void menuModule::setThemeFromLocalThemeSetting(QList<QAction* > themeActions)
{
#if DEBUG_MENUMODULE
    confPath = "org.lingmo-music-data.settings";
#endif
    m_pGsettingThemeStatus = new QGSettings(confPath.toLocal8Bit());
    if(!m_pGsettingThemeStatus){
    	qDebug()<<"系统对应的gsetting字段未正确配置";
    }
    QString appConf = m_pGsettingThemeStatus->get("thememode").toString();
    if("lightonly" == appConf){
        themeStatus = themeLightOnly;
        themeActions[1]->setChecked(true);   //程序gsetting中为浅色only的时候就给浅色按钮设置checked
    }else if("darkonly" == appConf){
        themeStatus = themeBlackOnly;
        themeActions[2]->setChecked(true);
    }else{
        themeStatus = themeAuto;
        themeActions[0]->setChecked(true);
    }
}

void menuModule::themeUpdate(){
//    if(themeStatus == themeLightOnly)
//    {
//        setThemeLight();
//    }else if(themeStatus == themeBlackOnly){
//        setThemeDark();
//    }else{
//        setStyleByThemeGsetting();
//    }
      setStyleByThemeGsetting();   //应用颜色更新只跟随主题

}

void menuModule::setStyleByThemeGsetting(){
    QString nowThemeStyle = m_pGsettingThemeData->get("styleName").toString();
    if("lingmo-dark" == nowThemeStyle || "lingmo-black" == nowThemeStyle)
    {
        setThemeDark();
    }else{
        setThemeLight();
    }
}

void menuModule::triggerMenu(QAction *act){


    QString str = act->text();
    if(tr("Exit") == str){
        g_user_signal->exitApp();
    }else if(tr("About") == str){
        aboutAction();
    }else if(tr("Help") == str){
        helpAction();
    }
}

void menuModule::triggerThemeMenu(QAction *act){
    if(!m_pGsettingThemeStatus)
    {
        m_pGsettingThemeStatus = new QGSettings(confPath.toLocal8Bit());  //m_pGsettingThemeStatus指针重复使用避免占用栈空间
    }
    QString str = act->objectName();
    if("Light" == str){
        themeStatus = themeLightOnly;
        disconnect(m_pGsettingThemeData,&QGSettings::changed,this,&menuModule::dealSystemGsettingChange);
        m_pGsettingThemeStatus->set("thememode","lightonly");
//        disconnect()
        setThemeLight();
    }else if("Dark" == str){
        themeStatus = themeBlackOnly;
        disconnect(m_pGsettingThemeData,&QGSettings::changed,this,&menuModule::dealSystemGsettingChange);
        m_pGsettingThemeStatus->set("thememode","darkonly");
        setThemeDark();
    }else{
        themeStatus = themeAuto;
        m_pGsettingThemeStatus->set("thememode","auto");
        initGsetting();
//        updateTheme();
        themeUpdate();
    }
}

void menuModule::aboutAction(){
//    关于点击事件处理
    initAbout();
}

void menuModule::helpAction(){
//    帮助点击事件处理
#if DEBUG_MENUMODULE
    appName = "tools/lingmo-music";
#endif
    if(!ipcDbus)
    {
        ipcDbus = new DaemonIpcDbus();
    }
    if(!ipcDbus->daemonIsNotRunning())
    {
        ipcDbus->showGuide(appName);
    }
}

void menuModule::initAbout(){
    KAboutDialog about_dialog(this);
    about_dialog.setModal(true);
    about_dialog.setAppIcon(QIcon::fromTheme("lingmo-music"));
    about_dialog.setAppName(tr("Music Player"));
    about_dialog.setAppVersion(tr("Version: ").append(appVersion));
    kdk::LingmoUIStyleHelper::self()->removeHeader(&about_dialog);
//    about_dialog.move(Widget::mutual->width()/2-about_dialog.width()/2+Widget::mutual->x(),Widget::mutual->height()/2-about_dialog.height()/2+Widget::mutual->y());
    about_dialog.show();
    if(!Global::isWayland){
        about_dialog.move(Widget::mutual->width()/2-about_dialog.width()/2+Widget::mutual->x(),Widget::mutual->height()/2-about_dialog.height()/2+Widget::mutual->y());
        kdk::WindowManager::setGeometry(about_dialog.windowHandle(),about_dialog.geometry());
    }
    about_dialog.exec();
}

QHBoxLayout* menuModule::initTitleBar(){
    QPushButton* titleIcon = new QPushButton();
    QPushButton *titleBtnClose = new QPushButton;
    titleIcon->setFixedSize(24,24);
    titleIcon->setIconSize(QSize(25,25));
    //TODO：直接从主题调图标，不会QIcon转qpixmap所以暂时从本地拿
//    titleIcon->setPixmap(QPixmap::fromImage(QImage(iconPath)));
    titleIcon->setIcon(QIcon::fromTheme("lingmo-music"));
    titleIcon->setStyleSheet("QPushButton{border:0px;background:transparent;}"
                             "QPushButton::hover{border:0px;background:transparent;}"
                             "QPushButton::pressed{border:0px;background:transparent;}");

//    titleIcon->setScaledContents(true);
    titleBtnClose->setFixedSize(30,30);
    titleBtnClose->setIcon(QIcon::fromTheme("window-close-symbolic"));
    titleBtnClose->setProperty("isWindowButton",0x2);
    titleBtnClose->setProperty("useIconHighlightEffect",0x8);
    titleBtnClose->setFlat(true);
    connect(titleBtnClose,&QPushButton::clicked,[=](){aboutWindow->close();});
    QHBoxLayout *hlyt = new QHBoxLayout;
    titleText->setText(tr("Music Player"));
//    titleText->setStyleSheet("font-size:14px;");
    hlyt->setSpacing(0);
    hlyt->setMargin(4);
    hlyt->addSpacing(4);
    hlyt->addWidget(titleIcon,0,Qt::AlignBottom); //居下显示
    hlyt->addSpacing(8);
    hlyt->addWidget(titleText,0,Qt::AlignBottom);
    hlyt->addStretch();
    hlyt->addWidget(titleBtnClose,0,Qt::AlignBottom);
    return hlyt;
}

void menuModule::slotLableSetFontSize(int size)
{
    //默认大小12px,换算成pt为9
    double lableBaseFontSize = PT_14;//魔鬼数字，自行处理
    double nowFontSize = lableBaseFontSize * double(size) / 11;//11为系统默认大小，魔鬼数字，自行处理
    QFont font;
    font.setPointSizeF(nowFontSize);
    bodyAppName->setFont(font);
}

QVBoxLayout* menuModule::initBody(){
    QPushButton* bodyIcon = new QPushButton();
//    bodyIcon->setPixmap(QPixmap::fromImage(QImage(iconPath)));
//    bodyIcon->setStyleSheet("font-size:14px;");
//    bodyIcon->setScaledContents(true);
    bodyIcon->setFixedSize(96,94);
    bodyIcon->setIconSize(QSize(96,94));
    bodyIcon->setIcon(QIcon::fromTheme("lingmo-music"));
    bodyIcon->setStyleSheet("QPushButton{border:0px;background:transparent;}"
                            "QPushButton::hover{border:0px;background:transparent;}"
                            "QPushButton::pressed{border:0px;background:transparent;}");

    //bodyAppDescribe->setText(tr("Music player is a multimedia playback software.Cover Various music formats Playback tools for,fast and simple."));
    //bodyAppDescribe->setFixedSize(380, 86);
//    bodyAppDescribe->setFixedHeight();
//    bodyAppDescribe->setStyleSheet("font-size:14px;");
    bodyAppDescribe->setWordWrap(true);
//    bodyAppName->setFixedHeight(28);
    bodyAppName->setText(tr("Music Player"));

//    bodyAppName->setStyleSheet("font-size:18px;");
//    bodyAppVersion->setFixedHeight(24);
    bodyAppVersion->setText(tr("Version: ") + appVersion);
    bodyAppVersion->setAlignment(Qt::AlignLeft);
//    bodyAppVersion->setStyleSheet("font-size:14px;");

    QVBoxLayout *vlyt = new QVBoxLayout;
//    vlyt->setMargin(0);
//    vlyt->setSpacing(0);
//    vlyt->addSpacing(20);
    vlyt->addWidget(bodyIcon,0,Qt::AlignHCenter);
    vlyt->addSpacing(5);
    vlyt->addWidget(bodyAppName,0,Qt::AlignHCenter);
    vlyt->addSpacing(5);
    vlyt->addWidget(bodyAppVersion,0,Qt::AlignHCenter);
    vlyt->addSpacing(5);
    vlyt->addWidget(bodyAppDescribe,0,Qt::AlignLeft);
    vlyt->addSpacing(5);
//    vlyt->addStretch();
    vlyt->setContentsMargins(20,10,20,10);
    return vlyt;
}

void menuModule::setStyle(){
//    menuButton->setStyleSheet("QPushButton::menu-indicator{image:None;}");
}

void menuModule::initGsetting(){
    if(QGSettings::isSchemaInstalled(FITTHEMEWINDOWS)){
        m_pGsettingThemeData = new QGSettings(FITTHEMEWINDOWS);
        connect(m_pGsettingThemeData,&QGSettings::changed,this,&menuModule::dealSystemGsettingChange);
    }

}

void menuModule::dealSystemGsettingChange(const QString key){
    if(key == "styleName"){
        refreshThemeBySystemConf();
    }
}

void menuModule::refreshThemeBySystemConf(){
    QString themeNow = m_pGsettingThemeData->get("styleName").toString();
    if("lingmo-dark" == themeNow || "lingmo-black" == themeNow){
        setThemeDark();
    }else{
        setThemeLight();
    }
}

void menuModule::setThemeDark(){
    Q_EMIT menuModuleSetThemeStyle("dark-theme");
}

void menuModule::setThemeLight(){
    Q_EMIT menuModuleSetThemeStyle("light-theme");

}
