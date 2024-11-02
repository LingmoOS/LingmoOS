/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Copyright: 2012-2013 Razor team
 * Authors:
 *   Kuzma Shapran <kuzma.shapran@gmail.com>
 *
 * Copyright: 2019 Tianjin LINGMO Information Technology Co., Ltd. *
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "lingmocalendar.h"

#include <QCalendarWidget>
#include <QDate>
#include <QDesktopWidget>
#include <QDialog>
#include <QEvent>
#include <QHBoxLayout>
#include <QLocale>
#include <QScopedArrayPointer>
#include <QTimer>
#include <QWheelEvent>
#include <QProcess>
#include "../panel/pluginsettings.h"
#include <QDebug>
#include <QApplication>
#include <QStyleOption>
#include <glib.h>
#include <gio/gio.h>
#include <QSize>
#include <QScreen>
#include <QSettings>
#include <QDir>

#define CALENDAR_HEIGHT (46)
#define CALENDAR_WIDTH (104)

#define WEBVIEW_WIDTH (454)
#define WEBVIEW_MAX_HEIGHT (704)
#define WEBVIEW_MIN_HEIGHT (600)
#define POPUP_BORDER_SPACING 4
#define HOUR_SYSTEM_CONTROL "org.lingmo.control-center.panel.plugins"
#define HOUR_SYSTEM_24_Horizontal "hh:mm ddd  yyyy/MM/dd"
#define HOUR_SYSTEM_24_Vertical "hh:mm ddd  MM/dd"
#define HOUR_SYSTEM_12_Horizontal   "Ahh:mm ddd  yyyy/MM/dd"
#define HOUR_SYSTEM_12_Vertical   "Ahh:mm ddd  MM/dd"
#define CURRENT_DATE "yyyy/MM/dd dddd"

#define HOUR_SYSTEM_24_Horizontal_CN "hh:mm ddd  yyyy-MM-dd"
#define HOUR_SYSTEM_24_Vertical_CN "hh:mm ddd  MM-dd"
#define HOUR_SYSTEM_12_Horizontal_CN   "Ahh:mm ddd  yyyy-MM-dd"
#define HOUR_SYSTEM_12_Vertical_CN   "Ahh:mm ddd  MM-dd"
#define CURRENT_DATE_CN "yyyy-MM-dd dddd"

#define HOUR_SYSTEM_KEY  "hoursystem"
#define SYSTEM_FONT_SIZE "systemFontSize"
#define SYSTEM_FONT      "systemFont"
#define SYSTEM_FONT_SET  "org.lingmo.style"

#define LINGMOSDK_TIMERSERVER "com.lingmo.lingmosdk.TimeServer"
#define LINGMOSDK_TIMERPATH "/com/lingmo/lingmosdk/Timer"
#define LINGMOSDK_TIMERINTERFACE "com.lingmo.lingmosdk.TimeInterface"

QString calendar_version;
extern LingmoUIWebviewDialogStatus status;

IndicatorCalendar::IndicatorCalendar(const ILINGMOPanelPluginStartupInfo &startupInfo):
    QWidget(),
    ILINGMOPanelPlugin(startupInfo),
    mTimer(new QTimer(this)),
    mCheckTimer(new QTimer(this)),
    mUpdateInterval(1),
    mbActived(false),
    mbHasCreatedWebView(false),
    mViewWidht(WEBVIEW_WIDTH),
    mViewHeight(0),
    mWebViewDiag(NULL)
{
    translator();
    mMainWidget = new QWidget();
    mContent = new CalendarActiveLabel(this);
    mWebViewDiag = new LingmoUIWebviewDialog(this);

    QVBoxLayout *borderLayout = new QVBoxLayout(this);

    mLayout = new UKUi::GridLayout(mMainWidget);
    setLayout(mLayout);
    mLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->setSpacing(0);
    mLayout->setAlignment(Qt::AlignCenter);
    mLayout->addWidget(mContent);

    mContent->setObjectName(QLatin1String("WorldClockContent"));
    mContent->setAlignment(Qt::AlignCenter);

    mTimer->setTimerType(Qt::PreciseTimer);
    const QByteArray id(HOUR_SYSTEM_CONTROL);
    if(QGSettings::isSchemaInstalled(id)){
        gsettings = new QGSettings(id);
        connect(gsettings, &QGSettings::changed, this, [=] (const QString &keys){
                updateTimeText();
        });
        if(QString::compare(gsettings->get("date").toString(),"cn"))
        {
                hourSystem_24_horzontal=HOUR_SYSTEM_24_Horizontal_CN;
                hourSystem_24_vartical=HOUR_SYSTEM_24_Vertical_CN;
                hourSystem_12_horzontal=HOUR_SYSTEM_12_Horizontal_CN;
                hourSystem_12_vartical=HOUR_SYSTEM_12_Vertical_CN;
                current_date=CURRENT_DATE_CN;
        }
        else
        {
            hourSystem_24_horzontal=HOUR_SYSTEM_24_Horizontal;
            hourSystem_24_vartical=HOUR_SYSTEM_24_Vertical;
            hourSystem_12_horzontal=HOUR_SYSTEM_12_Horizontal;
            hourSystem_12_vartical=HOUR_SYSTEM_12_Vertical;
            current_date=CURRENT_DATE;
        }
    } else {
        hourSystem_24_horzontal=HOUR_SYSTEM_24_Horizontal_CN;
        hourSystem_24_vartical=HOUR_SYSTEM_24_Vertical_CN;
        hourSystem_12_horzontal=HOUR_SYSTEM_12_Horizontal_CN;
        hourSystem_12_vartical=HOUR_SYSTEM_12_Vertical_CN;
        current_date=CURRENT_DATE_CN;
    }

    //六小时会默认刷新时间
    mCheckTimer->setInterval(60*60*1000);
    connect(mCheckTimer, &QTimer::timeout, [this]{checkUpdateTime();});
    mCheckTimer->start();

    connect(mTimer, &QTimer::timeout, [this]{checkUpdateTime();});
    mTimer->start(1000);

    const QByteArray _id(SYSTEM_FONT_SET);
    fgsettings = new QGSettings(_id);
    connect(fgsettings, &QGSettings::changed, this, [=] (const QString &keys){
        if(keys == SYSTEM_FONT_SIZE || keys == SYSTEM_FONT){
            updateTimeText();
        }
    });

    connect(mWebViewDiag, SIGNAL(deactivated()), SLOT(hidewebview()));
    if(QGSettings::isSchemaInstalled(id)) {
        connect(gsettings, &QGSettings::changed, this, [=] (const QString &key)
        {
            if (key == HOUR_SYSTEM_KEY)
            {
                if(gsettings->keys().contains("hoursystem"))
                {
                    hourSystemMode=gsettings->get("hoursystem").toString();
                }
                else
                    hourSystemMode=24;
            }
            if(key == "calendar")
            {

                mbHasCreatedWebView = false;
                initializeCalendar();
            }
            if(key == "firstday")
            {
                qDebug()<<"key == firstday";
                mbHasCreatedWebView = false;
                initializeCalendar();
            }
            if(key == "date")
            {
                if(gsettings->keys().contains("date"))
                {
                    if(QString::compare(gsettings->get("date").toString(),"cn"))
                    {
                        hourSystem_24_horzontal=HOUR_SYSTEM_24_Horizontal_CN;
                        hourSystem_24_vartical=HOUR_SYSTEM_24_Vertical_CN;
                        hourSystem_12_horzontal=HOUR_SYSTEM_12_Horizontal_CN;
                        hourSystem_12_vartical=HOUR_SYSTEM_12_Vertical_CN;
                        current_date=CURRENT_DATE_CN;
                    }
                    else
                    {
                        hourSystem_24_horzontal=HOUR_SYSTEM_24_Horizontal;
                        hourSystem_24_vartical=HOUR_SYSTEM_24_Vertical;
                        hourSystem_12_horzontal=HOUR_SYSTEM_12_Horizontal;
                        hourSystem_12_vartical=HOUR_SYSTEM_12_Vertical;
                        current_date=CURRENT_DATE;
                    }
                }
                updateTimeText();
            }
        });
    }
    connect(mContent,&CalendarActiveLabel::pressTimeText,[=]{CalendarWidgetShow();});

//    initializeCalendar();
    setTimeShowStyle();
    mContent->setWordWrap(true);

    ListenGsettings *m_ListenGsettings = new ListenGsettings();
    QObject::connect(m_ListenGsettings,&ListenGsettings::iconsizechanged,[this]{updateTimeText();});
    QObject::connect(m_ListenGsettings,&ListenGsettings::panelpositionchanged,[this]{updateTimeText();});
    updateTimeText();
    QTimer::singleShot(10000,[this] { updateTimeText();});

    //读取配置文件中CalendarVersion 的值
    QString filename ="/usr/share/lingmo/lingmo-panel/panel-commission.ini";
    QSettings m_settings(filename, QSettings::IniFormat);
    m_settings.setIniCodec("UTF-8");

    m_settings.beginGroup("Calendar");
    calendar_version = m_settings.value("CalendarVersion", "").toString();
    if (calendar_version.isEmpty()) {
        calendar_version = "old";
    }
    m_settings.endGroup();

    //监听手动更改时间,后期找到接口进行替换
    QTimer::singleShot(1000,this,[=](){ListenForManualSettingTime();});

    //使用系统提供的sdk刷新时间显示
    QDBusConnection::systemBus().connect(LINGMOSDK_TIMERSERVER,
                                         LINGMOSDK_TIMERPATH,
                                         LINGMOSDK_TIMERINTERFACE,
                                         "TimeSignal",
                                         this,
                                         SLOT(timeChange(QString))
                                         );

    QDBusConnection::systemBus().connect(LINGMOSDK_TIMERSERVER,
                                         LINGMOSDK_TIMERPATH,
                                         LINGMOSDK_TIMERINTERFACE,
                                         "TimeChangeSignal",
                                         this,
                                         SLOT(timeChange(QString))
                                         );

}

IndicatorCalendar::~IndicatorCalendar()
{
    if(mMainWidget != NULL)
    {
        mMainWidget->deleteLater();
    }
    if(mWebViewDiag != NULL)
    {
        mWebViewDiag->deleteLater();
    }
    if(mContent != NULL)
    {
        mContent->deleteLater();
    }
    gsettings->deleteLater();
    fgsettings->deleteLater();
}

void IndicatorCalendar::translator(){
    m_translator = new QTranslator(this);
     QString locale = QLocale::system().name();
     if (locale == "zh_CN"){
         if (m_translator->load(QM_INSTALL))
             qApp->installTranslator(m_translator);
         else
             qDebug() <<PLUGINNAME<<"Load translations file" << locale << "failed!";
     } else if (locale == "bo_CN") {
         if (m_translator->load(BO_QM_INSTALL)) {
             qApp->installTranslator(m_translator);
         } else {
             qDebug() <<PLUGINNAME<<"Load translations file" << locale << "failed!";
         }
     }
}

void IndicatorCalendar::checkUpdateTime()
{
     QDateTime tzNow = QDateTime::currentDateTime();
    if(QString::compare(tzNow.toString("hh:mm ddd  yyyy-MM-dd"),timeState) == 0) {
        return;
    }

    //任务栏第一次启动与系统时间进行比较校正，确定第一次刷新时间
    QString delaytime=QTime::currentTime().toString();
    QList<QString> pathresult=delaytime.split(":");
    int second=pathresult.at(2).toInt();
    if(second==0){
        mTimer->setInterval(60*1000);
    }else{
        mTimer->setInterval((60+1-second)*1000);
    }

    timeState = tzNow.toString("hh:mm ddd  yyyy-MM-dd");
    updateTimeText();
}




void IndicatorCalendar::updateTimeText()
{


    QDateTime tzNow = QDateTime::currentDateTime();

    QString str;
    QByteArray id(HOUR_SYSTEM_CONTROL);
    if(QGSettings::isSchemaInstalled(id))
    {
        QStringList keys = gsettings->keys();
        if(keys.contains("hoursystem"))
            hourSystemMode=gsettings->get("hoursystem").toString();
    } else {
        hourSystemMode = 24;
    }

    if(!QString::compare("24",hourSystemMode))
    {
        if(panel()->isHorizontal())
            str=tzNow.toString(hourSystem_24_horzontal);
        else
            str=tzNow.toString(hourSystem_24_vartical);
    }
    else
    {
        if(panel()->isHorizontal())
        {
            str=tzNow.toString(hourSystem_12_horzontal);
        }
        else
        {
            str = tzNow.toString(hourSystem_12_vartical);
            str.replace("AM","AM ");
            str.replace("PM","PM ");
        }
    }

    QString style;
    int font_size = fgsettings->get(SYSTEM_FONT_SIZE).toInt();
    if(font_size>14) font_size=14;
    if(font_size<12) font_size=12;
    style.sprintf( //正常状态样式
                   "QLabel{"
                   "border-width:  0px;"                     //边框宽度像素
                   "border-radius: 6px;"                       //边框圆角半径像素
                   "font-size:     %dpx;"                      //字体，字体大小
                   "padding:       0px;"                       //填衬
                   "text-align:center;"                        //文本居中
                   "}"
                   //鼠标悬停样式
                   "QLabel:hover{"
                   "background-color:rgba(190,216,239,20%%);"
                   "border-radius:6px;"                       //边框圆角半径像素
                   "}"
                   //鼠标按下样式
                   "QLabel:pressed{"
                   "background-color:rgba(190,216,239,12%%);"
                   "}", font_size);
    mContent->setStyleSheet(style);
    mContent->setText(str);
}

/*when widget is loading need initialize here*/
void IndicatorCalendar::initializeCalendar()
{
    QByteArray id(HOUR_SYSTEM_CONTROL);
    CalendarShowMode showCalendar = defaultMode;
    QString lunarOrsolar;
    QString firstDay;
    int iScreenHeight = QApplication::screens().at(0)->size().height() - panel()->panelSize();
    if(iScreenHeight > WEBVIEW_MAX_HEIGHT)
    {
        mViewHeight = WEBVIEW_MAX_HEIGHT;
    }
    else
    {
        mViewHeight = WEBVIEW_MIN_HEIGHT;
    }
    if(QGSettings::isSchemaInstalled(id))
    {
        if(!gsettings)
        {
            qDebug()<<"get gsetting error!!!";
            return;
        }
        if(gsettings->keys().contains("calendar"))
        {
            lunarOrsolar= gsettings->get("calendar").toString();
        }
        if(gsettings->keys().contains("firstday"))
        {
            firstDay= gsettings->get("firstday").toString();
        }
        if (QLocale::system().name() == "zh_CN")
        {

            if(lunarOrsolar == "lunar")
            {
                if(firstDay == "sunday")
                {
                    showCalendar = lunarSunday;
                }
                else if(firstDay == "monday")
                {
                    showCalendar = lunarMonday;
                }
                if(iScreenHeight > WEBVIEW_MAX_HEIGHT)
                {
                    mViewHeight = WEBVIEW_MAX_HEIGHT;
                }
                else
                {
                    mViewHeight = WEBVIEW_MIN_HEIGHT;
                }
            }
            else if(lunarOrsolar == "solarlunar")
            {
                if(firstDay == "sunday")
                {
                    showCalendar = solarSunday;
                }
                else if(firstDay == "monday")
                {
                    showCalendar = solarMonday;
                }
                mViewHeight = WEBVIEW_MIN_HEIGHT;
            }
        }
        else// for internaitional
        {
            if(firstDay == "sunday")
            {
                showCalendar = solarSunday;
            }
            else if(firstDay == "monday")
            {
                showCalendar = solarMonday;
            }
            mViewHeight = WEBVIEW_MIN_HEIGHT;
        }
    }

    if(mWebViewDiag != NULL )
    {
        if(!mbHasCreatedWebView)
        {
            mWebViewDiag->creatwebview(showCalendar,panel()->panelSize());
            mbHasCreatedWebView = true;
        }
    }
}

void IndicatorCalendar::CalendarWidgetShow()
{
    if(mWebViewDiag != NULL )
    {
        mViewHeight = WEBVIEW_MAX_HEIGHT;
        QByteArray id(HOUR_SYSTEM_CONTROL);
        if(QGSettings::isSchemaInstalled(id)) {
            if(gsettings->get("calendar").toString() == "solarlunar")
             mViewHeight = WEBVIEW_MIN_HEIGHT;
        }
        if (QLocale::system().name() != "zh_CN")
            mViewHeight = WEBVIEW_MIN_HEIGHT;
        int iScreenHeight = QApplication::screens().at(0)->size().height() - panel()->panelSize();
        if (iScreenHeight < WEBVIEW_MAX_HEIGHT) {
            mViewHeight = iScreenHeight;
            if (iScreenHeight >= WEBVIEW_MIN_HEIGHT)
                mViewHeight = WEBVIEW_MIN_HEIGHT;;
        }
        if(qgetenv("XDG_SESSION_TYPE")=="wayland") mWebViewDiag->setGeometry(calculatePopupWindowPos(QSize(mViewWidht+POPUP_BORDER_SPACING,mViewHeight+POPUP_BORDER_SPACING)));
        else modifyCalendarWidget();
#if 0
        mWebViewDiag->show();
        mWebViewDiag->activateWindow();
        if(!mbActived)
        {
            mWebViewDiag->setHidden(false);
//            mWebViewDiag->webview()->reload();
            mbActived = true;
        }
        else
        {
            mWebViewDiag->setHidden(true);
//            mWebViewDiag->webview()->reload();
            mbActived = false;
        }
#endif
        if(status==ST_HIDE)
        {
            status = ST_SHOW;
            mWebViewDiag->setHidden(false);
        }
        else
        {
            status = ST_HIDE;
            mWebViewDiag->setHidden(true);
        }
    }
}
/**
 * @brief IndicatorCalendar::activated
 * @param reason
 * 如下两种方式也可以设置位置，由于ui问题弃用
 * 1.mWebViewDiag->setGeometry(calculatePopupWindowPos(QSize(mViewWidht+POPUP_BORDER_SPACING,mViewHeight+POPUP_BORDER_SPACING)));
 * 2.
//        QRect screen = QApplication::desktop()->availableGeometry();
//        switch (panel()->position()) {
//        case ILINGMOPanel::PositionBottom:
//            mWebViewDiag->move(screen.width()-mViewWidht-POPUP_BORDER_SPACING,screen.height()-mViewHeight-POPUP_BORDER_SPACING);
//            break;
//        case ILINGMOPanel::PositionTop:
//            mWebViewDiag->move(screen.width()-mViewWidht-POPUP_BORDER_SPACING,panel()->panelSize()+POPUP_BORDER_SPACING);
//            break;
//        case ILINGMOPanel::PositionLeft:
//            mWebViewDiag->move(panel()->panelSize()+POPUP_BORDER_SPACING,screen.height()-mViewHeight-POPUP_BORDER_SPACING);
//            break;
//        default:
//            mWebViewDiag->setGeometry(calculatePopupWindowPos(QSize(mViewWidht+POPUP_BORDER_SPACING,mViewHeight+POPUP_BORDER_SPACING)));
//            break;
//        }
 */

void IndicatorCalendar::hidewebview()
{
    mWebViewDiag->setHidden(true);
    mbActived = false;
}

void IndicatorCalendar::realign()
{
    setTimeShowStyle();
}

void IndicatorCalendar::setTimeShowStyle()
{
    int size = panel()->panelSize() - 3;
    if (size > 0) {
        if(panel()->isHorizontal())
        {
           mContent->setFixedSize(CALENDAR_WIDTH, size);
        }
        else
        {
           mContent->setFixedSize(size, CALENDAR_WIDTH);
        }
    }
}

/**
 * @brief IndicatorCalendar::modifyCalendarWidget
 * 任务栏上弹出窗口的位置标准为距离屏幕边缘及任务栏边缘分别为4像素
 */
void IndicatorCalendar::modifyCalendarWidget()
{
       int totalHeight = qApp->primaryScreen()->size().height() + qApp->primaryScreen()->geometry().y();
       int totalWidth = qApp->primaryScreen()->size().width() + qApp->primaryScreen()->geometry().x();

       switch (panel()->position()) {
       case ILINGMOPanel::PositionBottom:
           mWebViewDiag->setGeometry(totalWidth-mViewWidht-4,totalHeight-panel()->panelSize()-mViewHeight-4,mViewWidht,mViewHeight);
           break;
       case ILINGMOPanel::PositionTop:
           mWebViewDiag->setGeometry(totalWidth-mViewWidht-4,qApp->primaryScreen()->geometry().y()+panel()->panelSize()+4,mViewWidht,mViewHeight);
           break;
       case ILINGMOPanel::PositionLeft:
           mWebViewDiag->setGeometry(qApp->primaryScreen()->geometry().x()+panel()->panelSize()+4,totalHeight-mViewHeight-4,mViewWidht,mViewHeight);
           break;
       case ILINGMOPanel::PositionRight:
           mWebViewDiag->setGeometry(totalWidth-panel()->panelSize()-mViewWidht-4,totalHeight-mViewHeight-4,mViewWidht,mViewHeight);
           break;
       default:
           mWebViewDiag->setGeometry(qApp->primaryScreen()->geometry().x()+panel()->panelSize()+4,totalHeight-mViewHeight,mViewWidht,mViewHeight);
           break;
       }
}

void IndicatorCalendar::ListenForManualSettingTime(){
    mProcess=new QProcess(this);
    QString command="journalctl -u systemd-timedated.service -f";
    mProcess->setReadChannel(QProcess::StandardOutput);
    mProcess->start(command);
    mProcess->startDetached(command);

    connect(mProcess,&QProcess::readyReadStandardOutput,this,[=](){
        updateTimeText();
    });
}

void IndicatorCalendar::timeChange(QString time)
{
    updateTimeText();
}

CalendarActiveLabel::CalendarActiveLabel(ILINGMOPanelPlugin *plugin, QWidget *parent) :
    QLabel(parent),
    mPlugin(plugin),
    mInterface(new QDBusInterface(SERVICE,PATH,INTERFACE,QDBusConnection::sessionBus(),this))
{
    w = new frmLunarCalendarWidget();
    connect(w,&frmLunarCalendarWidget::yijiChangeDown, this, [=] (){
        changeHight = 0;
        changeWidowpos();

    });
    connect(w,&frmLunarCalendarWidget::yijiChangeUp, this, [=] (){
        changeHight = 52;
        changeWidowpos();
    });

    QTimer::singleShot(1000,[this] {setToolTip(tr("Time and Date")); });
}

void CalendarActiveLabel::mousePressEvent(QMouseEvent *event)
{
    if (Qt::LeftButton == event->button()){
        if(calendar_version == "old"){
            Q_EMIT pressTimeText();
        } else {
            //点击时间标签日历隐藏，特殊处理
            if(w->isHidden()){
                changeWidowpos();
            }else{
                w->hide();
            }
        }
//        mInterface->call("ShowCalendar");
        
    }
}

void CalendarActiveLabel::changeWidowpos()
{
    int totalHeight = qApp->primaryScreen()->size().height() + qApp->primaryScreen()->geometry().y();
    int totalWidth = qApp->primaryScreen()->size().width() + qApp->primaryScreen()->geometry().x();
    switch (mPlugin->panel()->position()) {
    case ILINGMOPanel::PositionBottom:
        w->setGeometry(totalWidth-mViewWidht-4,totalHeight-mPlugin->panel()->panelSize()-mViewHeight-4-changeHight,mViewWidht,mViewHeight);
        break;
    case ILINGMOPanel::PositionTop:
        w->setGeometry(totalWidth-mViewWidht-4,qApp->primaryScreen()->geometry().y()+mPlugin->panel()->panelSize()+4,mViewWidht,mViewHeight);
        break;
    case ILINGMOPanel::PositionLeft:
        w->setGeometry(qApp->primaryScreen()->geometry().x()+mPlugin->panel()->panelSize()+4,totalHeight-mViewHeight-4-changeHight,mViewWidht,mViewHeight);
        break;
    case ILINGMOPanel::PositionRight:
        w->setGeometry(totalWidth-mPlugin->panel()->panelSize()-mViewWidht-4,totalHeight-mViewHeight-4-changeHight,mViewWidht,mViewHeight);
        break;
    default:
        w->setGeometry(qApp->primaryScreen()->geometry().x()+mPlugin->panel()->panelSize()+4,totalHeight-mViewHeight,mViewWidht,mViewHeight);
        break;
    }
    w->show();
}

void CalendarActiveLabel::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menuCalender=new QMenu(this);
    menuCalender->setAttribute(Qt::WA_DeleteOnClose);

    menuCalender->addAction(QIcon::fromTheme("document-page-setup-symbolic"),
                   tr("Time and Date Setting"),
                   this, SLOT(setControlTime())
                  );
    menuCalender->setGeometry(mPlugin->panel()->calculatePopupWindowPos(mapToGlobal(event->pos()), menuCalender->sizeHint()));
    menuCalender->show();
}

void CalendarActiveLabel::setControlTime()
{
    QProcess::startDetached(QString("lingmo-control-center -m Date"));
}
