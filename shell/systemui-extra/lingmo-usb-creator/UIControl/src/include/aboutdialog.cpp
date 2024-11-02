#include "aboutdialog.h"
#include "lingmostylehelper/lingmostylehelper.h"
#include "windowmanager/windowmanager.h"
#include "global.h"
#include "xatom-helper.h"
#include "constant_class.h"
#define STYLE_NAME_VALUE_DEFAULT  "lingmo-default"
#define STYLE_NAME_VALUE_DARK     "lingmo-dark"
#define STYLE_NAME_VALUE_BLACK    "lingmo-black"
#define STYLE_NAME_VALUE_LIGHT    "lingmo-light"
#define STYLE_NAME_VALUE_WHITE    "lingmo-white"
#define SUPPORT "<u>support@kylinos.cn</u>"

aboutdialog::aboutdialog(QWidget *parent)
   : kdk::KAboutDialog(parent)
{
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

    setAppIcon(QIcon::fromTheme("lingmo-usb-creator"));
    setAppName(tr("usb boot maker"));
    setAppVersion(tr("Version: ") + getUSBCreatorVersion());
//    setBodyText(tr("USB Boot Maker provides system image making function."
//                   "The operation process is simple and easy."
//                   "You can choose ISO image and usb driver,"
//                   "and make boot driver with a few clicks."));
//     setBodyTextVisiable(true);
   //  setAppSupport(tr("Service & Support:%1").arg(SUPPORT));

     initGsetting();
     fontSizeInit();
     kdk::LingmoUIStyleHelper::self()->removeHeader(this);
     this->setModal(true);
     this->show();
     QRect availableGeometry1 = qApp->primaryScreen()->availableGeometry();
     this->move((availableGeometry1.width()-this->width())/2,(availableGeometry1.height()- this->height())/2);
     kdk::WindowManager::setGeometry(this->windowHandle(),this->geometry());


}
void aboutdialog::initGsetting(){
    m_isDarktheme=false;
    if(QGSettings::isSchemaInstalled(LINGMO_STYLE)){
        m_pGsettingThemeData = new QGSettings(LINGMO_STYLE);
        QString themeNow = m_pGsettingThemeData->get("styleName").toString();
        if("lingmo-dark" == themeNow || "lingmo-black" == themeNow){
            m_isDarktheme=true;
        }else{
            m_isDarktheme=false;
        }

        connect(m_pGsettingThemeData,&QGSettings::changed,this,&aboutdialog::dealSystemGsettingChange);
    }
}
void aboutdialog::themeUpdate(){
//    if(themeStatus == themeLightOnly)
//    {
//        m_isDarktheme=false;
//        setThemeLight();
//    }else if(themeStatus == themeBlackOnly){
//        setThemeDark();
//        m_isDarktheme=true;
//    }else{
//        setStyleByThemeGsetting();
//    }
}
void aboutdialog::setStyleByThemeGsetting(){
//    QString nowThemeStyle = m_pGsettingThemeData->get("styleName").toString();
//    if("lingmo-dark" == nowThemeStyle || "lingmo-black" == nowThemeStyle)
//    {
//        m_isDarktheme=true;
//        setThemeDark();
//    }else{
//        m_isDarktheme=false;
//        setThemeLight();
//    }
}
void aboutdialog::fontSizeInit(){
    connect(this,&aboutdialog::setFontSize,this,&aboutdialog::dealSetFontSize);
    QGSettings* m_fontsize = nullptr;
    if(QGSettings::isSchemaInstalled(LINGMO_STYLE)){
        m_fontsize = new QGSettings(LINGMO_STYLE);
        connect(m_fontsize,&QGSettings::changed,this,[=](const QString &key){
            if(key == LINGMO_FONT_SIZE){
                int fontsize = m_fontsize->get(LINGMO_FONT_SIZE).toInt();
                if(fontsize > 0){
                    emit setFontSize(fontsize);
                }
            }
        });
    }
    if(m_fontsize){
        dealSetFontSize(m_fontsize->get(LINGMO_FONT_SIZE).toInt());
    }
}
void aboutdialog::dealSetFontSize(int size){
    QFont font;
    font.setPointSize(size);
    this->setFont(font);
   // bodyAppDesc->setFont(font);
   // bodyAppName->setFont(font);
  //  bodySupport->setFont(font);
 //   titleText->setFont(font);
  //  bodyAppVersion->setFont(font);
}
void aboutdialog::dealSystemGsettingChange(const QString key){
    if(key == "styleName"){
        refreshThemeBySystemConf();
    }
    if(key==STYLE_ICON_NAME || key==STYLE_ICON){
        this->setAppIcon(QIcon::fromTheme("lingmo-usb-creator"));
       // this->setWindowIcon(QIcon::fromTheme("lingmo-usb-creator"));
    }
}

void aboutdialog::refreshThemeBySystemConf(){

}

void aboutdialog::setThemeDark(){
 //   m_isDarktheme=true;
}
void aboutdialog::setThemeLight(){
   // m_isDarktheme=false;

}
void aboutdialog::uiModeChange(bool isTabletMode){
    qInfo()<<"平板模式"<<isTabletMode;
    // 是否平板模式，平板模式
    m_isTabletMode = isTabletMode;
   // bool m_isMinimized=false;
    if (isTabletMode) {
        if(this->isMinimized()==false)
        {
            this->setWindowState(Qt::WindowMinimized);
            qInfo()<<"最小化"<<isMinimized();
        }
    }
}
QString aboutdialog::getUSBCreatorVersion()
{
    //return VERSION;
    FILE *pp = NULL;
       char *line = NULL;
       size_t len = 0;
       ssize_t read;
       char *q = NULL;
       QString version = tr("none");

       pp = popen("dpkg -l lingmo-usb-creator", "r");
       if(NULL == pp)
           return version;

       while((read = getline(&line, &len, pp)) != -1){
           q = strrchr(line, '\n');
           *q = '\0';

           QString content = line;
           QStringList list = content.split(" ");

           list.removeAll("");

           if (list.size() >= 3)
               version = list.at(2);
       }

       free(line);
       pclose(pp);
       return version;
}
