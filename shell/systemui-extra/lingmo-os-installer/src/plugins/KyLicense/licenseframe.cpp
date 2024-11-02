#include "licenseframe.h"

#include <QDebug>

#include "../PluginService/klinguist.h"
#include <QTextCodec>
#include <QEvent>
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QScrollBar>
#include <QtDBus/QtDBus>
#include <QDBusError>
#include "../PluginService/ksystemsetting_unit.h"
#include "../PluginService/ui_unit/xrandrobject.h"
using namespace KServer;
namespace KInstaller {


LicenseFrame::LicenseFrame(QWidget *parent) : MiddleFrameManager(parent),
    lang_bl(true),
    scrollbarInt(0),
    tmpDouble(0)
{
    m_nextBtn->setEnabled(false);
    initUI();
    initAllConnect();
    addStyleSheet();
    setBoardTabOrder();
    translateStr();
    this->setFocus();
}

void LicenseFrame::initUI()
{
    // 定义一个横向 layout, 居中对其，边距为左 300,上 20,右 300,下 20,
    licneseLayout = new QGridLayout();
    licneseLayout->setSpacing(0);
    licneseLayout->setContentsMargins(0, 0, 0, 0);
//    m_Widget->setLayout(licneseLayout);
    m_Widget->setLayout(licneseLayout);

    licneseLayout->setColumnStretch(0, 2);
    licneseLayout->setColumnStretch(1, 5);
    licneseLayout->setColumnStretch(2, 2);

    m_mainTitle = new QLabel();
    m_mainTitle->setObjectName("mainTitle");
    licneseLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Preferred), 0, 1, 1, 1);
    licneseLayout->addWidget(m_mainTitle, 1, 1, 1, 1, Qt::AlignCenter | Qt::AlignTop);
    licneseLayout->addItem(new QSpacerItem(10, 34, QSizePolicy::Expanding, QSizePolicy::Preferred), 2, 1, 1, 1);

    // 定义一个 QTextBrowser，来用显示许可协议
    license_text = new QTextBrowser();
    license_text->setObjectName("licensetext");
    license_text->setAlignment(Qt::AlignCenter);
    license_text->setContextMenuPolicy(Qt::NoContextMenu);

    loadLicense();
//    licneseLayout->setRowStretch(3, 4);
//    license_text->setMaximumSize(1500, 1050);
//    license_text->setMinimumSize(800, 500);

    license_text->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    license_text->setWindowFlags( Qt::FramelessWindowHint);

    licneseLayout->addWidget(license_text, 3, 1, 1, 1, Qt::AlignCenter);
    licneseLayout->addItem(new QSpacerItem(10, 36, QSizePolicy::Fixed, QSizePolicy::Fixed), 4, 1, 1, 1);

    QVBoxLayout * vLayout = new QVBoxLayout();
    vLayout->setSpacing(0);
    m_sendData = new QCheckBox;
//    licneseLayout->addWidget(m_sendData, 5, 1, 1, 1, Qt::AlignCenter);
    m_sendData->setCheckable(true);
    int nUploadState = 0;
    getUploadState(nUploadState);
    if(1 == nUploadState)
        m_sendData->setChecked(true);
    else
        m_sendData->setChecked(false);
    vLayout->addWidget(m_sendData, 0, Qt::AlignLeft);
    m_timetip = new QCheckBox;
//    licneseLayout->addWidget(m_timetip, 6, 1, 1, 1, Qt::AlignCenter);
    vLayout->addWidget(m_timetip, 0, Qt::AlignLeft);
    licneseLayout->addLayout(vLayout, 5, 1, Qt::AlignCenter);
    licneseLayout->addItem(new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Expanding), 6, 1, 1, 1);
//    m_timetip->setCheckable(false);
//    m_timetip->setEnabled(false);
}

void LicenseFrame::setBoardTabOrder()
{
    QWidget::setTabOrder(license_text, m_nextBtn);
}

QString LicenseFrame::readSettingIni()
{
    return KServer::ReadSettingIni("config", "language");

}
void LicenseFrame::loadLicense()
{
    // 打开需要显示的许可协议文件
    QString strCurLang = readSettingIni();
    if(strCurLang == "zh_CN") {
        path = ":/res/file/EULA";
    } else if(strCurLang == "en_US" ||
              strCurLang == "de_DE" ||
              strCurLang == "es_ES" ||
              strCurLang == "fr_FR") {
        path = ":/res/file/EULAE";
    }else if(strCurLang == "bo_CN") {
        path = ":/res/file/EULABO";
    }else if(strCurLang == "ug_CN") {
        path = ":/res/file/EULAUG";
    }else if(strCurLang == "ky_KG") {
        path = ":/res/file/EULAKY";
    }else if(strCurLang == "kk_KZ") {
        path = ":/res/file/EULAKK";
    }else
        path = ":/res/file/EULA";

    license_text->clear();
    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        qDebug() << "license Can not open";
    QTextStream in(&file);
    // 设置字体编码格式
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    in.setCodec(codec);

    // 读取文件内容进显示控件
    // license_text->setText(in.readAll());
    m_fileStr = in.readAll();
    license_text->setText(m_fileStr);
    license_text->setLineWrapMode(QTextBrowser::WidgetWidth);
    license_text->setLineWrapColumnOrWidth(license_text->width());
    license_text->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
 //   license_text->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    license_text->verticalScrollBar()->hasTracking();
    QRect rect = KServer::GetScreenRect();
    if(rect.height() < 768)
    {
        license_text->setFixedSize(rect.width() / 2 + 50, rect.height() / 2 - 50);//bugI810KM
    }
    else
        license_text->setFixedSize(rect.width() / 2, rect.height() / 2 - 20);

    file.close();
}

void LicenseFrame::initAllConnect()
{
    connect(this->m_timetip, &QCheckBox::clicked, this, &LicenseFrame::nextBtnAvailable);
    connect(license_text->verticalScrollBar(), &QScrollBar::valueChanged, this, &LicenseFrame::nextBtnAvailable);
    connect(this->m_sendData, &QCheckBox::clicked, this, &LicenseFrame::setUploadState);
}
void LicenseFrame::addStyleSheet()
{
    QFile file(":/res/qss/KyLicense.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = QString("");
    stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}
void LicenseFrame::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        translateStr();
        qDebug() << "1:" << lang_bl;
        lang_bl = !lang_bl;
        tmpDouble =1.0 * license_text->verticalScrollBar()->value() / license_text->verticalScrollBar()->maximum();
        loadLicense();
//        license_text->verticalScrollBar()->setValue(license_text->verticalScrollBar()->maximum() * tmpDouble);
        qDebug() << "2:" << lang_bl;
    } else {
        QWidget::changeEvent(event);
    }
}



void LicenseFrame::translateStr()
{
    m_mainTitle->setText(tr("Read License Agreement"));//阅读许可协议
    m_sendData->setText(tr("Send optional diagnostic data"));//发送可选诊断数据
    m_timetip->setText(tr("I have read and agree to the terms of the agreement"));//我已经阅读并同意协议条款
//    license_text->setHtml(QObject::tr(m_fileStr));
    m_nextBtn->setText(tr("Next"));
}

void LicenseFrame::nextBtnAvailable()
{
    if(m_timetip->isChecked()) {
        m_nextBtn->setEnabled(true);
        m_nextBtn->setStyleSheet("background:#1D8EFF;");
    } else {
        m_nextBtn->setEnabled(false);
    }
}

void LicenseFrame::clickNextButton()
{
    emit signalStackPageChanged(1);
}

void LicenseFrame::keyPressEvent(QKeyEvent *event)
{
    qDebug()<< Q_FUNC_INFO << event->key() << "key pressed!";
    if(event->key() == Qt::Key_Up)
    {
        license_text->verticalScrollBar()->setValue(license_text->verticalScrollBar()->value()-50);
    }
    else if(event->key() == Qt::Key_Down)
    {
        license_text->verticalScrollBar()->setValue(license_text->verticalScrollBar()->value()+50);
    }
    else if (event->key() == Qt::Key_PageUp) {
        license_text->verticalScrollBar()->setValue(license_text->verticalScrollBar()->value()-500);
    }
    else if (event->key() == Qt::Key_PageDown) {
        license_text->verticalScrollBar()->setValue(license_text->verticalScrollBar()->value()+500);
    }
    else if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        if(m_nextBtn->isEnabled() == true)
            //clickNextButton();
            emit enterpressed();
            //this->releaseKeyboard();
    }
    else if(event->key() == Qt::Key_Backspace)
    {
        emit backspacepressed();
        //this->releaseKeyboard();
    }
    else if(event->key() == Qt::Key_Escape)
    {
        this->setFocus();
    }
    else
        QWidget::keyPressEvent(event);
}

void LicenseFrame::setUploadState(bool isSet)//写dbus总线: 1收集；0不收集
{
    int nStatus = 0;
//    int strDate;
    QDBusMessage messageCallStats = QDBusMessage::createMethodCall("com.lingmo.daq", "/com/lingmo/daq",
                                                           "com.lingmo.daq.interface","SetUploadState");
    if (isSet)
        nStatus = 1;
    else
        nStatus = 0;

    messageCallStats << nStatus;
    QDBusMessage messageResponseStats = QDBusConnection::systemBus().call(messageCallStats);
    if (messageResponseStats.type() == QDBusMessage::ReplyMessage) {
        qDebug() << "Set dbus SetUploadState["<< nStatus <<"] successfully";
    } else {
        qDebug() << "Set dbus SetUploadState error：" << QDBusConnection::systemBus().lastError().message();
    }
}

bool LicenseFrame::getUploadState(int& status)
{
    QDBusMessage messageCallStats = QDBusMessage::createMethodCall("com.lingmo.daq", "/com/lingmo/daq",
                                                           "com.lingmo.daq.interface","GetUploadState");
    QDBusMessage messageResponseStats = QDBusConnection::systemBus().call(messageCallStats);
    if (messageResponseStats.type() == QDBusMessage::ReplyMessage) {
        status = messageResponseStats.arguments().takeFirst().toInt();
        qDebug() << "Get dbus GetUploadState successfully:"<<status;
        return true;
    } else {
        qDebug() << "Get dbus GetUploadState error：" << QDBusConnection::systemBus().lastError().message();
        return false;
    }
}

}
