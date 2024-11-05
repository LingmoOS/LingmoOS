#include "userframe.h"
#include <pwquality.h>
#include <QApplication>
#include <clocale>
#include <QDebug>
#include <QKeyEvent>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QtDBus>
#include <QRegExp>
#include <QDBusInterface>

#include "locale.h"
#include "libintl.h"
#include <QDBusMessage>
#include <QDBusArgument>
#include <QSizePolicy>
#include "../PluginService/ksystemenv.h"
#include "../PluginService/ksystemsetting_unit.h"
#include "../PluginService/kcommand.h"
#include "../PluginService/kdir.h"
#include "../PluginService/sysInfo/kpasswordcheck.h"
#include "lingmo-chkname.h"
#include "string"
#include "stdio.h"

namespace KInstaller{

using namespace KServer;

UserFrame::UserFrame(QWidget *parent) : MiddleFrameManager(parent),
    m_autoLoginFlag(false),
    blfinger(false)
{
    this->setObjectName("UserFrame");
    blfinger = findFingerPrintDevice();
    m_nextBtn->setEnabled(false);
    m_settings = pwquality_default_settings();
    pwquality_read_config(m_settings,PWQUALITYPATH, &auxerror);
    initUI();
    initAllConnect();
    addStyleSheet();
    setBoardTabOrder();
    KServer::KPasswordCheck::init();
    this->setFocus();
    translateStr();
}

UserFrame::~UserFrame()
{
    KServer::KPasswordCheck::getInstance()->releasePWquality();
}

void UserFrame::initUI()
{
    QRect screenRect = QGuiApplication::primaryScreen()->geometry();
    int nScreenHeight = screenRect.height();
    QVBoxLayout *usersetupLayout = new  QVBoxLayout();
    m_Widget->setLayout(usersetupLayout);
    usersetupLayout->setContentsMargins(0, 0, 0, 0);
    usersetupLayout->setSpacing(0);

    m_mainTitle = new QLabel();
    m_mainTitle->setObjectName("mainTitle");

    usersetupLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Minimum));
    usersetupLayout->addWidget(m_mainTitle, 1, Qt::AlignCenter | Qt::AlignTop);
    usersetupLayout->addItem(new QSpacerItem(10, 28, QSizePolicy::Expanding, QSizePolicy::Minimum));

    //用户名框设置
    QHBoxLayout* hlayout1 = new QHBoxLayout;
    hlayout1->setMargin(0);

    m_username = new CLineEditLabel("", QIcon(":/res/svg/user.svg"), this);
    m_username->setObjectName("username");
//    m_username->m_lineEdit->setStyleSheet("QLineEdit::placeholder{opacity:0.3;}}");
//    m_username->m_lineEdit->setStyleSheet("QLineEdit::placeholder{font-size:4px;opacity:0.3;}}");
//    QPalette myPale = m_username
//    QPalette pale = m_username->m_lineEdit->palette();
//    pale.setColor(QPalette::PlaceholderText, QColor("#b1b1b1"));
//    m_username->m_lineEdit->setPalette(pale);

    hlayout1->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
    hlayout1->addLayout(m_username->layout);
    hlayout1->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));

    m_userErrorTip=new QLabel(this);
    QHBoxLayout* hlayout1e = new QHBoxLayout;
    hlayout1e->setMargin(0);

    hlayout1e->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
    hlayout1e->addWidget(m_userErrorTip,1,Qt::AlignTop|Qt::AlignHCenter);
    hlayout1e->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));

    m_userErrorTip->setObjectName("m_userErrorTip");
    m_userErrorTip->setAlignment(Qt::AlignTop);
    m_userErrorTip->setStyleSheet("font-size: 16px;\
                                     font-family: Noto Sans CJK SC;\
                                    color: #FB9140;\                     
                                    opacity: 1;");

    //主机名框设置
    QHBoxLayout* hlayout2 = new QHBoxLayout;
    hlayout2->setMargin(0);

    m_hostname = new CLineEditLabel("", QIcon(":/res/svg/computer.svg"), this);
    m_hostname->setObjectName("hostname");
    m_hostname->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
//    m_hostname->m_lineEdit->setStyleSheet("QWidget:placeholder{opacity:0.3;}}");
//    QPalette pale1 = m_hostname->m_lineEdit->palette();
//    pale1.setColor(QPalette::PlaceholderText, QColor("#b1b1b1"));
//    m_hostname->m_lineEdit->setPalette(pale1);

    hlayout2->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
    hlayout2->addLayout(m_hostname->layout);
    hlayout2->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));

    m_hostErrorTip=new QLabel(this);
    m_hostErrorTip->setObjectName("m_hostErrorTip");
    QHBoxLayout* hlayout2e = new QHBoxLayout;
    hlayout2e->setMargin(0);

    hlayout2e->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
    hlayout2e->addWidget(m_hostErrorTip,1,Qt::AlignTop|Qt::AlignHCenter);
    hlayout2e->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));

    m_hostErrorTip->setStyleSheet("font-size: 16px;\
                                  font-family: Noto Sans CJK SC;\
                                 color: #FB9140;\
                                 opacity: 1;");

    //密码框设置
    QHBoxLayout* hlayout3 = new QHBoxLayout;
    hlayout3->setMargin(0);

    m_passwd = new CLineEditLabel("", QIcon(":/res/svg/passwd.svg"), this);
    m_passwd->setObjectName("passwd");
    m_passwd->setTrailingPng(QIcon(":/res/svg/code.svg"));
    m_passwd->m_lineEdit->setEchoMode(QLineEdit::Password);
    m_passwd->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    m_passwd->setBLToolTip(true);
//    m_passwd->m_lineEdit->setStyleSheet("QWidget::placeholder{opacity:0.3;}}");

    hlayout3->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
    hlayout3->addLayout(m_passwd->layout);
    hlayout3->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));

    m_pwdErrorTip=new QLabel(this);
    m_pwdErrorTip->setObjectName("m_pwdErrorTip");
    QHBoxLayout* hlayout3e = new QHBoxLayout;
    hlayout3e->setMargin(0);

    hlayout3e->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
    hlayout3e->addWidget(m_pwdErrorTip,1,Qt::AlignTop|Qt::AlignHCenter);
    hlayout3e->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));

    m_pwdErrorTip->setStyleSheet("font-size: 16px;\
                                 font-family: Noto Sans CJK SC;\
                                color: #FB9140;\
                                opacity: 1;");

    //确认密码框设置
    QHBoxLayout* hlayout4 = new QHBoxLayout;
    hlayout4->setMargin(0);
    hlayout4->setContentsMargins(0,0,0,0);

    m_confirm_passwd = new CLineEditLabel("", QIcon(":/res/svg/confirm_passwd.svg"), this);
    m_confirm_passwd->m_lineEdit->setEchoMode(QLineEdit::Password);
    m_confirm_passwd->setObjectName("confirm_passwd");
    m_confirm_passwd->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    m_confirm_passwd->setTrailingPng(QIcon(":/res/svg/code.svg"));
//    m_confirm_passwd->m_lineEdit->setStyleSheet("QLineEdit:placeholder{opacity:0.3;}}");

    hlayout4->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
    hlayout4->addLayout(m_confirm_passwd->layout);
    hlayout4->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));


    m_cpwdErrortip=new QLabel(this);
    m_cpwdErrortip->setObjectName("m_cpwdErrortip");
    QHBoxLayout* hlayout4e = new QHBoxLayout;
    hlayout4e->setMargin(0);
    hlayout4e->setContentsMargins(0,0,0,0);

    hlayout4e->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
    hlayout4e->addWidget(m_cpwdErrortip,1,Qt::AlignTop|Qt::AlignHCenter);
    hlayout4e->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));

    m_cpwdErrortip->setStyleSheet("font-size: 16px;\
                                  font-family: Noto Sans CJK SC;\
                                 color: #FB9140;\
                                 opacity: 1;");

    if (nScreenHeight > 600) {
        hlayout1->setContentsMargins(0,15,0,0);
        hlayout1e->setContentsMargins(0,15,0,0);
        hlayout2->setContentsMargins(0,15,0,0);
        hlayout2e->setContentsMargins(0,15,0,0);
        hlayout3->setContentsMargins(0,15,0,0);
        hlayout3e->setContentsMargins(0,15,0,0);
        hlayout4->setContentsMargins(0,15,0,0);
        hlayout4e->setContentsMargins(0,15,0,0);
    } else {
        hlayout1->setContentsMargins(0,0,0,0);
        hlayout1e->setContentsMargins(0,0,0,0);
        hlayout2->setContentsMargins(0,0,0,0);
        hlayout2e->setContentsMargins(0,0,0,0);
        hlayout3->setContentsMargins(0,0,0,0);
        hlayout3e->setContentsMargins(0,0,0,0);
        hlayout4->setContentsMargins(0,0,0,0);
        hlayout4e->setContentsMargins(0,0,0,0);
    }

    usersetupLayout->addLayout(hlayout1);
    usersetupLayout->addLayout(hlayout1e);

    usersetupLayout->addLayout(hlayout2);
    usersetupLayout->addLayout(hlayout2e);

    usersetupLayout->addLayout(hlayout3);
    usersetupLayout->addLayout(hlayout3e);

    usersetupLayout->addLayout(hlayout4);
    usersetupLayout->addLayout(hlayout4e);

    QHBoxLayout* hlayout5 = new QHBoxLayout;
    int len=(this->width()-m_username->width())/2;
    hlayout5->setContentsMargins(len,0,len,0);
    pwLoginCKBox = new QCheckBox();
    pwLoginCKBox->setChecked(false);

    hlayout5->addSpacerItem(new QSpacerItem(len, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
    hlayout5->addWidget(pwLoginCKBox);
    hlayout5->addSpacerItem(new QSpacerItem(len+m_username->width(), 34, QSizePolicy::Expanding, QSizePolicy::Preferred));

    usersetupLayout->addLayout(hlayout5);
    usersetupLayout->addItem(new QSpacerItem(10, 1000, QSizePolicy::Expanding, QSizePolicy::Expanding));


    fingerprintpng = new QLabel;
    fingerprinttip = new QLabel;
    if(blfinger) {
        fingerprintpng->setObjectName("fingerpng");
        fingerprintpng->setPixmap(QPixmap(":/res/svg/finger.png").scaled(16, 16));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHeightForWidth(fingerprintpng->sizePolicy().hasHeightForWidth());
        sizePolicy.setWidthForHeight(fingerprintpng->sizePolicy().hasWidthForHeight());
        sizePolicy.setVerticalStretch(0);
        fingerprintpng->setSizePolicy(sizePolicy);

        fingerprinttip->setObjectName("fingertip");
        fingerprinttip->setWordWrap(true);//bugI8VODI
        fingerprintpng->setContentsMargins(3, 3, 3, 44);
        QHBoxLayout* hlay = new QHBoxLayout;
        hlay->setSpacing(8);
        hlay->setContentsMargins(0, 0, 0, 0);
        QLabel *contex=new QLabel;
        contex->setLayout(hlay);
        contex->setMinimumSize(400,84);
        contex->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::MinimumExpanding);

        hlay->addWidget(fingerprintpng, Qt::AlignTop);
        fingerprintpng->setAlignment(Qt::AlignTop);
        hlay->addWidget(fingerprinttip, Qt::AlignCenter);
        fingerprinttip->setAlignment(Qt::AlignTop);
        hlay->addItem(new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Expanding));
        usersetupLayout->addWidget(contex,1,Qt::AlignCenter);
    }

    usersetupLayout->addItem(new QSpacerItem(10, 30, QSizePolicy::Expanding, QSizePolicy::Preferred));
    translateStr();
}

bool UserFrame::findFingerPrintDevice()
{

    QDBusMessage m = QDBusMessage::createMethodCall("org.lingmo.Biometric",
                         "/org/lingmo/Biometric",
                         "org.lingmo.Biometric",
                         "GetDevList");

    int count = 0;
    QDBusMessage res = QDBusConnection::systemBus().call(m);
    if(res.type() == QDBusMessage::ReplyMessage) {
        count = res.arguments().takeFirst().toInt();
    } else {
        return false;
    }
    if(count > 0) {
        return true;
    } else
        return false;

}

void UserFrame::setBoardTabOrder()
{
    QWidget::setTabOrder(m_username, m_hostname);
    QWidget::setTabOrder(m_hostname, m_passwd);
    QWidget::setTabOrder(m_passwd, m_confirm_passwd);
    QWidget::setTabOrder(m_confirm_passwd, m_nextBtn);
}

void UserFrame::initAllConnect()
{
    //用户名有改变，需要同时核对主机名，密码
    QObject::connect(m_username->m_lineEdit, &QLineEdit::textChanged, this, &UserFrame::checkUserName);
    QObject::connect(m_hostname->m_lineEdit, &QLineEdit::textChanged, this, &UserFrame::checkHostName);
    QObject::connect(m_passwd->m_lineEdit, &QLineEdit::textChanged, this, &UserFrame::checkPasswd);
    QObject::connect(m_confirm_passwd->m_lineEdit, &QLineEdit::textChanged, this, &UserFrame::checkCPasswd);
    QObject::connect(pwLoginCKBox, &QCheckBox::clicked, this, &UserFrame::getAutoLogin);

}

void UserFrame::checkUserName(QString username)
{
    if(username.isEmpty()) m_userstate=EMPY;
    else
    {
        m_userstate=lingmo_username_check(username.toUtf8(), 1);
        m_usererr=lingmo_username_strerror(m_userstate);
        qDebug()<<"error statue="<<  m_userstate<< m_usererr;
    }
    markLineEditCorrect();
    setComputerText();//设置主机名
    checkPasswd(m_passwd->getLineEditText());
}

void UserFrame::checkHostName(QString hostname)
{
    if(m_username->getLineEditText().isEmpty()){
        m_hostname->setLineEditText("");
        m_hoststate=EMPY;
        return;
    }

     const QRegExp reg("[a-zA-Z0-9\\._-]*");
    if(hostname.isEmpty()) m_hoststate=EMPY;
    else if(hostname.length() < HOSTNAME_MIN) {m_hoststate=HOSTNAME_TOOSHORT; }
    else if(hostname.length() > HOSTNAME_MAX)m_hoststate=HOSTNAME_TOOLONG;
    else if(!reg.exactMatch(hostname)){ m_hoststate=HOSTNAME_INVALCHAR;}
    else m_hoststate=HOSTNAME_OK;
    markLineEditCorrect();
}


void UserFrame::checkPasswd(QString pwd)
{
    if(pwd.isEmpty()) m_pwdstate=EMPY;
    else
    {
        m_pwdstate = KServer::KPasswordCheck::getInstance()->passwdCheck(pwd, m_pwderr, m_username->getLineEditText());
        if(m_pwdstate<0)
            m_pwdstate=200;
        else
            m_pwdstate=0;
    }
    markLineEditCorrect();
    checkCPasswd(m_confirm_passwd->getLineEditText());

}

void UserFrame::checkCPasswd(QString cpwd)
{   
    if(cpwd.isEmpty()) m_cpwdstate=EMPY;
    else if(cpwd.compare(m_passwd->getLineEditText())==0)
        m_cpwdstate=CPPASSWD_OK;
    else
    {
        m_cpwdstate=CPPASSWD_DIFF;
        m_cpwderr=(QObject::tr("Two password entries are inconsistent!"));
    }
   markLineEditCorrect();
}

void UserFrame::getAutoLogin()
{
    if(pwLoginCKBox->isChecked()) {
        m_autoLoginFlag = true;
    } else {
        m_autoLoginFlag = false;
    }
}
//标记错误信息
void UserFrame::markLineEditCorrect()
{
    //如果没有错，使能下一步按钮
    if((m_userstate | m_hoststate | m_pwdstate | m_cpwdstate)==0)
    {
        m_nextBtn->setEnabled(true);
        m_nextBtn->setStyleSheet("color:#FFFFFF;background:#1D8EFF;");
    }
    else
    {
        m_nextBtn->setEnabled(false);
        m_nextBtn->setStyleSheet("color: rgba(255, 255, 255, 0.4);background:#2D6BAA;");
    }


    //设置错误图标，设置错误提示
     if(m_userstate==0)
     {
         m_userErrorTip->setText("");
     }
     else if(m_userstate==EMPY){
         m_userErrorTip->setText("");
     }
     else{
        m_userErrorTip->setText(m_usererr);
    }



    //2.
     if(m_hoststate==0){
         m_hosterr="";
     m_hostErrorTip->setText("");
    }
    else if(m_hoststate==EMPY){
          m_hosterr="";
         m_hostErrorTip->setText("");
     }
     else{
        m_hosterr=QObject::tr("Your hostname only letters,numbers,underscore and hyphen are allowed, no more than 64 bits in length.");
        m_hostErrorTip->setText(m_hosterr);
    }



    //3.
     if(m_pwdstate==0) {
     m_pwdErrorTip->setText("");
    }
    else if(m_pwdstate==EMPY){
         m_pwdErrorTip->setText("");
     }
     else{
        m_pwdErrorTip->setText(m_pwderr);
    }


    //4
     if(m_cpwdstate==0){
     m_cpwdErrortip->setText("");
    }
    else if(m_cpwdstate==EMPY){
         m_cpwdErrortip->setText("");
     }
     else if( m_cpwdstate==CPPASSWD_DIFF){
        m_cpwdErrortip->setText(m_cpwderr);
    }

}

void UserFrame::addStyleSheet()
{
    QFile file(":/res/qss/KUserRegister.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
   QString stylesheet = filetext.readAll();
    file.close();
    this->setStyleSheet(stylesheet);
}

void UserFrame::setComputerText()
{
   if(m_username->getLineEditText().isEmpty()) {
    m_hostname->setLineEditText("");
     m_hoststate=EMPY;
     return;
   }

    QString path = QString("");
    QString error = QString("");
    QString systemManu = QString("");
    QString systemProName = QString("");
    QString systemVer = QString("");
    int excid = 0;
    QString txt = ("[\\ ,,\\*,\\&,\\\\,\\/,\\?,\\|,\\:,\\<,\\>,\"]");
    QRegExp reg(txt);
    KServer::KCommand::getInstance()->RunScripCommand("dmidecode", {"-s", "system-manufacturer"}, path, systemManu, error, excid);

    if (systemManu != "" && !systemManu.contains("N/A") && !systemManu.contains("Not Provided") && !systemManu.contains("Loongson", Qt::CaseInsensitive))
    {
        systemManu = systemManu.simplified().toLower();
        if(systemManu.contains("lenovo") || systemManu.contains("ibm") || systemManu.contains("huawei")) {
            KServer::KCommand::getInstance()->RunScripCommand("dmidecode", {"-s", "system-version"}, path, systemVer, error, excid);
            systemVer = systemVer.simplified().toLower();
            if (systemVer != "" && ! systemVer.contains(QRegExp("[\\x4e00-\\x9fa5]+"))) {
                m_hostname->setLineEditText(m_username->getLineEditText() + "-" +systemVer.remove(reg));
            } else {
                m_hostname->setLineEditText(m_username->getLineEditText() + "-" + systemVer.remove(reg));
            }

        } else {
            KServer::KCommand::getInstance()->RunScripCommand("dmidecode", {"-s", "system-product-name"}, path, systemProName, error, excid);
            systemProName = systemProName.simplified().toLower();
             systemProName.remove("-");
            if (systemProName != "" && ! systemProName.contains(QRegExp("[\\x4e00-\\x9fa5]+"))) {
                m_hostname->setLineEditText(m_username->getLineEditText() + "-" + systemProName.remove(reg));
            } else {
                m_hostname->setLineEditText(m_username->getLineEditText() + "-" + systemManu.remove(reg));
            }

        }
    } else {
        m_hostname->setLineEditText(m_username->getLineEditText() + "-PC");
        QDBusInterface *iface = new QDBusInterface("org.freedesktop.UPower", "/org/freedesktop/UPower", "org.freedesktop.UPower", QDBusConnection::systemBus());

        bool isLapTop = false;
        QVariant msg = iface->property("LidIsPresent");
        isLapTop =  msg.toBool();
        qDebug() << isLapTop;
        if (isLapTop) {
            m_hostname->setLineEditText(m_username->getLineEditText() + "-laptop");
        } else {
            m_hostname->setLineEditText(m_username->getLineEditText() + "-pc");
        }
    }
}

void UserFrame::translateStr()
{
    if(ReadSettingIni("config", "language").compare("zh_CN")==0)
    {
        unsetenv("LANG");
        setenv("LANG", "zh_CN.UTF-8",1);
//        setenv("LANGUAGE", "zh_CN:en",1);
        unsetenv("LANGUAGE");
        setenv("LANGUAGE", "zh_CN.en",1);
    }
    else if(ReadSettingIni("config", "language").compare("bo_CN")==0)
    {
        unsetenv("LANG");
        setenv("LANG", "bo_CN.UTF-8",1);
        unsetenv("LANGUAGE");
        setenv("LANGUAGE", "bo_CN.en",1);
    }
    else if(ReadSettingIni("config", "language").compare("en_US")==0)
    {
        unsetenv("LANG");
        setenv("LANG", "en_US.UTF-8",1);
        unsetenv("LANGUAGE");
        setenv("LANGUAGE", "en",1);
    }
    else if(ReadSettingIni("config", "language").compare("mn_MN")==0)
    {
        unsetenv("LANG");
        setenv("LANG", "mn_MN.UTF-8",1);
        unsetenv("LANGUAGE");
        setenv("LANGUAGE", "mn_MN.UTF-8",1);
    }
    else if(ReadSettingIni("config", "language").compare("zh_HK")==0)
    {
        unsetenv("LANG");
        setenv("LANG", "zh_HK.UTF-8",1);
        unsetenv("LANGUAGE");
        setenv("LANGUAGE", "zh_HK.UTF-8",1);
    }
    else if(ReadSettingIni("config", "language").compare("kk_KZ")==0)
    {
        unsetenv("LANG");
        setenv("LANG", "kk_KZ.UTF-8",1);
        unsetenv("LANGUAGE");
        setenv("LANGUAGE", "kk_KZ.UTF-8",1);
    }
    else if(ReadSettingIni("config", "language").compare("ug_CN")==0)
    {
        unsetenv("LANG");
        setenv("LANG", "ug_CN.UTF-8",1);
        unsetenv("LANGUAGE");
        setenv("LANGUAGE", "ug_CN.UTF-8",1);
    }
    else if(ReadSettingIni("config", "language").compare("ky_KG")==0)
    {
        unsetenv("LANG");
        setenv("LANG", "ky_KG.UTF-8",1);
        unsetenv("LANGUAGE");
        setenv("LANGUAGE", "ky_KG.UTF-8",1);
    }
    else if(ReadSettingIni("config", "language").compare("de_DE")==0)
    {
        unsetenv("LANG");
        setenv("LANG", "de_DE.UTF-8",1);
        unsetenv("LANGUAGE");
        setenv("LANGUAGE", "de_DE.UTF-8",1);
    }
    else if(ReadSettingIni("config", "language").compare("es_ES")==0)
    {
        unsetenv("LANG");
        setenv("LANG", "es_ES.UTF-8",1);
        unsetenv("LANGUAGE");
        setenv("LANGUAGE", "es_ES.UTF-8",1);
    }
    else if(ReadSettingIni("config", "language").compare("fr_FR")==0)
    {
        unsetenv("LANG");
        setenv("LANG", "fr_FR.UTF-8",1);
        unsetenv("LANGUAGE");
        setenv("LANGUAGE", "fr_FR.UTF-8",1);
    }

    m_mainTitle->setText(QObject::tr("Create User"));
    m_username->setPlaceholderText(QObject::tr("username"));
//    QPalette pale1 = m_hostname->m_lineEdit->palette();
//    pale1.setColor(QPalette::PlaceholderText, QColor("#e5e5e5"));
//    m_hostname->m_lineEdit->setPalette(pale1);

    m_hostname->setPlaceholderText(QObject::tr("hostname"));
//    QPalette pale2 = m_hostname->m_lineEdit->palette();
//    pale2.setColor(QPalette::PlaceholderText, QColor("#cccccc"));
//    m_hostname->m_lineEdit->setPalette(pale2);

    m_passwd->setPlaceholderText(QObject::tr("new password"));
//    QPalette pale3 = m_hostname->m_lineEdit->palette();
//    pale3.setColor(QPalette::PlaceholderText, QColor("#b0b0b0"));
//    m_hostname->m_lineEdit->setPalette(pale3);

    m_confirm_passwd->setPlaceholderText(QObject::tr("enter the password again"));
//    QPalette pale4 = m_hostname->m_lineEdit->palette();
//    pale4.setColor(QPalette::PlaceholderText, QColor("#8d8d8d"));
//    m_hostname->m_lineEdit->setPalette(pale4);

    m_nextBtn->setText(QObject::tr("Next"));
    pwLoginCKBox->setText(QObject::tr("Automatic login on boot"));
    fingerprinttip->setText(QObject::tr("After logging into the system, it is recommended that you set the biometric password in [Settings - Login Options] to have a better experience in the system"));

    checkUserName(m_username->getLineEditText());
    checkHostName(m_hostname->getLineEditText());
    checkPasswd(m_passwd->getLineEditText());
    checkCPasswd(m_confirm_passwd->getLineEditText());
}

void UserFrame::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        translateStr();
    } else {
        QWidget::changeEvent(event);
    }
}

void UserFrame::clickNextButton()
{
        emit signalKeyboradClose();
        writeSettingIni();
        emit signalStackPageChanged(1);
}

void UserFrame::writeSettingIni()
{
    WriteSettingToIni("config", "username", m_username->getLineEditText());
    WriteSettingToIni("config", "hostname", m_hostname->getLineEditText());
    WriteSettingToIni("config", "password", m_passwd->getLineEditText());
    WriteSettingToIni("config", "autologin", QString("%1").arg(m_autoLoginFlag));
}

void UserFrame::keyPressEvent(QKeyEvent *event)
{



    if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        if(m_nextBtn->isEnabled()==true)
        emit enterpressed();
    }
    else if(event->key() == Qt::Key_Backspace)
    {
        emit backspacepressed();
    }
    else if(event->key() == Qt::Key_Escape)
    {
        this->setFocus();
    }
    else
        QWidget::keyPressEvent(event);
}

}
