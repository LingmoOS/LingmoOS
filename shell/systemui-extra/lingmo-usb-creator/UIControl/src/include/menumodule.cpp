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
#include "menumodule.h"
#include "xatom-helper.h"
#include "constant_class.h"
#define STYLE_NAME_VALUE_DEFAULT  "lingmo-default"
#define STYLE_NAME_VALUE_DARK     "lingmo-dark"
#define STYLE_NAME_VALUE_BLACK    "lingmo-black"
#define STYLE_NAME_VALUE_LIGHT    "lingmo-light"
#define STYLE_NAME_VALUE_WHITE    "lingmo-white"
#include "lingmostylehelper/lingmostylehelper.h"
#include "windowmanager/windowmanager.h"
#include "global.h"
#include "aboutdialog.h"
menuModule::menuModule(QWidget *parent) : QWidget(parent)
{
    if(QString(qgetenv("XDG_SESSION_TYPE")) == "wayland") {
        isWayland = true;
    }
    if(isWayland){
        kdk::LingmoUIStyleHelper::self()->removeHeader(this);
    }
    //initGsetting();
    init();

    //判断是否是平板
    QDBusConnection sessionBus = QDBusConnection::sessionBus();

    QDBusMessage m = QDBusMessage::createMethodCall(QString("com.lingmo.statusmanager.interface"),
                                                    QString("/"),
                                                    QString("com.lingmo.statusmanager.interface"),
                                                    QString("get_current_tabletmode"));

    QDBusMessage res = sessionBus.call(m);
    if (res.type() == 1 || res.type() == 2){

        m_isTabletMode = res.arguments().first().toBool();
         qInfo()<<"m_isTabletMode"<<m_isTabletMode;
    }
    else{
        qInfo()<<"m_isTabletMode"<<m_isTabletMode;

        m_isTabletMode = false;
    }
    //切换模式
    QDBusConnection::sessionBus().connect(QString("com.lingmo.statusmanager.interface"),
                                          QString("/"),
                                          QString("com.lingmo.statusmanager.interface"),
                                          QString("mode_change_signal"), this,
                                          SLOT(uiModeChange(bool)));


//    mStatusSettings = new QDBusInterface("com.lingmo.statusmanager.interface",
//                                         "/",
//                                         "com.lingmo.statusmanager.interface",
//                                         QDBusConnection::sessionBus(),
//                                         this);
//    if(mStatusSettings->isValid()){
//        QDBusReply<bool> is_tabletmode = mStatusSettings->call("get_current_tabletmode");
//        tabletmodechange(is_tabletmode);
//        connect(mStatusSettings,SIGNAL(mode_change_signal(bool)),this,SLOT(tabletmodechange(bool)));
//    }/*else{

//    }*/
}
void menuModule::uiModeChange(bool isTabletMode){
    qInfo()<<"平板模式"<<isTabletMode;
    // 是否平板模式，平板模式
    m_isTabletMode = isTabletMode;
   // bool m_isMinimized=false;
    if (isTabletMode) {
        if(aboutWindow->isMinimized()==false)
        {
            aboutWindow->setWindowState(Qt::WindowMinimized);
            qInfo()<<"最小化"<<isMinimized();
        }
    }
}
//void menuModule::tabletmodechange(bool istabletmode){
//    if(istabletmode){
//    aboutWindow->setWindowState(Qt::WindowMinimized);
//    //aboutWindow->isMinimized(true);
//    }

//}
void menuModule::init(){
    initAction();
}

void menuModule::initAction(){
//    titleIcon = new QPushButton();
//     bodyIcon = new QPushButton();
//    aboutWindow = new QWidget();

    titleIcon = new QLabel();
    bodyIcon = new QLabel();
    sucbackWindow = new QWidget();
    body =new QWidget();
    titleText = new QLabel();
    bodyAppName = new QLabel();
    bodyAppDesc = new QLabel();
    bodyAppVersion = new QLabel();
   // bodySupport = new QLabel();
    toollabel=new QLabel();
    submitbtn = new QPushButton();
    textedit=new QTextEdit();
//    menuButton = new QToolButton;
//    menuButton->setToolTip(tr("Menu"));
//    menuButton->setProperty("isWindowButton", 0x1);
//    menuButton->setProperty("useIconHighlightEffect", 0x2);
//    menuButton->setPopupMode(QToolButton::InstantPopup);
//    menuButton->setFixedSize(30,30);
//    menuButton->setIconSize(QSize(16, 16));
//    menuButton->setAutoRaise(true);
//    menuButton->setIcon(QIcon::fromTheme("open-menu-symbolic"));

    m_menu = new QMenu();
    QList<QAction *> actions ;
    actionTheme = new QAction(m_menu);
    actionTheme->setText(tr("Theme"));
    actionHelp = new QAction(m_menu);
    actionHelp->setText(tr("Help"));
    actionAbout = new QAction(m_menu);
    actionAbout->setText(tr("About"));
    actionsetting = new QAction(m_menu);
    actionsetting->setText(tr("Setting"));
    actionQuit = new QAction(m_menu);
    actionQuit->setText(tr("Close"));
   // actions<<actionTheme<<actionHelp<<actionAbout<<actionsetting<<actionQuit;
    actions<<actionAbout<<actionHelp<<actionQuit;
    m_menu->addActions(actions);
//    互斥按钮组

//    menuButton->setMenu(m_menu);
    connect(m_menu,&QMenu::triggered,this,&menuModule::triggerMenu);
    initGsetting();
    fontSizeInit();
    //setThemeFromLocalThemeSetting(themeActions);
    themeUpdate();
    connect(themeMenu,&QMenu::triggered,this,&menuModule::triggerThemeMenu);
  //  connect(helpMenu,&QMenu::triggered,this,&menuModule::triggerHelpMenu);
}

void menuModule::themeUpdate(){
    if(themeStatus == themeLightOnly)
    {
        m_isDarktheme=false;
        setThemeLight();
    }else if(themeStatus == themeBlackOnly){
        setThemeDark();
        m_isDarktheme=true;
    }else{
        setStyleByThemeGsetting();
    }
}

void menuModule::setStyleByThemeGsetting(){
    QString nowThemeStyle = m_pGsettingThemeData->get("styleName").toString();
    if("lingmo-dark" == nowThemeStyle || "lingmo-black" == nowThemeStyle)
    {
        m_isDarktheme=true;
        setThemeDark();
    }else{
        m_isDarktheme=false;
        setThemeLight();
    }
}

void menuModule::triggerMenu(QAction *act){
    QString str = act->text();
    if(tr("Quit") == str){
        emit menuModuleClose();
    }else if(tr("About") == str){
        aboutAction();
    }else if(tr("Help") == str){
        helpAction();
    }
}

void menuModule::triggerThemeMenu(QAction *act){
    if(!m_pGsettingThemeStatus)
    {
        m_pGsettingThemeStatus = new QGSettings(APPDATA);  //m_pGsettingThemeStatus指针重复使用避免占用栈空间
    }
    QString str = act->text();
    if(tr("Light") == str){
        m_isDarktheme=false;
        themeStatus = themeLightOnly;
        disconnect(m_pGsettingThemeData,&QGSettings::changed,this,&menuModule::dealSystemGsettingChange);
        m_pGsettingThemeStatus->set("thememode","lightonly");
        setThemeLight();
    }else if(tr("Dark") == str){
        m_isDarktheme=true;
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
        fontSizeInit();
    }
}
void menuModule::triggerHelpMenu(QAction *act){
    QString str=act->text();
//    if(str==tr("successback")){
//        successbackAction();
//    }
}
void menuModule::successbackAction(){
   // initsuccessbackdilog();
}
void menuModule::aboutAction(){
    emit clickabout();
//    关于点击事件处理
//    if(isWayland){
//        kdk::LingmoUIStyleHelper::self()->removeHeader(this);
//        kdk::LingmoUIStyleHelper::self()->removeHeader((QWidget *)aboutWindow);
//    }
   // initAbout();
}

void menuModule::helpAction(){
//    帮助点击事件处理
    if(!ipcDbus)
    {
        ipcDbus = new DaemonIpcDbus();
    }
    if(!ipcDbus->daemonIsNotRunning()){
       // ipcDbus->showGuide(appName);
        ipcDbus->showGuide("lingmo-usb-creator");
    }
}
void menuModule::initsuccessbackdilog(){
    return;

}
void menuModule::slot_limitTextEditTextCount(){
    limitTextEditTextCount(textedit,150);
}
void menuModule::limitTextEditTextCount(QTextEdit* text_edit,const int& limit_count)
{
  QString text = text_edit->toPlainText();
  int count=text.count();
  if(count>limit_count){
      toollabel->setText(tr("Maximum 150 words!"));
      int position = text_edit->textCursor().position();
      QTextCursor text_cursor = text_edit->textCursor();
      text.remove(position - (count - limit_count), count - limit_count);

      text_edit->setText(text);
      text_cursor.setPosition(position - (count - limit_count));
      text_edit->setTextCursor(text_cursor);
  }
}
/*void menuModule::initAbout(){
    aboutdialog * aboutWindow=new aboutdialog;
//    aboutWindow = new QWidget();
//    aboutWindow->setAttribute(Qt::WA_DeleteOnClose);

//    if(isWayland){
//        kdk::LingmoUIStyleHelper::self()->removeHeader(this);
//        kdk::LingmoUIStyleHelper::self()->removeHeader((QWidget *)aboutWindow);
//    }else{
        MotifWmHints hints;
        hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
        hints.functions = MWM_FUNC_ALL;
        hints.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(aboutWindow->winId(), hints);
        //弹窗位置应用居中
        QRect availableGeometry = this->parentWidget()->geometry();
       this->move(availableGeometry.center()-this->rect().center());
//    }
//    aboutWindow->setWindowModality(Qt::ApplicationModal);
//    aboutWindow->setWindowFlag(Qt::Tool);
//    aboutWindow->setFixedSize(420,400);
//    QVBoxLayout *mainlyt = new QVBoxLayout();
//    QHBoxLayout *titleLyt = initTitleBar();
//    QVBoxLayout *bodylyt = initBody();
//    mainlyt->setMargin(0);
//    mainlyt->addLayout(titleLyt);
//    mainlyt->addLayout(bodylyt);
//    mainlyt->addStretch();
//    aboutWindow->setLayout(mainlyt);
//    //TODO:在屏幕中央显示
//    QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
//    aboutWindow->move((availableGeometry.width()-aboutWindow->width())/2,(availableGeometry.height()- aboutWindow->height())/2);
    //弹窗位置应用居中
//    QRect availableGeometry = this->parentWidget()->geometry();
//    aboutWindow->move(availableGeometry.center()-aboutWindow->rect().center());
//    aboutWindow->show();
}*/
QHBoxLayout*menuModule::initTitlesucbackBar(){
//    QPushButton *titleBtnClose = new QPushButton;
//    titleBtnClose->setFixedSize(30,30);
//    titleBtnClose->setIcon(QIcon::fromTheme("window-close-symbolic"));
//    titleBtnClose->setProperty("isWindowButton",0x2);
//    titleBtnClose->setProperty("useIconHighlightEffect",0x8);
//    titleBtnClose->setFlat(true);
//    titleBtnClose->setToolTip(tr("Quit"));
//    QHBoxLayout *hlyt = new QHBoxLayout;
//    titleText->setText(tr("successback"));
//    hlyt->setSpacing(0);
//    hlyt->setMargin(4);
//    hlyt->addSpacing(4);
//    hlyt->addSpacing(8);
//    hlyt->addWidget(titleText,0,Qt::AlignBottom);
//    hlyt->addStretch();
//    hlyt->addWidget(titleBtnClose,0,Qt::AlignBottom);
//    return hlyt;
}
//QHBoxLayout* menuModule::initTitleBar(){
//    QPushButton *titleBtnClose = new QPushButton;
//    titleIcon->setFixedSize(QSize(24,24));
//    titleIcon->setFocusPolicy(Qt::NoFocus);
//    titleIcon->setPixmap(QIcon::fromTheme("lingmo-usb-creator").pixmap(24,24));
//    titleBtnClose->setFixedSize(30,30);
//    titleBtnClose->setIcon(QIcon::fromTheme("window-close-symbolic"));
//    titleBtnClose->setProperty("isWindowButton",0x2);
//    titleBtnClose->setProperty("useIconHighlightEffect",0x8);
//    titleBtnClose->setFlat(true);
//    titleBtnClose->setToolTip(tr("Quit"));
//    connect(titleBtnClose,&QPushButton::clicked,[=](){aboutWindow->close();});
//    QHBoxLayout *hlyt = new QHBoxLayout;
//    titleText->setText(tr("usb boot maker"));
//    hlyt->setSpacing(0);
//    hlyt->setMargin(4);
//    hlyt->addSpacing(4);
//    hlyt->addWidget(titleIcon,0,Qt::AlignBottom); //居下显示
//    hlyt->addSpacing(8);
//    hlyt->addWidget(titleText,0,Qt::AlignBottom);
//    hlyt->addStretch();
//    hlyt->addWidget(titleBtnClose,0,Qt::AlignBottom);
//    return hlyt;
//}

/*QVBoxLayout* menuModule::initBody(){
    bodyIcon->setFixedSize(96,96);
    bodyIcon->setPixmap(QIcon::fromTheme("lingmo-usb-creator").pixmap(96,96));
    bodyIcon->setFocusPolicy(Qt::NoFocus);
   // bodyIcon->setIcon(QIcon::fromTheme("lingmo-usb-creator",QIcon(iconPath)));
  //  bodyIcon->setSize(QSize(96,96));
    bodyAppDesc->setText(tr("USB Boot Maker provides system image making function."
                            "The operation process is simple and easy."
                            "You can choose ISO image and usb driver,"
                            "and make boot driver with a few clicks."));
    bodyAppDesc->setFixedWidth(390);
    bodyAppDesc->setWordWrap(true);
    bodyAppName->setFixedHeight(28);
    bodyAppName->setText(tr("usb boot maker"));
    bodyAppVersion->setFixedHeight(30);
    bodyAppVersion->setText(tr("Version: ") + appVersion);
    bodyAppVersion->setAlignment(Qt::AlignLeft);

    connect(bodySupport,&QLabel::linkActivated,this,[=](const QString url){
        QDesktopServices::openUrl(QUrl(url));
    });
    bodySupport->setContextMenuPolicy(Qt::NoContextMenu);
    bodySupport->setFixedHeight(30);
    QVBoxLayout *vlyt = new QVBoxLayout;
    vlyt->setMargin(0);
    vlyt->setSpacing(0);
    vlyt->addSpacing(12);
    vlyt->addWidget(bodyIcon,0,Qt::AlignHCenter);
    vlyt->addSpacing(12);
    vlyt->addWidget(bodyAppName,0,Qt::AlignHCenter);
    vlyt->addSpacing(12);
    vlyt->addWidget(bodyAppVersion,0,Qt::AlignHCenter);
    vlyt->addSpacing(12);
    vlyt->addWidget(bodyAppDesc,0,Qt::AlignHCenter);
    vlyt->addSpacing(12);
    vlyt->addWidget(bodySupport,0,Qt::AlignHCenter);
    vlyt->addStretch();
    return vlyt;
}*/
void menuModule::initGsetting(){
    m_isDarktheme=false;
    if(QGSettings::isSchemaInstalled(FITTHEMEWINDOW)){
        m_pGsettingThemeData = new QGSettings(FITTHEMEWINDOW);
        QString themeNow = m_pGsettingThemeData->get("styleName").toString();
        if("lingmo-dark" == themeNow || "lingmo-black" == themeNow){
            m_isDarktheme=true;
        }else{
            m_isDarktheme=false;
        }

        connect(m_pGsettingThemeData,&QGSettings::changed,this,&menuModule::dealSystemGsettingChange);
    }
}
void menuModule::fontSizeInit(){
    connect(this,&menuModule::setFontSize,this,&menuModule::dealSetFontSize);
    QGSettings* m_fontsize = nullptr;
    if(QGSettings::isSchemaInstalled(LINGMO_STYLE)){
        m_fontsize = new QGSettings(LINGMO_STYLE);
        connect(m_fontsize,&QGSettings::changed,this,[=](const QString &key){
            if(key == LINGMO_FONT_SIZE){
                double fontsize = m_fontsize->get(LINGMO_FONT_SIZE).toString().toDouble();
                qInfo()<<"字体"<<fontsize;
                if(fontsize > 0){
                    emit setFontSize(fontsize);
                }
            }
        });
    }
    if(m_fontsize){
        dealSetFontSize(m_fontsize->get(LINGMO_FONT_SIZE).toString().toDouble());
    }
}
void menuModule::dealSetFontSize(double size){
    QFont font;
    font.setPointSize(size);
    this->setFont(font);
    bodyAppDesc->setFont(font);
    bodyAppName->setFont(font);
  //  bodySupport->setFont(font);
    titleText->setFont(font);
    bodyAppVersion->setFont(font);
    actionQuit->setFont(font);
    actionTheme->setFont(font);
    actionHelp->setFont(font);
    actionAbout->setFont(font);
    actionsetting->setFont(font);
    textedit->setFont(font);
    toollabel->setFont(font);
    submitbtn->setFont(font);
    //officewebsitehelp->setFont(font);
   // successbackhelp->setFont(font);
   // checkupdatehelp->setFont(font);
    //autoTheme->setFont(font);
  //  darkTheme->setFont(font);
  //  lightTheme->setFont(font);
}
void menuModule::keyPressEvent(QKeyEvent *event){
    if(event->key() == Qt::Key_F1){
        emit pullupHelp();
    }else{
        QWidget::keyPressEvent(event);
    }
}
void menuModule::dealSystemGsettingChange(const QString key){
    if(key == "styleName"){
        refreshThemeBySystemConf();
    }
    if(key==STYLE_ICON_NAME || key==STYLE_ICON){
        emit iconchange();
        bodyIcon->setPixmap(QIcon::fromTheme("lingmo-usb-creator").pixmap(96,96));
        titleIcon->setPixmap(QIcon::fromTheme("lingmo-usb-creator").pixmap(24,24));
    }
}

void menuModule::refreshThemeBySystemConf(){
    QString themeNow = m_pGsettingThemeData->get("styleName").toString();
    if("lingmo-dark" == themeNow || "lingmo-black" == themeNow){
        m_isDarktheme=true;
        setThemeDark();
    }else{
        m_isDarktheme=false;
        setThemeLight();
    }
}

void menuModule::setThemeDark(){
    m_isDarktheme=true;
    //aboutWindow->setStyleSheet(".QWidget{background-color:rgba(61,61,65,1);}");
  //  textedit->setStyleSheet("background-color:rgba(61,61,65,1);color:rgba(255,255,255,1);");
 //   this->setStyleSheet("QLabel{color:rgba(255,255,255,1);}");
 //   titleText->setStyleSheet("color:rgba(255,255,255,1);");
    emit menuModuleSetThemeStyle("dark-theme");
//    bodySupport->setText(tr("Service & Support: ") +
//                         "<a href=\"mailto://support@kylinos.cn\""
//                         "style=\"color:white\">"
//                         "support@kylinos.cn</a>"
//                         );
//    bodyIcon->setStyleSheet("QPushButton{border:0px;border-radius:4px;background:transparent;}"
//                            "QPushButton::hover{border:0px;border-radius:4px;background:transparent;}"
//                            "QPushButton::pressed{border:0px;border-radius:4px;background:transparent;}"
//                            );
//    titleIcon->setStyleSheet("QPushButton{border:0px;border-radius:4px;background:transparent;}"
//                            "QPushButton::hover{border:0px;border-radius:4px;background:transparent;}"
//                            "QPushButton::pressed{border:0px;border-radius:4px;background:transparent;}"
//                            );
//    submitbtn->setStyleSheet("QPushButton{border:0px;border-radius:4px;background-color:rgba(0,0,0,1);color:rgba(255,255,255,1)}"
//                             "QPushButton::hover{border:0px;border-radius:4px;background-color:rgba(41,135,236,1);color:rgba(255,255,255,1)}"
//                             "QPushButton::pressed{border:0px;border-radius:4px;background-color:rgba(0,0,0,1);color:rgba(255,255,255,1)}");
}
void menuModule::setThemeLight(){
    m_isDarktheme=false;
  //  aboutWindow->setStyleSheet(".QWidget{background-color:rgba(255,255,255,1);}");
 //   titleText->setStyleSheet("color:rgba(0,0,0,1);");
 //   textedit->setStyleSheet("background-color:rgba(255,255,255,1);color:rgba(61,61,65,1);");
    emit menuModuleSetThemeStyle("light-theme");
//    bodySupport->setText(tr("Service & Support: ") +
//                         "<a href=\"mailto://support@kylinos.cn\""
//                         "style=\"color:black\">"
//                         "support@kylinos.cn</a>");
//    bodyIcon->setStyleSheet("QPushButton{border:0px;border-radius:4px;background:transparent;}"
//                            "QPushButton::hover{border:0px;border-radius:4px;background:transparent;}"
//                            "QPushButton::pressed{border:0px;border-radius:4px;background:transparent;}"
//                            );
//    titleIcon->setStyleSheet("QPushButton{border:0px;border-radius:4px;background:transparent;}"
//                            "QPushButton::hover{border:0px;border-radius:4px;background:transparent;}"
//                            "QPushButton::pressed{border:0px;border-radius:4px;background:transparent;}"
//                            );
//    submitbtn->setStyleSheet("QPushButton{border:0px;border-radius:4px;background-color:rgba(128,128,128,1);color:rgba(0,0,0,1)}"
//                             "QPushButton::hover{border:0px;border-radius:4px;background-color:rgba(41,135,236,1);color:rgba(0,0,0,1)}"
//                             "QPushButton::pressed{border:0px;border-radius:4px;background-color:rgba(128,128,128,1);color:rgba(0,0,0,1)}");
}
