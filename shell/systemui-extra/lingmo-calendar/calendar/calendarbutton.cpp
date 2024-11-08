/*
* Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "calendarbutton.h"
#include <QStylePainter>
#include <QStyleOptionButton>
#include <QApplication>
#include <QToolTip>
#include "lingmosdk/applications/windowmanager/windowmanager.h"
#include <QFile>


#define SYSTEM_FONT_SET  "org.lingmo.style"
#define SYSTEM_FONT_SIZE "systemFontSize"
#define SYSTEM_FONT      "systemFont"

#define CALENDAR_BUTTON_WIDTH  120

#define LINGMOSDK_TIMERSERVER "com.lingmo.lingmosdk.TimeServer"
#define LINGMOSDK_TIMERPATH "/com/lingmo/lingmosdk/Timer"
#define LINGMOSDK_TIMERINTERFACE "com.lingmo.lingmosdk.TimeInterface"

#define LINGMOSDK_DATESERVER "com.lingmo.lingmosdk.DateServer"
#define LINGMOSDK_DATEPATH "/com/lingmo/lingmosdk/Date"
#define LINGMOSDK_DATEINTERFACE "com.lingmo.lingmosdk.DateInterface"

#define  LINGMO_CONTROL_CENTER_SERVER     "org.freedesktop.Accounts"
#define  LINGMO_CONTROL_CENTER_PATH       "/org/freedesktop/Accounts/User1000"
#define  LINGMO_CONTROL_CENTER_INTERFACE  "org.freedesktop.Accounts.User"

#define PANEL_SIZE_LARGE  92
#define PANEL_SIZE_MEDIUM 70
#define PANEL_SIZE_SMALL  46
#define PANEL_SIZE_KEY    "panelsize"

CalendarButton::CalendarButton(ILINGMOPanelPlugin *plugin, QWidget *parent):
    QPushButton(parent),
    m_parent(parent),
    m_plugin(plugin)
{
    //设置按钮样式和属性
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setFlat(true);
    setProperty("useButtonPalette",true);
    setSystemStyle();

    /*按钮边距固定0px，尽可能多显示日期信息。CalendarButton不继承ToolButton的原因就是
    主题框架提供的ToolButton的边距是固定值，无法修改*/
    this->setStyleSheet("padding: 0px;}");
    const QByteArray id(ORG_LINGMO_STYLE);
    if (QGSettings::isSchemaInstalled(id)) {
        m_styleGsettings = new QGSettings(id);
        connect(m_styleGsettings, &QGSettings::changed, this, [=] (const QString &key) {
            if (key == STYLE_NAME) {
                setSystemStyle();
                update();
            }
        });
    }

    //初始化系统字体相关Gsetting
    initFontGsettings();

    //使用系统提供的sdk刷新时间显示
    QDBusConnection::systemBus().connect(LINGMOSDK_TIMERSERVER,
                                         LINGMOSDK_TIMERPATH,
                                         LINGMOSDK_TIMERINTERFACE,
                                         "TimeSignal",
                                         this,
                                         SLOT(updateBtnText(QString))
                                         );
    QDBusConnection::systemBus().connect(LINGMOSDK_TIMERSERVER,
                                         LINGMOSDK_TIMERPATH,
                                         LINGMOSDK_TIMERINTERFACE,
                                         "TimeChangeSignal",
                                         this,
                                         SLOT(updateBtnText(QString))
                                         );

    QDBusConnection::sessionBus().connect(LINGMOSDK_DATESERVER,
                                         LINGMOSDK_DATEPATH,
                                         LINGMOSDK_DATEINTERFACE,
                                         "TimeSignal",
                                         this,
                                         SLOT(updateBtnText(QString))
                                         );

    QDBusConnection::sessionBus().connect(LINGMOSDK_DATESERVER,
                                         LINGMOSDK_DATEPATH,
                                         LINGMOSDK_DATEINTERFACE,
                                         "DateSignal",
                                         this,
                                         SLOT(updateBtnText(QString))
                                         );

    m_listenGsettings = new ListenGsettings();
    QObject::connect(m_listenGsettings,&ListenGsettings::iconsizechanged,[this]{updateBtnText(QString());});
    QObject::connect(m_listenGsettings,&ListenGsettings::panelpositionchanged,[this]{updateBtnText(QString());});

    //更新日历按钮内容
    updateBtnText(QString());
}

CalendarButton::~CalendarButton()
{
    if(m_styleGsettings) {
        m_styleGsettings->deleteLater();
    }
    if(m_fontGsettings) {
        m_fontGsettings->deleteLater();
    }
    if(m_listenGsettings) {
        m_listenGsettings->deleteLater();
    }
}

void CalendarButton::contextMenuEvent(QContextMenuEvent *event)
{
    m_menuCalender = new QMenu();
    m_menuCalender->setAttribute(Qt::WA_DeleteOnClose);
    m_menuCalender->setGeometry(m_plugin->panel()->calculatePopupWindowPos
                              (mapToGlobal(event->pos()),
                               m_menuCalender->sizeHint()));
    m_menuCalender->addAction(QIcon::fromTheme("document-page-setup-symbolic"),
                   tr("Time and Date Setting"),
                   this, SLOT(setControlTime()));
    m_menuCalender->show();
    kdk::WindowManager::setGeometry(m_menuCalender->windowHandle(),
                                        m_plugin->panel()->calculatePopupWindowPos(mapToGlobal(event->pos()), m_menuCalender->sizeHint()));

    QObject::connect(m_menuCalender, &QMenu::destroyed, this, [&](){
        this->setAttribute(Qt::WA_UnderMouse, false);
        this->setDown(false);
        this->update();
    });

}

void CalendarButton::mousePressEvent(QMouseEvent *event)
{
    if (Qt::LeftButton == event->button()){
        Q_EMIT pressShowHideCalendar();
    }
}

//初始化系统字体相关Gsetting
void CalendarButton::initFontGsettings()
{
    const QByteArray id(SYSTEM_FONT_SET);
    if (QGSettings::isSchemaInstalled(id)) {
        m_fontGsettings = new QGSettings(id);
        connect(m_fontGsettings, &QGSettings::changed, this, [=] (const QString &keys){
            if(keys == SYSTEM_FONT_SIZE || keys == SYSTEM_FONT){
                m_systemFontSize = m_fontGsettings->get(SYSTEM_FONT_SIZE).toString();
                m_systemFont = m_fontGsettings->get(SYSTEM_FONT).toString();
                updateBtnText(QString());
            }
        });

        QStringList ketList = m_fontGsettings->keys();
        if(ketList.contains(SYSTEM_FONT_SIZE))
            m_systemFontSize = m_fontGsettings->get(SYSTEM_FONT_SIZE).toString();
        if(ketList.contains(SYSTEM_FONT))
            m_systemFont = m_fontGsettings->get(SYSTEM_FONT).toString();
    }
}

//右键跳转到控制面板日期页面
void CalendarButton::setControlTime()
{
    QProcess *process =new QProcess(this);
    process->start(
            "bash",
            QStringList() << "-c"
                          << "dpkg -l | grep lingmo-control-center");
    process->waitForFinished();
    QString strResult = process->readAllStandardOutput() + process->readAllStandardError();
    if (-1 != strResult.indexOf("3.0")) {
        QProcess::startDetached(QString("lingmo-control-center -t"));
    } else {
        QProcess::startDetached(QString("lingmo-control-center -m Date"));
    }
}

void CalendarButton::updateBtnText(QString timerStr)
{
    Q_UNUSED(timerStr);
    CalendarButtonText btnText(m_plugin,this);
    this->setText(btnText.getBtnText());
}

void CalendarButton::setSystemStyle()
{
    QPalette pal = this->palette();
    QColor col = pal.color(QPalette::Active, QPalette::ButtonText);
    col.setAlphaF(0.05);
    pal.setColor(QPalette::Button, col);
    this->setPalette(pal);
}

bool CalendarButton::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {
        QString toolTipText = SDK2CPP(kdk_system_get_longformat_date)();
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        QToolTip::showText(helpEvent->globalPos(), toolTipText, this);
        return true;
    }
    return QWidget::event(event);
}
