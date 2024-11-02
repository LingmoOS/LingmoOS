/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU  Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */
#include "lunarcalendarwidget.h"
#include "calendarcolor.h"
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <unistd.h>
#include<QDebug>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QJsonParseError>
#include <QJsonObject>
#include <QProcess>
#include <QContextMenuEvent>
#include <QGesture>
#include <KWindowEffects>
#include <KWindowSystem>
#include <kpushbutton.h>
#include <QGuiApplication>
#include <QHeaderView>
#include "colorarea.h"
#include <QDesktopWidget>
#include <QScreen>
#include <lingmostylehelper/lingmostylehelper.h>
//#include "schedulewidget.h"
//#include "schedulemark.h"
#include "schedule_item.h"
#include <libkydate.h>
#include "morelabel.h"
#include "frmlunarcalendarwidget.h"
#include <QVector>
#include <QSettings>
#include "custommessagebox.h"
#define TwoSchedule  170
#define OneSchdule  100
#define afterClickMore 600
#define CALENDAR_GSETTINGS_SCHEMAS "org.lingmo.calendar.plugin"
#define CALENDAR_LUNAR_DATE_KEY "lunar-date"
#define YIJISTATUS "yijistatus" //记录yiji的
#define PANEL_CONTROL_IN_CALENDAR "org.lingmo.control-center.panel.plugins"
#define LUNAR_KEY                 "calendar"
#define FIRST_DAY_KEY "firstday"
#define SDK2CPP(func) []()->QString { char* p = func(); QString str(p); free(p); return str; }
#define TRANSPARENCY_SETTING "org.lingmo.control-center.personalise"
#define TRANSPARENCY_KEY "transparency"

#define PANEL_SETTINGS      "org.lingmo.panel.settings"
#define PANEL_POSITION_KEY  "panelposition"
#define ICON_SIZE_KEY       "iconsize"
#define PANEL_SIZE_KEY      "panelsize"

#define SCROLLA 40
#define ORIGIN_CALENDAR_HEIGHT 600
#define LESSTWOSHCEDULE_CALENDAR_HEIGHT 740
#define ONE_CALENDAR_SCHEDULE 690
#define FENBIANLV 800
#define ORIGIN_CALENDAR_WIDTH 452
#define TWO_MORE_SCHEDULE_HEIGHT 845
#define THREE_SCHEDULE_HEIGHT 890
#define FOUR_MORE_SCHEDULE_HEIGHT 925

const int WIDGET_GAP = 8;
#define ORG_LINGMO_STYLE            "org.lingmo.style"
#define SYSTEM_FONT_SIZE "systemFontSize"
#define SYSTEM_FONT      "systemFont"
#define STYLE_NAME                "styleName"
#define STYLE_NAME_KEY_DARK       "lingmo-dark"
#define STYLE_NAME_KEY_DEFAULT    "lingmo-default"
#define STYLE_NAME_KEY_BLACK       "lingmo-black"
#define STYLE_NAME_KEY_LIGHT       "lingmo-light"
#define STYLE_NAME_KEY_WHITE       "lingmo-white"
//#define PANEL_SETTINGS "org.lingmo.panel.settings"
//#define PANEL_SIZE_KEY
//#define PANEL_POSITION_KEY
#define PANEL_SIZE_LARGE  92
#define PANEL_SIZE_MEDIUM 70
#define PANEL_SIZE_SMALL  46
#define PANEL_SIZE_KEY    "panelsize"
#define ICON_COLOR_LOGHT      255
#define ICON_COLOR_DRAK       0
#define THEME_SCHAME "org.lingmo.style"
#define COLOR_THEME "styleName"
#define LINGMOSDK_DATESERVER "com.lingmo.lingmosdk.DateServer"
#define LINGMOSDK_DATEPATH "/com/lingmo/lingmosdk/Date"
#define LINGMOSDK_DATEINTERFACE "com.lingmo.lingmosdk.DateInterface"
#include "customscrollarea.h"
#define YIJI_HEIGHT 55
#include "calendardatabase.h"
class CalendarDataBase;
class schedule_item;
QDate LunarCalendarWidget::s_clickDate = QDate();
LunarCalendarWidget::LunarCalendarWidget(QWidget *parent) : QWidget(parent)
{
    setObjectName("LunarCalendarWidget");
    installEventFilter(this);
    setMouseTracking(true);
    analysisWorktimeJs();
    int res;
    res = DATABASE_INSTANCE.initDataBase();
    if (res != DB_OP_SUCC)
    {
        qDebug() << "数据库加载失败" << __FILE__ << "," << __FUNCTION__ << "," << __LINE__;
    }
    CalendarDataBase::getInstance().deleteschedule();//删除已经过期的日程

    CalendarDataBase::getInstance().searchDataSlot(date);
    updateAllSchedule();
    if(QString(qgetenv("XDG_SESSION_TYPE")) == "wayland")
    {
        //        qputenv("QT_QPA_PLATFORM", "wayland"); //Lingmo
        isWayland = true;
    }
    else
    {
        isWayland = false;
    }
    btnYear = new QPushButton;
    btnMonth = new QPushButton;
    btnToday = new QPushButton;
    btnClick = false;
    isRightToLeftLanguage = isRightToLeft();
    calendarStyle = CalendarStyle_Red;
    date = QDate::currentDate();
    //    CalendarDataBase::getInstance().searchDataSlot(date);
    widgetTime = new QWidget();
    m_verLayoutTopLeft = new QVBoxLayout(widgetTime);
    datelabel = new QLabel(this);
    datelabel->setFixedSize(290, 30);
    timelabel = new QLabel(this);
    timelabel->setFixedSize(290, 35);
    lunarlabel = new QLabel(this);
    screen = QApplication::primaryScreen();
    primaryScreenHeight = qApp->screenAt(QCursor::pos())->size().height() + qApp->screenAt(QCursor::pos())->geometry().y();
    initGsettings();
    QString vertical = "#666666";
    QString hover = "#ffffff";
    if (styleGsettings != nullptr)
    {
        QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
        if(currentTheme == "lingmo-light")
        {
            vertical = "#ffffff";
            hover = "#999999";
        }
    }
    ScrollAreaStyleSheet = QString("\
                                   QScrollBar:vertical {\
                                       background: transparent; /* 滚动条背景颜色 */\
                                       width: 5px; /* 滚动条宽度 */\
                                       margin: 0px 0px 0px 0px; /* 外边距 */\
                                       border-radius: 10px; /* 滚动条的整体圆角 */\
                                   }\
                                   QScrollBar::handle:vertical {\
                                       background: %1; /* 滚动条滑块背景颜色 */\
                                       min-height: 30px; /* 滑块最小高度 */\
                                       border-radius: 10px; /* 滑块圆角半径 */\
                                   }\
                                   QScrollBar::handle:vertical:hover {\
                                       background: %2; /* 滚动条滑块背景颜色（悬停时）*/\
                                       min-height: 30px; /* 滑块最小高度 */\
                                       border-radius: 10px; /* 悬停时的滑块圆角半径 */\
                                   }\
                                   QScrollBar::add-line:vertical {\
                                       background: transparent; /* 上方按钮背景颜色 */\
                                       border-radius: 10px; /* 上方按钮圆角半径 */\
                                   }\
                                   QScrollBar::sub-line:vertical {\
                                       background: transparent; /* 下方按钮背景颜色 */\
                                       border-radius: 10px; /* 下方按钮圆角半径 */\
                                   }\
                                   QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {\
                                       background: transparent; /* 滚动条轨道背景颜色 */\
                                       border-radius: 10px; /* 轨道圆角半径 */\
                                   }").arg(vertical).arg(hover);
}

//原样式
void LunarCalendarWidget::noSchedule()
{
    m_scheduleWidget = new QWidget(this);
    m_scheduleWidget->setContentsMargins(0, 0, 0, 23);
    m_scheduleWidget->setStyleSheet("background-color: transparent;");
    m_scheduleWidget->setFixedSize(ORIGIN_CALENDAR_WIDTH, 100);
    //scrollArea->setFixedSize(ORIGIN_CALENDAR_WIDTH, 140);
    scrollArea->setFixedWidth(ORIGIN_CALENDAR_WIDTH);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scheduleWidget = new schedule_item(this, false);
    scheduleWidget->setStartTime("");
    scheduleWidget->setDescription("");
    labWidget->setContentsMargins(16, 0, 16, 0);
    yijiWidget->setContentsMargins(16, 0, 16, 0);
    item_layout->addWidget(scheduleWidget);
    item_layout->addSpacing(2);
    m_scheduleWidget->setLayout(item_layout);
    scrollArea->setWidget(m_scheduleWidget);
    if(panelHeight < this->height() || panelHeight < LESSTWOSHCEDULE_CALENDAR_HEIGHT || isExceedScreenHeight)
    {
        qDebug() << "分辨率支持";
        int height = qApp->screenAt(QCursor::pos())->size().height() + qApp->screenAt(QCursor::pos())->geometry().y();
        QPoint currentPos = this->pos();
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        this->setFixedSize(ORIGIN_CALENDAR_WIDTH, panelHeight - SCROLLA);
        m_scheduleWidget->setFixedHeight(100);
        //scrollArea->setFixedSize(ORIGIN_CALENDAR_WIDTH, height*0.1);
        scrollArea->setFixedWidth(ORIGIN_CALENDAR_WIDTH);
        this->move(currentPos.x(), SCROLLA - WIDGET_GAP);
        isExceedScreenHeight = false;
    }
    else
    {
        QPoint currentPos = this->pos();
        if(yijistate)
        {
            this->setFixedSize(ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT + YIJI_HEIGHT);
        }
        else
        {
            this->setFixedSize(ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT);
        }
        if(ScrollAreaNeedChange)
        {
            qDebug() << "ScrollAreaNeedChange";
            if(yijistate)
            {
                scrollArea->setFixedSize(420, 80);

            }
            else
            {
                //                scrollArea->setFixedSize(420,100);
                scrollArea->setFixedWidth(ORIGIN_CALENDAR_WIDTH);

            }
            //            scrollArea->setFixedWidth(ORIGIN_CALENDAR_WIDTH);
            this->setFixedSize(ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT);
            this->move(currentPos.x(), panelHeight - LESSTWOSHCEDULE_CALENDAR_HEIGHT - WIDGET_GAP);
        }
        else
        {
            QRect rect;
            int totalHeight = qApp->screenAt(QCursor::pos())->size().height() + qApp->screenAt(QCursor::pos())->geometry().y();//屏幕高度
            int totalWidth = qApp->screenAt(QCursor::pos())->size().width() + qApp->screenAt(QCursor::pos())->geometry().x();
            switch(position)
            {
            case 0:
                //                this->move(currentPos.x(), panelHeight - LESSTWOSHCEDULE_CALENDAR_HEIGHT - WIDGET_GAP);
                //                if(yijistate){
                //                    this->move(currentPos.x(), panelHeight - LESSTWOSHCEDULE_CALENDAR_HEIGHT -YIJI_HEIGHT - WIDGET_GAP);
                //                }

                if(yijistate)
                {
                    rect.setRect(totalWidth - ORIGIN_CALENDAR_WIDTH - WIDGET_GAP, totalHeight - panelSize - LESSTWOSHCEDULE_CALENDAR_HEIGHT - YIJI_HEIGHT - WIDGET_GAP,
                                 ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT + YIJI_HEIGHT);
                }
                else
                {
                    rect.setRect(totalWidth - ORIGIN_CALENDAR_WIDTH - WIDGET_GAP, totalHeight - panelSize - LESSTWOSHCEDULE_CALENDAR_HEIGHT - WIDGET_GAP,
                                 ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT);
                    if(isRTLLanguage())
                    {
                        qDebug() << "QLocale::system().language();" << QLocale::system().language();
                        rect.setRect(qApp->screenAt(QCursor::pos())->geometry().x() + WIDGET_GAP, totalHeight - LESSTWOSHCEDULE_CALENDAR_HEIGHT - WIDGET_GAP,
                                     ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT);
                    }
                }
                kdk::WindowManager::setGeometry(this->windowHandle(), rect);
                break;
            case 1:
                //this->move(currentPos.x(), m_panelGsettings->get(PANEL_SIZE_KEY).toInt() + WIDGET_GAP);
                //if(yijistate){
                //this->move(currentPos.x(), m_panelGsettings->get(PANEL_SIZE_KEY).toInt() + WIDGET_GAP);
                //}
                if(yijistate)
                {
                    rect.setRect(totalWidth - ORIGIN_CALENDAR_WIDTH - WIDGET_GAP, qApp->screenAt(QCursor::pos())->geometry().y() + panelSize + WIDGET_GAP,
                                 ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT + YIJI_HEIGHT);
                }
                else
                {
                    rect.setRect(totalWidth - ORIGIN_CALENDAR_WIDTH - WIDGET_GAP, qApp->screenAt(QCursor::pos())->geometry().y() + panelSize + WIDGET_GAP,
                                 ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT);
                }
                kdk::WindowManager::setGeometry(this->windowHandle(), rect);
                break;
            case 2:
                if(yijistate)
                {
                    rect.setRect(qApp->screenAt(QCursor::pos())->geometry().x() + panelSize + WIDGET_GAP, totalHeight - LESSTWOSHCEDULE_CALENDAR_HEIGHT - WIDGET_GAP - YIJI_HEIGHT,
                                 ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT + YIJI_HEIGHT);
                }
                else
                {
                    rect.setRect(qApp->screenAt(QCursor::pos())->geometry().x() + panelSize + WIDGET_GAP, totalHeight - LESSTWOSHCEDULE_CALENDAR_HEIGHT - WIDGET_GAP,
                                 ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT);
                }
                kdk::WindowManager::setGeometry(this->windowHandle(), rect);
                break;
            case 3:
                //                this->move(currentPos.x(), primaryScreenHeight - LESSTWOSHCEDULE_CALENDAR_HEIGHT - WIDGET_GAP);
                //                if(yijistate){
                //                    this->move(currentPos.x(), primaryScreenHeight - this->height() - WIDGET_GAP);

                //                }
                if(yijistate)
                {
                    rect.setRect(totalWidth - panelSize - ORIGIN_CALENDAR_WIDTH - WIDGET_GAP, totalHeight - LESSTWOSHCEDULE_CALENDAR_HEIGHT - WIDGET_GAP - YIJI_HEIGHT,
                                 ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT + YIJI_HEIGHT);
                }
                else
                {
                    rect.setRect(totalWidth - panelSize - ORIGIN_CALENDAR_WIDTH - WIDGET_GAP, totalHeight - LESSTWOSHCEDULE_CALENDAR_HEIGHT - WIDGET_GAP,
                                 ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT);
                }
                kdk::WindowManager::setGeometry(this->windowHandle(), rect);
                break;
            default:
                break;
            }
        }

    }
    m_horLayoutWidgetTop = new QHBoxLayout(this);
    m_horLayoutWidgetTop->setContentsMargins(26, 0, 24, 0);
    m_horLayoutWidgetTop->addWidget(widgetTime);
    m_horLayoutWidgetTop->addWidget(m_createScheduleButton);

    m_verLayoutCalendar->setMargin(0);
    m_verLayoutCalendar->setSpacing(0);
    m_verLayoutCalendar->addLayout(m_horLayoutWidgetTop);
    m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
    m_verLayoutCalendar->addWidget(lineUp);
    m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
    m_verLayoutCalendar->addWidget(m_widgetTop);
    m_verLayoutCalendar->addSpacing(18);
    m_verLayoutCalendar->addWidget(widgetWeek);
    m_verLayoutCalendar->addWidget(m_widgetDayBody, 1);
    m_verLayoutCalendar->addWidget(widgetYearBody, 1);
    m_verLayoutCalendar->addWidget(widgetmonthBody, 1);
    m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
    m_verLayoutCalendar->addWidget(lineDown);
    controlDownLine();
    m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
    m_verLayoutCalendar->addWidget(labWidget);
    m_verLayoutCalendar->addWidget(yijiWidget);
    m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
    lineSchedule->setFixedHeight(1);

    m_verLayoutCalendar->addWidget(lineSchedule);
    m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
    m_verLayoutCalendar->addWidget(scrollArea);


}
void LunarCalendarWidget::updateFromItem(QString id)
{
    QRect rect;
    CustomMessageBox *msgBox = new CustomMessageBox(1, this);
    msgBox->SetText(tr("Are you sure you want to delete this event?"));
    msgBox->setWindowFlags(msgBox->windowFlags() | Qt::WindowStaysOnTopHint);
    // 获取屏幕中心点和尺寸
    QPoint screenCenter = QApplication::desktop()->availableGeometry().center();
    QSize dialogSize = msgBox->size();
    // 计算对话框应显示的中心点坐标，并将其移动到屏幕中心
    //    msgBox->move(screenCenter.x() - dialogSize.width() / 2, screenCenter.y() - dialogSize.height() / 2);
    rect.setRect(screenCenter.x() - dialogSize.width() / 2, screenCenter.y() - dialogSize.height() / 2, dialogSize.width(), dialogSize.height());

    connect(msgBox, &CustomMessageBox::yesClicked, [&]()
    {
        foreach(MarkInfo info, m_markInfoList)
        {
            if (id == info.m_markId)
            {
                emit DATABASE_INSTANCE.deleteDataSignal(info);
                CalendarDataBase::getInstance().del(info);
                updateAllSchedule();
                updateLayout();
                break;
            }

        }
        delete msgBox;
    });
    connect(msgBox, &CustomMessageBox::noClicked, [&]()
    {
        delete msgBox;
    });
    msgBox->exec();
}
/**
 * @brief LunarCalendarWidget::twoLessSchedule
 * 两个日程以下的部分，但是如果分辨率过低的情况下，无论几个日程都会按照这个处理
 */
void LunarCalendarWidget::twoLessSchedule()
{

    int num = m_markInfoList.count();
    m_scheduleWidget = new QWidget(this);
    m_scheduleWidget->setContentsMargins(0, 0, 0, 23);
    m_scheduleWidget->setStyleSheet("background-color: transparent;");
    if(num == 1)
    {
        m_scheduleWidget->setFixedHeight(107);
        //scrollArea->setFixedSize(ORIGIN_CALENDAR_WIDTH, 140);//80是一个日程时候的日程区域高度
        scrollArea->setFixedWidth(ORIGIN_CALENDAR_WIDTH);
    }
    else if(num == 2)
    {
        m_scheduleWidget->setFixedHeight(170);
        //scrollArea->setFixedSize(ORIGIN_CALENDAR_WIDTH, 140);//140是2个日程时候的日程区域高度
        scrollArea->setFixedWidth(ORIGIN_CALENDAR_WIDTH);
    }
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //panelHeight 是除了任务栏之外的高度
    QPoint currentPos = this->pos();
    //如果需要调整滑动区域大小了 如果当前只有一个日程 无需滑动条，2个日程就需要了
    if(ScrollAreaNeedChange)
    {
        qDebug() << "ScrollAreaNeedChange";

        this->setFixedSize(ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT); //根据设计稿的高度，770
        this->move(currentPos.x(), panelHeight - LESSTWOSHCEDULE_CALENDAR_HEIGHT - WIDGET_GAP);

        if(yijistate)
        {
            scrollArea->setFixedSize(420, 80);
        }
        else
        {
            scrollArea->setFixedWidth(ORIGIN_CALENDAR_WIDTH);
        }
        //        scrollArea->setFixedWidth(ORIGIN_CALENDAR_WIDTH);
        if(num == 2)
        {
            scrollArea->verticalScrollBar()->setStyleSheet(ScrollAreaStyleSheet);

            scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);//滑动条样式还没处理
        }
    }
    else
    {
        if(yijistate)
        {
            this->setFixedSize(ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT + YIJI_HEIGHT);
        }
        else
        {
            this->setFixedSize(ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT);

        }
        QRect rect;
        int totalHeight = qApp->screenAt(QCursor::pos())->size().height() + qApp->screenAt(QCursor::pos())->geometry().y();//屏幕高度
        int totalWidth = qApp->screenAt(QCursor::pos())->size().width() + qApp->screenAt(QCursor::pos())->geometry().x();
        switch(position)
        {
        case 0:
            if(yijistate)
            {
                rect.setRect(totalWidth - ORIGIN_CALENDAR_WIDTH - WIDGET_GAP, totalHeight - panelSize - LESSTWOSHCEDULE_CALENDAR_HEIGHT - YIJI_HEIGHT - WIDGET_GAP,
                             ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT + YIJI_HEIGHT);
            }
            else
            {
                rect.setRect(totalWidth - ORIGIN_CALENDAR_WIDTH - WIDGET_GAP, totalHeight - panelSize - LESSTWOSHCEDULE_CALENDAR_HEIGHT - WIDGET_GAP,
                             ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT);
                if(isRTLLanguage())
                {
                    qDebug() << "QLocale::system().language();" << QLocale::system().language();
                    rect.setRect(qApp->screenAt(QCursor::pos())->geometry().x() + WIDGET_GAP, totalHeight - LESSTWOSHCEDULE_CALENDAR_HEIGHT - WIDGET_GAP,
                                 ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT);
                }

            }
            kdk::WindowManager::setGeometry(this->windowHandle(), rect);
            break;
        case 1:
            if(yijistate)
            {
                rect.setRect(totalWidth - ORIGIN_CALENDAR_WIDTH - WIDGET_GAP, qApp->screenAt(QCursor::pos())->geometry().y() + panelSize + WIDGET_GAP,
                             ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT + YIJI_HEIGHT);
            }
            else
            {
                rect.setRect(totalWidth - ORIGIN_CALENDAR_WIDTH - WIDGET_GAP, qApp->screenAt(QCursor::pos())->geometry().y() + panelSize + WIDGET_GAP,
                             ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT);
            }
            kdk::WindowManager::setGeometry(this->windowHandle(), rect);
            break;
        case 2:
            if(yijistate)
            {
                rect.setRect(qApp->screenAt(QCursor::pos())->geometry().x() + panelSize + WIDGET_GAP, totalHeight - LESSTWOSHCEDULE_CALENDAR_HEIGHT - WIDGET_GAP - YIJI_HEIGHT,
                             ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT + YIJI_HEIGHT);
            }
            else
            {
                rect.setRect(qApp->screenAt(QCursor::pos())->geometry().x() + panelSize + WIDGET_GAP, totalHeight - LESSTWOSHCEDULE_CALENDAR_HEIGHT - WIDGET_GAP,
                             ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT);
            }
            kdk::WindowManager::setGeometry(this->windowHandle(), rect);
            break;
        case 3:
            if(yijistate)
            {
                rect.setRect(totalWidth - panelSize - ORIGIN_CALENDAR_WIDTH - WIDGET_GAP, totalHeight - LESSTWOSHCEDULE_CALENDAR_HEIGHT - WIDGET_GAP - YIJI_HEIGHT,
                             ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT + YIJI_HEIGHT);
            }
            else
            {
                rect.setRect(totalWidth - panelSize - ORIGIN_CALENDAR_WIDTH - WIDGET_GAP, totalHeight - LESSTWOSHCEDULE_CALENDAR_HEIGHT - WIDGET_GAP,
                             ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT);
            }
            kdk::WindowManager::setGeometry(this->windowHandle(), rect);
            break;
        default:
            break;
        }
    }

    if(panelHeight < this->height() || panelHeight < LESSTWOSHCEDULE_CALENDAR_HEIGHT || isExceedScreenHeight)
    {
        if(ScrollAreaNeedChange)
        {
            qDebug() << "不支持分辨率";
        }
        else
        {
            qDebug() << "分辨率支持111";
            int height = qApp->screenAt(QCursor::pos())->size().height() + qApp->screenAt(QCursor::pos())->geometry().y();
            scrollArea->verticalScrollBar()->setStyleSheet(ScrollAreaStyleSheet);
            QPoint currentPos = this->pos();
            scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
            this->setFixedSize(ORIGIN_CALENDAR_WIDTH, panelHeight - SCROLLA);
            m_scheduleWidget->setFixedHeight(100 * num);
            //scrollArea->setFixedSize(420, height*0.1);
            scrollArea->setFixedWidth(ORIGIN_CALENDAR_WIDTH);
            this->move(currentPos.x(), SCROLLA - WIDGET_GAP);
            isExceedScreenHeight = false;
        }
    }
    foreach(info, m_markInfoList)
    {
        scheduleWidget = new schedule_item(this, true);
        QString time1;
        QString time2;
        if(timemodel == "12")
        {
            scheduleWidget->timemode = "12";
            if(info.hour <= 12)
            {
                time1 = tr("morning");
            }
            else
            {
                time1 = tr("afternoon");
                info.hour = info.hour - 12;
                //            info.end_hour = info.end_hour-12;
            }
            if(info.end_hour < 12)
            {
                time2 = tr("morning");
            }
            else
            {
                time2 = tr("afternoon");
                //            info.hour = info.hour-12;
                info.end_hour = info.end_hour - 12;
            }

        }
        else if(timemodel == "24")
        {
            time1 = "";
            time2 = "";
        }

        QString starttimeString = QString::number(info.hour) + ":" + QString::number(info.minute); // 确保这是有效的格式
        QTime m_startTime = QTime::fromString(starttimeString, "h:m");
        QString start = m_startTime.toString("hh:mm");
        QString timeString = QString::number(info.end_hour) + ":" + QString::number(info.end_minute); // 确保这是有效的格式
        QTime m_endTime = QTime::fromString(timeString, "h:m");
        QString end = m_endTime.toString("hh:mm");

        QString  lineOne = QString(" %1%2-%3%4 ").arg(time1).arg(start).arg(time2).arg(end)
                           + info.timeLong;
        if(info.m_isAllDay)
        {
            lineOne = tr("   allday");
        }

        scheduleWidget->setStartTime(lineOne);
        scheduleWidget->adjustDate(this->date);
        scheduleWidget->setEndTime(info.end_hour, info.end_minute, this->date, time2);
        scheduleWidget->setDescription(info.m_descript);

        scheduleWidget->setId(info.m_markId);
        connect(scheduleWidget, &schedule_item::ScheduleClick, this, &LunarCalendarWidget::slotEditSchedule);
        connect(scheduleWidget, &schedule_item::EditSchedule, this, &LunarCalendarWidget::slotEditSchedule);
        //        connect(scheduleWidget,&schedule_item::updateSchedule1,this,&LunarCalendarWidget::updateFromItem);
        connect(scheduleWidget, &schedule_item::updateWindow, this, &LunarCalendarWidget::updateFromItem);
        item_layout->addWidget(scheduleWidget);
        item_layout->addSpacing(2);

    }
    labWidget->setContentsMargins(16, 0, 16, 0);
    yijiWidget->setContentsMargins(16, 0, 16, 0);
    m_scheduleWidget->setLayout(item_layout);
    scrollArea->setWidget(m_scheduleWidget);

    m_horLayoutWidgetTop = new QHBoxLayout(this);
    m_horLayoutWidgetTop->setContentsMargins(26, 0, 24, 0);
    m_horLayoutWidgetTop->addWidget(widgetTime);
    m_horLayoutWidgetTop->addWidget(m_createScheduleButton);

    m_verLayoutCalendar->setMargin(0);
    m_verLayoutCalendar->setSpacing(0);
    m_verLayoutCalendar->addLayout(m_horLayoutWidgetTop);
    m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
    m_verLayoutCalendar->addWidget(lineUp);
    m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
    m_verLayoutCalendar->addWidget(m_widgetTop);
    m_verLayoutCalendar->addSpacing(18);
    m_verLayoutCalendar->addWidget(widgetWeek);
    m_verLayoutCalendar->addWidget(m_widgetDayBody, 1);
    m_verLayoutCalendar->addWidget(widgetYearBody, 1);
    m_verLayoutCalendar->addWidget(widgetmonthBody, 1);
    m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
    m_verLayoutCalendar->addWidget(lineDown);
    controlDownLine();
    m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
    m_verLayoutCalendar->addWidget(labWidget);
    //    if(yijistate){
    m_verLayoutCalendar->addWidget(yijiWidget);
    //    }

    //yiji框的下划线分割
    m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
    lineSchedule->setFixedHeight(1);
    m_verLayoutCalendar->addWidget(lineSchedule);
    m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
    m_verLayoutCalendar->addWidget(scrollArea);
}
void LunarCalendarWidget::updateSchedule()
{
    item_layout = new QVBoxLayout(this);
    int num = m_markInfoList.count();
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_num = num;
    //正常分辨率
    if(!ScrollbarUnderTheResolution)
    {
        if(num == 0)
        {
            noSchedule();
        }
        else
        {
            if(num <= 2)
            {
                twoLessSchedule();
            }
            else if(CalendarDataBase::getInstance().m_markInfoList.count() > 2)
            {
                if(panelHeight < LESSTWOSHCEDULE_CALENDAR_HEIGHT)
                {
                    isExceedScreenHeight = true;
                    twoLessSchedule();
                    isExceedScreenHeight = false;
                }
                else
                {
                    ThreeMoreSchedule();

                }
            }

        }
    }
    //极限分辨率
    else
    {
        qDebug() << "-------DisplayAtResolution";
        DisplayAtResolution();
    }
}

void LunarCalendarWidget::getFontInfo(QString &fontName, int &fontSize)
{
    qDebug() << __FILE__ << __FUNCTION__;
    fontName = m_fontName;
    fontSize = m_fontSize;
}
LunarCalendarWidget::~LunarCalendarWidget()
{
}

/*
 * @brief 设置日历的背景及文字颜色
 * 参数：
 * weekColor 周六及周日文字颜色
*/

void LunarCalendarWidget::setColor(bool mdark_style)
{
    if(mdark_style)
    {
        weekTextColor = QColor(0, 0, 0);
        weekBgColor = QColor(180, 180, 180);
        if(calendar_gsettings != nullptr)
        {
            m_showLunar = calendar_gsettings->get(LUNAR_KEY).toString() == "lunar";
        }
        bgImage = ":/image/bg_calendar.png";
        selectType = SelectType_Rect;
    }
    else
    {
        weekTextColor = QColor(255, 255, 255);
        weekBgColor = QColor(0, 0, 0);
        if(calendar_gsettings != nullptr)
        {
            m_showLunar = calendar_gsettings->get(LUNAR_KEY).toString() == "lunar";
        }
        bgImage = ":/image/bg_calendar.png";
        selectType = SelectType_Rect;
    }
    initStyle();
}

void LunarCalendarWidget::controlDownLine()
{
    QString lang;
    QString locale;
    getLocale(lang, locale);
    if((!lang.contains("zh", Qt::CaseInsensitive)) ||
        (!locale.contains("zh", Qt::CaseInsensitive)))
    {
        if(lineDown != nullptr)
        {
            lineDown->hide();
        }
    }
    if(calendar_gsettings->get("calendar").toString() == "solarlunar")
    {
        if(lineDown != nullptr)
        {
            lineDown->hide();
        }
    }
}

void LunarCalendarWidget::_timeUpdate()
{
    QDateTime time = QDateTime::currentDateTime();
    QLocale locale = (QLocale::system().name() == "zh_CN" ? (QLocale::Chinese) : (QLocale::English));

    QString _time = SDK2CPP(kdk_system_second)();
    QFont font(m_fontName);
    datelabel->setText(_time);
    font.setPixelSize(24);
    //    datelabel->setFont(font);
    QString strHoliday;
    QString strSolarTerms;
    QString strLunarFestival;
    QString strLunarYear;
    QString strLunarMonth;
    QString strLunarDay;

    LunarCalendarInfo::Instance()->getLunarCalendarInfo(locale.toString(time, "yyyy").toInt(),
            locale.toString(time, "MM").toInt(),
            locale.toString(time, "dd").toInt(),
            strHoliday,
            strSolarTerms,
            strLunarFestival,
            strLunarYear,
            strLunarMonth,
            strLunarDay);

    if (QGSettings::isSchemaInstalled(QByteArray(CALENDAR_GSETTINGS_SCHEMAS)))
    {
        QGSettings(QByteArray(CALENDAR_GSETTINGS_SCHEMAS)).set(CALENDAR_LUNAR_DATE_KEY, QVariant(QString(strLunarMonth + strLunarDay)));
    }

    QString _date = SDK2CPP(kdk_system_get_longformat_date)() + " " + SDK2CPP(kdk_system_longweek)();
    if (lunarstate)
    {
        _date = _date + " " + strLunarMonth + strLunarDay;
    }


    if (style_settings != nullptr)
    {
        // 获取系统字体大小
        float systemFontSize = style_settings->get(SYSTEM_FONT_SIZE).toFloat();

        float systemFontSizeFloat = static_cast<float>(systemFontSize);
        int datelabelFontSize, timelabelFontSize;

        if (systemFontSizeFloat <= 10.0)
        {
            datelabelFontSize = 24;
            timelabelFontSize = 14;
        }
        else if (systemFontSizeFloat <= 12.0)
        {
            datelabelFontSize = 26;
            timelabelFontSize = 16;
        }
        else if (systemFontSizeFloat <= 13.5)
        {
            datelabelFontSize = 28;
            timelabelFontSize = 18;
        }
        else if (systemFontSizeFloat <= 15.0)
        {
            datelabelFontSize = 30;
            timelabelFontSize = 20;
        }
        else
        {
            datelabelFontSize = 24;
            timelabelFontSize = 14;
        }
        font.setPixelSize(datelabelFontSize);
        datelabel->setFont(font);
        font.setPixelSize(timelabelFontSize);
        timelabel->setFont(font);
        // 监听系统字体变化的信号，并在变化时调整字体大小
        connect(style_settings, &QGSettings::changed, this, [this](const QString &key)
        {
            if (key == SYSTEM_FONT_SIZE)
            {
                // 获取系统字体大小
                float systemFontSize = style_settings->get(SYSTEM_FONT_SIZE).toFloat();
                float systemFontSizeFloat = static_cast<float>(systemFontSize);
                int datelabelFontSize, timelabelFontSize;
                if (systemFontSizeFloat <= 10.0)
                {
                    datelabelFontSize = 24;
                    timelabelFontSize = 14;
                }
                else if (systemFontSizeFloat <= 12.0)
                {
                    datelabelFontSize = 26;
                    timelabelFontSize = 16;
                }
                else if (systemFontSizeFloat <= 13.5)
                {
                    datelabelFontSize = 28;
                    timelabelFontSize = 18;
                }
                else if (systemFontSizeFloat <= 15.0)
                {
                    datelabelFontSize = 30;
                    timelabelFontSize = 20;
                }
                else
                {
                    // 默认或更大字体大小
                    datelabelFontSize = 24;
                    timelabelFontSize = 14;
                }
                QFont newDateLabelFont = datelabel->font();
                QFont newTimeLabelFont = timelabel->font();
                newDateLabelFont.setPixelSize(datelabelFontSize);
                datelabel->setFont(newDateLabelFont);
                newTimeLabelFont.setPixelSize(timelabelFontSize);
                timelabel->setFont(newTimeLabelFont);
            }
        });
    }
    QFont timelabel_font;
    QFontMetrics metrics(timelabel_font);
    QString elided = metrics.elidedText(_date, Qt::ElideRight, 250);//250后用...省略
    timelabel->setText(elided);
    timelabel->setToolTip(_date);
    //    timelabel->setFont(font);
    QFont font1(m_font);
    font1.setPixelSize(22);
    if(isWayland)
    {
        font.setPointSize(15);
    }

    QFont font2(m_font);
    font2.setPixelSize(12);
    QFontMetrics fm(font1);
    QFontMetrics fm2(font2);
    int h = fm.boundingRect(datelabel->text()).height();
    int h2 = fm2.boundingRect(timelabel->text()).height();
    datelabel->setAlignment(Qt::AlignLeft);
    timelabel->setAlignment(Qt::AlignLeft);
    datelabel->setFixedHeight(h + 5);
    timelabel->setFixedHeight(h2 + 10);
    datelabel->setContentsMargins(0, 0, 0, 0);
    timelabel->setContentsMargins(0, 0, 0, 0);

}

void LunarCalendarWidget::timerUpdate()
{
    _timeUpdate();
}

void LunarCalendarWidget::initWidget()
{
    setObjectName("lunarCalendarWidget");

    //顶部widget
    m_widgetTop = new QWidget;
    m_widgetTop->setObjectName("widgetTop");
    m_widgetTop->setMinimumHeight(35);

    //上个月的按钮
    btnPrevYear = new kdk::KBorderlessButton;
    btnPrevYear->setObjectName("btnPrevYear");
    btnPrevYear->setFixedWidth(36);
    btnPrevYear->setFixedHeight(36);
    btnPrevYear->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    //btnPrevYear->setProperty("useIconHighlightEffect", 0x2);
    btnPrevYear->setIcon(QIcon::fromTheme("lingmo-start-symbolic"));


    //下个月按钮
    btnNextYear = new kdk::KBorderlessButton;
    btnNextYear->setObjectName("btnNextYear");
    btnNextYear->setFixedWidth(36);
    btnNextYear->setFixedHeight(36);
    //btnNextYear->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    btnNextYear->setProperty("useIconHighlightEffect", 0x2);
    btnNextYear->setIcon(QIcon::fromTheme("lingmo-end-symbolic"));

    m_font.setFamily(m_fontName);
    m_font.setPixelSize(14);
    QFont smallfont = m_font;
    if(QLocale::system().name() != "zh_CN")
    {
        smallfont.setPointSize(8);
    }
    //转到年显示
    btnYear->setObjectName("btnYear");
    btnYear->setFocusPolicy(Qt::NoFocus);
    btnYear->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    //    btnYear->setFont(smallfont);
    btnYear->setText(tr("Year"));
    btnYear->setToolTip(tr("Year"));
    btnYear->setFixedSize(56, 36);

    //btnYear->setStyle(new CustomStyle_pushbutton("lingmo-default"));
    connect(btnYear, &QPushButton::clicked, this, &LunarCalendarWidget::yearWidgetChange);

    //转到月显示
    btnMonth->setObjectName("btnMonth");
    btnMonth->setFocusPolicy(Qt::NoFocus);
    //    btnMonth->setFont(smallfont);
    btnMonth->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    btnMonth->setText(tr("Month"));
    btnMonth->setToolTip(tr("Month"));
    btnMonth->setFixedSize(56, 36);
    //btnMonth->setStyle(new CustomStyle_pushbutton("lingmo-default"));
    connect(btnMonth, &QPushButton::clicked, this, &LunarCalendarWidget::monthWidgetChange);

    //转到今天
    btnToday->setObjectName("btnToday");
    btnToday->setFocusPolicy(Qt::NoFocus);
    //    btnToday->setFont(smallfont);
    //btnToday->setFixedWidth(40);
    //btnToday->setStyle(new CustomStyle_pushbutton("lingmo-default"));
    btnToday->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    btnToday->setText(tr("Today"));
    btnToday->setToolTip(tr("Today"));
    btnToday->setFixedSize(56, 36);

    btnYear->setProperty("needTranslucent", true);
    btnMonth->setProperty("needTranslucent", true);
    btnToday->setProperty("needTranslucent", true);
    btnYear->setProperty("useButtonPalette", true);
    btnMonth->setProperty("useButtonPalette", true);
    btnToday->setProperty("useButtonPalette", true);
    //年份与月份下拉框 暂不用此
    cboxYearandMonth = new QComboBox;
    cboxYearandMonthLabel = new QLabel();
    cboxYearandMonthLabel->setFixedWidth(90);
    cboxYearandMonthLabel->setFixedHeight(36);
    if(isWayland)
    {
        cboxYearandMonthLabel->setFixedWidth(80);
    }

    //    cboxYearandMonthLabel->setFont(m_font);

    //顶部横向布局
    QHBoxLayout *layoutTop = new QHBoxLayout(m_widgetTop);
    layoutTop->setContentsMargins(8, 0, 24, 0);
    layoutTop->addSpacing(8);
    if(isRTLLanguage())
    {
        layoutTop->addWidget(btnNextYear);
        layoutTop->addWidget(cboxYearandMonthLabel);
        layoutTop->addWidget(btnPrevYear);
    }
    else
    {
        layoutTop->addWidget(btnPrevYear);
        layoutTop->addWidget(cboxYearandMonthLabel);
        layoutTop->addWidget(btnNextYear);
    }
    layoutTop->addStretch();
    layoutTop->addWidget(btnYear);
    layoutTop->addWidget(btnMonth);
    layoutTop->addWidget(btnToday);
    layoutTop->addSpacing(8);



    //时间
    widgetTime->setFixedHeight(89);
    //widgetTime->setStyleSheet("background-color:grey");
    m_verLayoutTopLeft->setContentsMargins(0, 0, 0, 0);
    m_verLayoutTopLeft->setSpacing(0);
    m_verLayoutTopLeft->addStretch();
    m_verLayoutTopLeft->addWidget(datelabel);
    m_verLayoutTopLeft->addWidget(timelabel);
    m_verLayoutTopLeft->addStretch();
    //datelabel->setStyleSheet("background-color:red");
    //timelabel->setStyleSheet("background-color:green");
    //星期widget
    widgetWeek = new QWidget();
    widgetWeek->setObjectName("widgetWeek");
    widgetWeek->setMinimumHeight(30);
    widgetWeek->installEventFilter(this);

    //星期布局
    QHBoxLayout *layoutWeek = new QHBoxLayout(widgetWeek);
    layoutWeek->setMargin(0);
    layoutWeek->setSpacing(0);

    for (int i = 0; i < 7; i++)
    {
        QLabel *lab = new QLabel;
        lab->setObjectName(QString("WeekLabel_%1").arg(i));
        QFont font(m_font);
        font.setPixelSize(12);
        lab->setFont(font);
        lab->setAlignment(Qt::AlignCenter);
        layoutWeek->addWidget(lab);
        labWeeks.append(lab);
    }

    //日期标签widget
    m_widgetDayBody = new QWidget;
    m_widgetDayBody->setObjectName("widgetDayBody");
    m_widgetDayBody->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_widgetDayBody->setMinimumHeight(350);
    //日期标签布局
    QGridLayout *layoutBodyDay = new QGridLayout(m_widgetDayBody);
    layoutBodyDay->setMargin(1);

    layoutBodyDay->setHorizontalSpacing(0);
    layoutBodyDay->setVerticalSpacing(0);
    //    QDate currentDate = date;
    //逐个添加日标签
    for (int i = 0; i < 42; i++)
    {
        //        QDate currentDate = date.addDays(i - date.dayOfWeek() +1);
        LunarCalendarItem *lab = new LunarCalendarItem(m_fontName, false);
        lab->setObjectName(QString("DayLabel_%1").arg(i));
        lab->setFixedSize(60, 56);
        lab->installEventFilter(this);
        lab->worktime = worktime;
        connect(lab, SIGNAL(clicked(QDate, LunarCalendarItem::DayType)), this, SLOT(labClicked(QDate, LunarCalendarItem::DayType)));
        layoutBodyDay->addWidget(lab, i / 7, i % 7);
        dayItems.append(lab);

    }

    //年份标签widget
    widgetYearBody = new QWidget;
    widgetYearBody->setObjectName("widgetYearBody");
    widgetYearBody->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    widgetYearBody->setMinimumHeight(350);
    //年份标签布局
    QGridLayout *layoutBodyYear = new QGridLayout(widgetYearBody);
    layoutBodyYear->setMargin(1);
    layoutBodyYear->setHorizontalSpacing(0);
    layoutBodyYear->setVerticalSpacing(0);

    //逐个添加年标签
    for (int i = 0; i < 12; i++)
    {
        LunarCalendarYearItem *labYear = new LunarCalendarYearItem(m_fontName);
        labYear->setObjectName(QString("YEARLabel_%1").arg(i));
        labYear->installEventFilter(this);
        connect(labYear, SIGNAL(yearMessage(QDate, LunarCalendarYearItem::DayType)), this, SLOT(updateYearClicked(QDate, LunarCalendarYearItem::DayType)));
        layoutBodyYear->addWidget(labYear, i / 3, i % 3);
        yearItems.append(labYear);
    }
    widgetYearBody->hide();

    //月份标签widget
    widgetmonthBody = new QWidget;
    widgetmonthBody->setObjectName("widgetmonthBody");
    widgetmonthBody->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    widgetmonthBody->setMinimumHeight(350);
    //月份标签布局
    QGridLayout *layoutBodyMonth = new QGridLayout(widgetmonthBody);
    layoutBodyMonth->setMargin(1);
    layoutBodyMonth->setHorizontalSpacing(0);
    layoutBodyMonth->setVerticalSpacing(0);

    //逐个添加月标签
    for (int i = 0; i < 12; i++)
    {
        LunarCalendarMonthItem *labMonth = new LunarCalendarMonthItem(m_fontName);
        labMonth->setObjectName(QString("MONTHLabel_%1").arg(i));
        labMonth->installEventFilter(this);
        connect(labMonth, SIGNAL(monthMessage(QDate, LunarCalendarMonthItem::DayType)), this, SLOT(updateMonthClicked(QDate, LunarCalendarMonthItem::DayType)));
        layoutBodyMonth->addWidget(labMonth, i / 3, i % 3);
        monthItems.append(labMonth);
    }
    widgetmonthBody->hide();

    QFont font(m_fontName);
    font.setPixelSize(14);
    //       font.setPointSize(5);
    //           if(isWayland){
    //             font.setPointSize(12);
    //    }

    labWidget = new QWidget();
    labWidget->installEventFilter(this);
    labBottom = new QLabel();
    labBottom->installEventFilter(this);
    yijichooseLabel = new QLabel();
    yijichooseLabel->setText("宜忌");
    //    yijichooseLabel->setFont(font);
    //    yijichooseLabel->setFixedHeight(13);
    yijichooseLabel->installEventFilter(this);

    //    labBottom->setFont(font);
    labBottom->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    //        labBottom->setFixedHeight(18);
    //            if(isWayland){
    //               labBottom->setFixedHeight(12);
    //           }
    //    labBottom->setFixedHeight(15);
    yijichoose = new QCheckBox();
    yijichoose->installEventFilter(this);
    yijichoose->setFixedHeight(18);

    labLayout = new QHBoxLayout();
    labLayout->addWidget(labBottom);
    labLayout->addStretch();
    labLayout->addWidget(yijichooseLabel);
    labLayout->addWidget(yijichoose);
    labWidget->setLayout(labLayout);

    yijiLayout = new QVBoxLayout;
    yijiWidget = new QWidget;

    yiLabel = new QLabel();
    jiLabel = new QLabel();


    yijiLayout->addWidget(yiLabel);
    yijiLayout->addWidget(jiLabel);
    yijiWidget->setLayout(yijiLayout);
    yiLabel->setVisible(false);
    jiLabel->setVisible(false);

    //添加日程按钮
    m_createScheduleButton = new kdk::KPushButton(this); //创建日程按钮
    m_createScheduleButton->setText(tr("Add Schedule"));
    //    m_createScheduleButton->setStyleSheet("background-color: transparent;");
    m_createScheduleButton->setFixedSize(96, 36);


    m_createScheduleButton->setProperty("needTranslucent", true);
    m_createScheduleButton->setProperty("useButtonPalette", true);

    //主布局
    lineUp = new PartLineWidget();
    lineDown = new PartLineWidget();
    lineSchedule = new PartLineWidget();

    lineUp->setFixedSize(ORIGIN_CALENDAR_WIDTH, 1);
    lineDown->setFixedSize(ORIGIN_CALENDAR_WIDTH, 1);
    lineSchedule->setFixedSize(ORIGIN_CALENDAR_WIDTH, 1);

    updateLayout();

    connect(yijichoose, &QRadioButton::clicked, this, &LunarCalendarWidget::customButtonsClicked);
    const QByteArray style_id(CALENDAR_GSETTINGS_SCHEMAS);
    if (QGSettings::isSchemaInstalled(style_id))
    {
        int num = CalendarDataBase::getInstance().m_markInfoList.count();
        yijiGsettings = new QGSettings(style_id);
        bool currentTheme = yijiGsettings->get(YIJISTATUS).toBool();
        yijichoose->setChecked(currentTheme);
        if(currentTheme)
        {
            yiLabel->setVisible(true);
            jiLabel->setVisible(true);
            yijistate = true;
            Q_EMIT yijiChangeUp(num);
        }
        else
        {
            yiLabel->setVisible(false);
            jiLabel->setVisible(false);
            yijistate = false;
            Q_EMIT yijiChangeDown(num);
        }
        connect(yijiGsettings, &QGSettings::changed, this, [ = ](const QString &keys)
        {
            if (keys == YIJISTATUS)
            {
                bool currentTheme = yijiGsettings->get(YIJISTATUS).toBool();
                yijichoose->setChecked(currentTheme);
                if(currentTheme)
                {
                    yiLabel->setVisible(true);
                    jiLabel->setVisible(true);
                    yijistate = true;
                    Q_EMIT yijiChangeUp(num);
                }
                else
                {
                    yiLabel->setVisible(false);
                    jiLabel->setVisible(false);
                    yijistate = false;
                    Q_EMIT yijiChangeDown(num);
                }

            }
        });
    }

    widgetWeek->setContentsMargins(16, 0, 16, 0);
    m_widgetDayBody->setContentsMargins(16, 0, 16, 0);
    widgetYearBody->setContentsMargins(16, 0, 16, 0);
    widgetmonthBody->setContentsMargins(16, 0, 16, 0);
    labWidget->setContentsMargins(16, 0, 16, 0);
    yijiWidget->setContentsMargins(16, 0, 16, 0);
    //绑定按钮和下拉框信号
    connect(btnPrevYear, &kdk::KBorderlessButton::clicked, this, [ = ]()
    {
        switch (m_currentType)
        {
        case 0:
            showPreviousMonth(true);
            break;
        case 2:
            changeYear(false);
            break;
        case 1:
        default:
            break;
        }
    });
    connect(btnNextYear, &kdk::KBorderlessButton::clicked, this, [ = ]()
    {
        switch (m_currentType)
        {
        case 0:
            showNextMonth(true);
            break;
        case 2:
            changeYear(true);
            break;
        case 1:
        default:
            break;
        }
    });
    connect(btnToday, SIGNAL(clicked(bool)), this, SLOT(showToday()));
    //connect(this,&LunarCalendarWidget::DayClick,this,&LunarCalendarWidget::CreateSchedule);
    connect(m_createScheduleButton, &QPushButton::clicked, this, &LunarCalendarWidget::slotAddSchedule);
    yijichoose->setProperty("needTranslucent", true);
}

void LunarCalendarWidget::slotShowMore()
{

}


//双击弹出新建日程弹窗
void LunarCalendarWidget::CreateSchedule(QDate date)
{
    dlg = new CSchceduleDlg(1, this, true);
    dlg->m_beginDateEdit->setDate(date);
    dlg->m_endDateEdit->setDate(date);
    dlg->show();
}

void LunarCalendarWidget::slotAddSchedule()
{
    QRect rect;
    dlg = new CSchceduleDlg(1, this, true);
    dlg->setWindowFlags(dlg->windowFlags());
    // 获取屏幕中心点和尺寸
    QPoint screenCenter = QApplication::desktop()->availableGeometry().center();
    QSize dialogSize = dlg->size();
    qDebug() << "获得的屏幕中心坐标是" << screenCenter.x() << screenCenter.y();
    // 计算对话框应显示的中心点坐标，并将其移动到屏幕中心

    //    dlg->move(screenCenter.x() - dialogSize.width() / 2, screenCenter.y() - dialogSize.height() / 2);
    rect.setRect(screenCenter.x() - dialogSize.width() / 2, screenCenter.y() - dialogSize.height() / 2, dialogSize.width(), dialogSize.height());
    qDebug() << "------------rect" << rect;
    KWindowSystem::setState(dlg->winId(), NET::SkipPager | NET::SkipTaskbar | NET::SkipSwitcher);
    connect(dlg, &CSchceduleDlg::NeedDelete, this, [ = ]()
    {
        dlg->deleteLater();
    });
    dlg->setFocus();
    dlg->show();
    kdk::WindowManager::setGeometry(dlg->windowHandle(), rect);

}
void LunarCalendarWidget::slotEditSchedulefromMenu(QString id)
{
    slotEditSchedule(id);
}
void LunarCalendarWidget::slotEditSchedule(QString id)
{
    //    CalendarDataBase::getInstance().m_markInfoList;
    QRect rect;
    dlg = new CSchceduleDlg(0, this, true);
    dlg->setId(id);
    dlg->setStatusFromId(id);

    // 获取屏幕中心点和尺寸
    QPoint screenCenter = QApplication::desktop()->availableGeometry().center();
    QSize dialogSize = dlg->size();
    // 计算对话框应显示的中心点坐标，并将其移动到屏幕中心
    //    dlg->move(screenCenter.x() - dialogSize.width() / 2, screenCenter.y() - dialogSize.height() / 2);
    rect.setRect(screenCenter.x() - dialogSize.width() / 2, screenCenter.y() - dialogSize.height() / 2, dialogSize.width(), dialogSize.height());
    KWindowSystem::setState(dlg->winId(), NET::SkipPager | NET::SkipTaskbar | NET::SkipSwitcher);

    connect(dlg, &QDialog::accepted, this, [ = ]()
    {
        updateAllSchedule();
        updateLayout();
        // 若有必要，也可以在这里发射信号 Q_EMIT Up(m_markInfoList.count());
    });
    dlg->setFocus();
    dlg->show();
    kdk::WindowManager::setGeometry(dlg->windowHandle(), rect);


}


//设置日历的地区
void LunarCalendarWidget::setLocaleCalendar()
{
#if 0
    QStringList res = getLocale();
    qDebug() << "设置区域:" << res.at(0);
    int isCN = res.at(0) == "zh_CN.UTF-8" ? 1 : 0;
#else
    QString language;
    QString locale;
    QStringList res = getLocale(language, locale);
    qDebug() << language << locale;
    qDebug() << "设置区域:" << res.at(0);
    //int isCN = res.at(0) == "zh_CN.UTF-8" ? 1 : 0;
    bool isCN = language.contains("zh_CN");
#endif
    if (FirstdayisSun)
    {
        labWeeks.at(0)->setText(tr("Sun"));
        labWeeks.at(1)->setText(tr("Mon"));
        labWeeks.at(2)->setText(tr("Tue"));
        labWeeks.at(3)->setText(tr("Wed"));
        labWeeks.at(4)->setText(tr("Thur"));
        labWeeks.at(5)->setText(tr("Fri"));
        labWeeks.at(6)->setText(tr("Sat"));
    }
    else
    {
        labWeeks.at(0)->setText(tr("Mon"));
        labWeeks.at(1)->setText(tr("Tue"));
        labWeeks.at(2)->setText(tr("Wed"));
        labWeeks.at(3)->setText(tr("Thur"));
        labWeeks.at(4)->setText(tr("Fri"));
        labWeeks.at(5)->setText(tr("Sat"));
        labWeeks.at(6)->setText(tr("Sun"));
    }
}

//获取指定地区的编号代码
QStringList LunarCalendarWidget::getLocale(QString &language, QString &locale)
{
    //判断区域(美国/中国)
    QString objpath;
    unsigned int uid = getuid();
    objpath = objpath + "/org/freedesktop/Accounts/User" + QString::number(uid);
    QString formats;
    QString lang;
    QStringList result;

    QDBusInterface localeInterface("org.freedesktop.Accounts",
                                   objpath,
                                   "org.freedesktop.DBus.Properties",
                                   QDBusConnection::systemBus(), this);
    QDBusReply<QMap<QString, QVariant>> reply = localeInterface.call("GetAll", "org.freedesktop.Accounts.User");
    if (reply.isValid())
    {
        QMap<QString, QVariant> propertyMap;
        propertyMap = reply.value();
        if (propertyMap.keys().contains("FormatsLocale"))
        {
            formats = propertyMap.find("FormatsLocale").value().toString();
            locale = formats;
        }
        if(lang.isEmpty() && propertyMap.keys().contains("Language"))
        {
            lang = propertyMap.find("Language").value().toString();
            language = lang;
        }
    }
    else
    {
        qDebug() << "reply failed";
    }
    result.append(formats);
    result.append(lang);
    return result;

}

void LunarCalendarWidget::initStyle()
{
}

void LunarCalendarWidget::analysisWorktimeJs()
{
    /*解析json文件*/
    QFile file("/usr/share/calendar/plugin-calendar/html/jiejiari.json");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString value = file.readAll();
    file.close();

    QJsonParseError parseJsonErr;
    QJsonDocument document = QJsonDocument::fromJson(value.toUtf8(), &parseJsonErr);
    if(!(parseJsonErr.error == QJsonParseError::NoError))
    {
        qDebug() << tr("解析json文件错误！");
        return;
    }
    QJsonObject jsonObject = document.object();
    QStringList args = jsonObject.keys();

    for (int i = 0; i < args.count(); i++)
    {
        if(jsonObject.contains(args.at(i)))
        {
            QJsonValue jsonValueList = jsonObject.value(args.at(i));
            QJsonObject item = jsonValueList.toObject();
            QStringList arg2 = item.keys();
            for (int j = 0; j < arg2.count(); j++)
            {
                worktimeinside.insert(arg2.at(j), item[arg2.at(j)].toString());
            }
        }
        worktime.insert(args.at(i), worktimeinside);
        worktimeinside.clear();
    }
}

void LunarCalendarWidget::yearWidgetChange()
{
    if (widgetYearBody->isHidden())
    {
        widgetYearBody->show();
        widgetWeek->hide();
        m_widgetDayBody->hide();
        widgetmonthBody->hide();
        changeMode(2);
        m_savedYear = date.year();
        qDebug () << m_savedYear;
    }
    else
    {
        widgetYearBody->hide();
        widgetWeek->show();
        m_widgetDayBody->show();
        widgetmonthBody->hide();
        date.setDate(m_savedYear, date.month(), date.day());
        changeMode(0);
        initDate();
    }

}

void LunarCalendarWidget::monthWidgetChange()
{
    if(widgetmonthBody->isHidden())
    {
        widgetYearBody->hide();
        widgetWeek->hide();
        m_widgetDayBody->hide();
        widgetmonthBody->show();
        changeMode(1);
    }
    else
    {
        widgetYearBody->hide();
        widgetWeek->show();
        m_widgetDayBody->show();
        widgetmonthBody->hide();
        changeMode(0);
    }
}
void LunarCalendarWidget::changeMode(int mode)
{
    m_currentType = mode;
    if(mode == 1 || mode == 2)
    {
        btnNextYear->setEnabled(false);
        btnPrevYear->setEnabled(false);
    }
    else
    {
        btnNextYear->setEnabled(true);
        btnPrevYear->setEnabled(true);
    }
}

//初始化日期面板
void LunarCalendarWidget::initDate()
{
    int year = date.year();
    int month = date.month();
    int day = date.day();
    if(oneRun)
    {
        downLabelHandle(date);
        yijihandle(date);
        oneRun = false;
    }
    //设置为今天,设置变量防止重复触发
    btnClick = true;
    //cboxYearandMonth->setCurrentIndex(cboxYearandMonth->findText(QString("%1.%2").arg(year).arg(month)));
    btnClick = false;

    cboxYearandMonthLabel->setAlignment(Qt::AlignCenter);
    if(m_currentType != 2)
    {
        updateYearMonthString(year, month);
    }
    else
    {
        updateYearMonthString(m_savedYear, month);
    }

    //首先判断当前月的第一天是星期几
    int week = LunarCalendarInfo::Instance()->getFirstDayOfWeek(year, month, FirstdayisSun);
    //当前月天数
    int countDay = LunarCalendarInfo::Instance()->getMonthDays(year, month);
    //上月天数
    int countDayPre = LunarCalendarInfo::Instance()->getMonthDays(1 == month ? year - 1 : year, 1 == month ? 12 : month - 1);
    pro = countDayPre;
    //如果上月天数上月刚好一周则另外处理
    int startPre, endPre, startNext, endNext, index, tempYear, tempMonth, tempDay;
    if (0 == week)
    {
        startPre = 0;
        endPre = 7;
        startNext = 0;
        endNext = 42 - (countDay + 7);
    }
    else
    {
        startPre = 0;
        endPre = week;
        startNext = week + countDay;
        endNext = 42;
    }

    //纠正1月份前面部分偏差,1月份前面部分是上一年12月份
    tempYear = year;
    tempMonth = month - 1;
    if (tempMonth < 1)
    {
        tempYear--;
        tempMonth = 12;
    }

    //显示上月天数
    for (int i = startPre; i < endPre; i++)
    {
        index = i;
        tempDay = countDayPre - endPre + i + 1;

        QDate date(tempYear, tempMonth, tempDay);
        QString lunar = LunarCalendarInfo::Instance()->getLunarDay(tempYear, tempMonth, tempDay);
        dayItems.at(index)->setDate(date, lunar, LunarCalendarItem::DayType_MonthPre);
        int x = CalendarDataBase::getInstance().searchDataSlot(dayItems.at(index)->date);
        if(x == DB_OP_SUCC)
        {
            dayItems.at(index)->hasSchedule = true;
        }
        else
        {
            dayItems.at(index)->hasSchedule = false;
        }
        if(date == QDate::currentDate())
        {
            dayItems.at(index)->isToday = true;
        }
        else
        {
            dayItems.at(index)->isToday = false;
        }


        dayItems.at(index)->setFont(m_fontName);
    }

    //纠正12月份后面部分偏差,12月份后面部分是下一年1月份
    tempYear = year;
    tempMonth = month + 1;
    if (tempMonth > 12)
    {
        tempYear++;
        tempMonth = 1;
    }

    //显示下月天数
    for (int i = startNext; i < endNext; i++)
    {
        index = 42 - endNext + i;
        tempDay = i - startNext + 1;

        QDate date(tempYear, tempMonth, tempDay);
        QString lunar = LunarCalendarInfo::Instance()->getLunarDay(tempYear, tempMonth, tempDay);
        dayItems.at(index)->setDate(date, lunar, LunarCalendarItem::DayType_MonthNext);
        int x = CalendarDataBase::getInstance().searchDataSlot(dayItems.at(index)->date);
        if(x == DB_OP_SUCC)
        {
            dayItems.at(index)->hasSchedule = true;
        }
        else
        {
            dayItems.at(index)->hasSchedule = false;
        }
        if(date == QDate::currentDate())
        {
            dayItems.at(index)->isToday = true;
        }
        else
        {
            dayItems.at(index)->isToday = false;
        }
    }

    //重新置为当前年月
    tempYear = year;
    tempMonth = month;
    QFont font(m_fontName);

    //显示当前月
    for (int i = week; i < (countDay + week); i++)
    {
        index = (0 == week ? (i + 7) : i);
        tempDay = i - week + 1;

        QDate date(tempYear, tempMonth, tempDay);
        QString lunar = LunarCalendarInfo::Instance()->getLunarDay(tempYear, tempMonth, tempDay);
        dayItems.at(index)->setDate(date, lunar, LunarCalendarItem::DayType_MonthCurrent);
        int x = CalendarDataBase::getInstance().searchDataSlot(dayItems.at(index)->date);
        if(x == DB_OP_SUCC)
        {
            dayItems.at(index)->hasSchedule = true;
        }
        else
        {
            dayItems.at(index)->hasSchedule = false;
        }
        if(date == QDate::currentDate())
        {
            dayItems.at(index)->isToday = true;
        }
        else
        {
            dayItems.at(index)->isToday = false;
        }

    }

    for (int i = 0; i < 12; i++)
    {
        monthItems.at(i)->setDate(QDate(date.year(), i + 1, 1));
        yearItems.at(i)->setDate(date.addYears(i));
        yearItems.at(i)->setFont(font);
        monthItems.at(i)->setFont(font);
    }
    auto it = dayItems.begin();
    for(; it != dayItems.end(); it++)
    {
        (*it)->setFont(font);
    }

    auto itw = labWeeks.begin();
    QFont f(m_fontName);
    f.setPixelSize(14);
    for(; itw != labWeeks.end(); itw++)
    {
        (*itw)->setFont(f);
    }

    //    labBottom->setFont(m_font);
    labBottom->repaint();

    //    yijichooseLabel->setFont(m_font);
    yijichooseLabel->repaint();

    //    yiLabel->setFont(m_font);
    //    jiLabel->setFont(m_font);
    yijiWidget->repaint();
    QFont smallfont = m_font;
    if(QLocale::system().name() != "zh_CN")
    {
        smallfont.setPointSize(8);
    }
    //    btnYear->setFont(smallfont);
    btnYear->repaint();
    //    btnToday->setFont(smallfont);
    btnToday->repaint();
    //    btnMonth->setFont(smallfont);
    btnMonth->repaint();
    m_widgetTop->repaint();
    //    cboxYearandMonthLabel->setFont(m_font);
    cboxYearandMonthLabel->repaint();
}

void LunarCalendarWidget::customButtonsClicked(int x)
{
    int num = CalendarDataBase::getInstance().m_markInfoList.count();
    int totalHeight = qApp->screenAt(QCursor::pos())->size().height() + qApp->screenAt(QCursor::pos())->geometry().y();//屏幕高度
    int totalWidth = qApp->screenAt(QCursor::pos())->size().width() + qApp->screenAt(QCursor::pos())->geometry().x();
    int  m_panelPoistion = m_panelGsettings->get(PANEL_POSITION_KEY).toInt();
    QRect rect;
    if (x == 1)
    {
        yiLabel->setVisible(true);
        jiLabel->setVisible(true);
        yijistate = true;
        if (yijiGsettings)
        {
            yijiGsettings->set(YIJISTATUS, QVariant(true));
        }

        QPoint currentPos = this->pos();

        //        this->setFixedSize(ORIGIN_CALENDAR_WIDTH,this->height()+YIJI_HEIGHT);
        //2种情况1.目前屏幕大小 740 <x< 740+75 2.屏幕高度 x <740
        if(LESSTWOSHCEDULE_CALENDAR_HEIGHT < panelHeight && panelHeight < LESSTWOSHCEDULE_CALENDAR_HEIGHT + YIJI_HEIGHT)
        {
            ScrollAreaNeedChange = true;
            updateLayout();
            return;
        }
        else if(panelHeight < LESSTWOSHCEDULE_CALENDAR_HEIGHT)
        {
            ScrollbarUnderTheResolution = true;
            updateLayout();
            return;
        }
        //当位置在上方的时候 不需要move，只需要改变高度即可
        if(position != 1)
        {
            if(isWayland)
            {
                //3右 0下 2左 1上
                switch (m_panelPoistion)
                {
                case 0:
                    rect.setRect(totalWidth - ORIGIN_CALENDAR_WIDTH - WIDGET_GAP, panelHeight - LESSTWOSHCEDULE_CALENDAR_HEIGHT - YIJI_HEIGHT - WIDGET_GAP,
                                 ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT + YIJI_HEIGHT);
                    kdk::WindowManager::setGeometry(this->windowHandle(), rect);
                    break;
                case 1:
                    rect.setRect(totalWidth - ORIGIN_CALENDAR_WIDTH - WIDGET_GAP, qApp->screenAt(QCursor::pos())->geometry().y() + panelSize + WIDGET_GAP,
                                 ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT + YIJI_HEIGHT);
                    kdk::WindowManager::setGeometry(this->windowHandle(), rect);
                    break;
                case 2:
                    rect.setRect(qApp->screenAt(QCursor::pos())->geometry().x() + panelSize + WIDGET_GAP, totalHeight - LESSTWOSHCEDULE_CALENDAR_HEIGHT - WIDGET_GAP - YIJI_HEIGHT,
                                 ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT + YIJI_HEIGHT);
                    kdk::WindowManager::setGeometry(this->windowHandle(), rect);
                    break;
                case 3:
                    rect.setRect(totalWidth - panelSize - ORIGIN_CALENDAR_WIDTH - WIDGET_GAP, totalHeight - LESSTWOSHCEDULE_CALENDAR_HEIGHT - WIDGET_GAP - YIJI_HEIGHT,
                                 ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT + YIJI_HEIGHT);
                    kdk::WindowManager::setGeometry(this->windowHandle(), rect);
                    break;
                }
            }
            else
            {
                this->move(currentPos.x(), currentPos.y() - YIJI_HEIGHT);
            }
        }
        this->setFixedSize(ORIGIN_CALENDAR_WIDTH, this->height() + YIJI_HEIGHT);
    }
    else
    {
        yiLabel->setVisible(false);
        jiLabel->setVisible(false);
        //        Q_EMIT yijiChangeDown(num);
        yijistate = false;
        if (yijiGsettings)
        {
            yijiGsettings->set(YIJISTATUS, QVariant(false));
        }

        if(LESSTWOSHCEDULE_CALENDAR_HEIGHT < panelHeight && panelHeight < LESSTWOSHCEDULE_CALENDAR_HEIGHT + YIJI_HEIGHT)
        {
            ScrollAreaNeedChange = false;
            updateLayout();
            return;
        }
        else if(panelHeight < LESSTWOSHCEDULE_CALENDAR_HEIGHT)
        {
            ScrollbarUnderTheResolution = true;
            updateLayout();
            return;
        }
        QPoint currentPos = this->pos();

        //        this->setFixedSize(ORIGIN_CALENDAR_WIDTH,this->height()-YIJI_HEIGHT);
        //当位置在上方的时候 不需要move，只需要改变高度即可
        if(position != 1)
        {
            if(isWayland)
            {
                //3右 0下 2左 1上
                switch (m_panelPoistion)
                {
                case 0:
                    rect.setRect(totalWidth - ORIGIN_CALENDAR_WIDTH - WIDGET_GAP, totalHeight - panelSize - LESSTWOSHCEDULE_CALENDAR_HEIGHT - WIDGET_GAP,
                                 ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT);
                    kdk::WindowManager::setGeometry(this->windowHandle(), rect);
                    break;
                case 1:
                    rect.setRect(totalWidth - ORIGIN_CALENDAR_WIDTH - WIDGET_GAP, qApp->screenAt(QCursor::pos())->geometry().y() + panelSize + WIDGET_GAP,
                                 ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT);
                    kdk::WindowManager::setGeometry(this->windowHandle(), rect);
                    break;
                case 2:
                    rect.setRect(qApp->screenAt(QCursor::pos())->geometry().x() + panelSize + WIDGET_GAP, totalHeight - LESSTWOSHCEDULE_CALENDAR_HEIGHT - WIDGET_GAP,
                                 ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT);
                    kdk::WindowManager::setGeometry(this->windowHandle(), rect);
                    break;
                case 3:
                    rect.setRect(totalWidth - panelSize - ORIGIN_CALENDAR_WIDTH - WIDGET_GAP, totalHeight - LESSTWOSHCEDULE_CALENDAR_HEIGHT - WIDGET_GAP,
                                 ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT);
                    kdk::WindowManager::setGeometry(this->windowHandle(), rect);
                    break;
                }
            }
            else
            {
                this->move(currentPos.x(), currentPos.y() + YIJI_HEIGHT);
            }
        }
        this->setFixedSize(ORIGIN_CALENDAR_WIDTH, this->height() - YIJI_HEIGHT);
    }
}

QString LunarCalendarWidget::getSettings()
{
    QString arg = "配置文件";
    return  arg;

}

void LunarCalendarWidget::setSettings(QString arg)
{

}

void LunarCalendarWidget::downLabelHandle(const QDate &date)
{

    QString strHoliday;
    QString strSolarTerms;
    QString strLunarFestival;
    QString strLunarYear;
    QString strLunarMonth;
    QString strLunarDay;

    LunarCalendarInfo::Instance()->getLunarCalendarInfo(date.year(),
            date.month(),
            date.day(),
            strHoliday,
            strSolarTerms,
            strLunarFestival,
            strLunarYear,
            strLunarMonth,
            strLunarDay);

    QString labBottomarg = strLunarYear + "  " + strLunarMonth + strLunarDay;
    labBottom->setText(labBottomarg);

}

void LunarCalendarWidget::yijihandle(const QDate &date)
{
    QString yi = "宜：";
    QString ji = "忌：";
    /*解析json文件*/
    QFile file(QString("/usr/share/calendar/plugin-calendar/html/hlnew/hl%1.json").arg(date.year()));
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString value = file.readAll();
    file.close();

    QJsonParseError parseJsonErr;
    QJsonDocument document = QJsonDocument::fromJson(value.toUtf8(), &parseJsonErr);
    if(!(parseJsonErr.error == QJsonParseError::NoError))
    {
        qDebug() << tr("解析json文件错误！");
        yiLabel->setText(yi);
        jiLabel->setText(ji);
        return;
    }
    QJsonObject jsonObject = document.object();

    if(jsonObject.contains(QString("d%1").arg(date.toString("MMdd"))))
    {
        QJsonValue jsonValueList = jsonObject.value(QString("d%1").arg(date.toString("MMdd")));
        QJsonObject item = jsonValueList.toObject();
        QString yiString;
        QString jiString;
        if (item["y"].toString() == "." || item["y"].isNull())
        {
            yiString = yi;
        }
        else
        {
            yiString = yi + item["y"].toString();
        }
        if (item["j"].toString() == "." || item["j"].isNull())
        {
            jiString = ji;
        }
        else
        {
            jiString = ji + item["j"].toString();
        }

        yiLabel->setText(toHK(yiString));
        jiLabel->setText(toHK(jiString));
    }
}

QString LunarCalendarWidget::toHK(QString str)
{
    QString result;
    for (int i = 0; i < str.length(); i++)
    {
        QString tmp = str.at(i);
        if (m_zh_HK_dict.keys().contains(tmp))
        {
            tmp = m_zh_HK_dict.value(tmp);
        }
        result.append(tmp);
    }
    return result;
}

void LunarCalendarWidget::yearChanged(const QString &arg1)
{
    //如果是单击按钮切换的日期变动则不需要触发
    if (btnClick)
    {
        return;
    }
    int nIndex = arg1.indexOf(".");
    if(-1 == nIndex)
    {
        return;
    }
    int year = arg1.mid(0, nIndex).toInt();
    int month = arg1.mid(nIndex + 1).toInt();
    int day = date.day();
    dateChanged(year, month, day);
}

void LunarCalendarWidget::monthChanged(const QString &arg1)
{
    //如果是单击按钮切换的日期变动则不需要触发
    if (btnClick)
    {
        return;
    }

    int year = date.year();
    int month = arg1.mid(0, arg1.length()).toInt();
    int day = date.day();
    dateChanged(year, month, day);
}

void LunarCalendarWidget::labClicked(const QDate &date, const LunarCalendarItem::DayType &dayType)
{
    this->date = date;
    updateAllSchedule();
    updateLayout();
    s_clickDate = date;
    if (LunarCalendarItem::DayType_MonthPre == dayType)
    {
        showPreviousMonth(false);
    }
    else if (LunarCalendarItem::DayType_MonthNext == dayType)
    {
        showNextMonth(false);
    }
    else
    {
        dayChanged(this->date, s_clickDate);
    }
}

void LunarCalendarWidget::updateYearClicked(const QDate &date, const LunarCalendarYearItem::DayType &dayType)
{
    //通过传来的日期，设置当前年月份
    this->date = date;
    updateAllSchedule();
    updateLayout();
    widgetYearBody->hide();
    widgetWeek->show();
    m_widgetDayBody->show();
    widgetmonthBody->hide();
    s_clickDate = date;
    changeDate(date);
    dayChanged(date, QDate());
    changeMode(0);
    initDate();
}

void LunarCalendarWidget::updateMonthClicked(const QDate &date, const LunarCalendarMonthItem::DayType &dayType)
{
    //通过传来的日期，设置当前年月份
    this->date = date;
    updateAllSchedule();
    updateLayout();
    widgetYearBody->hide();
    widgetWeek->show();
    m_widgetDayBody->show();
    widgetmonthBody->hide();
    s_clickDate = date;
    changeDate(date);
    dayChanged(date, QDate());
    changeMode(0);
    initDate();
}

void LunarCalendarWidget::changeDate(const QDate &date)
{
    int year = date.year();
    int month = date.month();
    int day = date.day();
    if(oneRun)
    {
        downLabelHandle(date);
        yijihandle(date);
        oneRun = false;
    }

    updateYearMonthString(year, month);
    //首先判断当前月的第一天是星期几
    int week = LunarCalendarInfo::Instance()->getFirstDayOfWeek(year, month, FirstdayisSun);
    //当前月天数
    int countDay = LunarCalendarInfo::Instance()->getMonthDays(year, month);
    //上月天数
    int countDayPre = LunarCalendarInfo::Instance()->getMonthDays(1 == month ? year - 1 : year, 1 == month ? 12 : month - 1);

    //如果上月天数上月刚好一周则另外处理
    int startPre, endPre, startNext, endNext, index, tempYear, tempMonth, tempDay;
    if (0 == week)
    {
        startPre = 0;
        endPre = 7;
        startNext = 0;
        endNext = 42 - (countDay + 7);
    }
    else
    {
        startPre = 0;
        endPre = week;
        startNext = week + countDay;
        endNext = 42;
    }

    //纠正1月份前面部分偏差,1月份前面部分是上一年12月份
    tempYear = year;
    tempMonth = month - 1;
    if (tempMonth < 1)
    {
        tempYear--;
        tempMonth = 12;
    }

    //显示上月天数
    for (int i = startPre; i < endPre; i++)
    {
        index = i;
        tempDay = countDayPre - endPre + i + 1;

        QDate date(tempYear, tempMonth, tempDay);
        QString lunar = LunarCalendarInfo::Instance()->getLunarDay(tempYear, tempMonth, tempDay);
        dayItems.at(index)->setDate(date, lunar, LunarCalendarItem::DayType_MonthPre);
    }

    //纠正12月份后面部分偏差,12月份后面部分是下一年1月份
    tempYear = year;
    tempMonth = month + 1;
    if (tempMonth > 12)
    {
        tempYear++;
        tempMonth = 1;
    }

    //显示下月天数
    for (int i = startNext; i < endNext; i++)
    {
        index = 42 - endNext + i;
        tempDay = i - startNext + 1;

        QDate date(tempYear, tempMonth, tempDay);
        QString lunar = LunarCalendarInfo::Instance()->getLunarDay(tempYear, tempMonth, tempDay);
        dayItems.at(index)->setDate(date, lunar, LunarCalendarItem::DayType_MonthNext);
    }

    //重新置为当前年月
    tempYear = year;
    tempMonth = month;

    //显示当前月
    for (int i = week; i < (countDay + week); i++)
    {
        index = (0 == week ? (i + 7) : i);
        tempDay = i - week + 1;

        QDate date(tempYear, tempMonth, tempDay);
        QString lunar = LunarCalendarInfo::Instance()->getLunarDay(tempYear, tempMonth, tempDay);
        dayItems.at(index)->setDate(date, lunar, LunarCalendarItem::DayType_MonthCurrent);
    }

    for (int i = 0; i < 12; i++)
    {
        yearItems.at(i)->setDate(s_clickDate.addYears(i));
        monthItems.at(i)->setDate(QDate(s_clickDate.year(), i + 1, 1));
    }
}

void LunarCalendarWidget::dayChanged(const QDate &date, const QDate &m_date)
{
    //计算星期几,当前天对应标签索引=日期+星期几-1
    int year = date.year();
    int month = date.month();
    int day = date.day();
    int week = LunarCalendarInfo::Instance()->getFirstDayOfWeek(year, month, FirstdayisSun);
    //选中当前日期,其他日期恢复,这里还有优化空间,比方说类似单选框机制
    for (int i = 0; i < 42; i++)
    {
        //当月第一天是星期天要另外计算
        int index = day + week - 1;
        if (week == 0)
        {
            index = day + 6;
        }
        dayItems.at(i)->setSelect(false);
        if(dayItems.at(i)->getDate() == m_date)
        {
            dayItems.at(i)->setSelect(i == index);
        }
        if (i == index)
        {
            downLabelHandle(dayItems.at(i)->getDate());
            yijihandle(dayItems.at(i)->getDate());
        }

    }

    //发送日期单击信号
    Q_EMIT clicked(date);
    //发送日期更新信号
    Q_EMIT selectionChanged();
}

void LunarCalendarWidget::dateChanged(int year, int month, int day)
{
    //如果原有天大于28则设置为1,防止出错
    if(year <= 1970 || year >= 2099)
    {
        return ;
    }
    date.setDate(year, month, day > 28 ? 1 : day);
    initDate();
}

bool LunarCalendarWidget::getShowLunar()
{
    //语言是简体或繁体中文且区域是中国且GSetting的值时，
    //这个时候才显示农历和黄历信息
    QString lang, format;
    getLocale(lang, format);
    //qInfo() << lang << format << (calendar_gsettings->get("calendar").toString() == "lunar") ;
    if((lang.contains("zh_CN") || lang.contains("zh_HK")) &&
        (calendar_gsettings->get("calendar").toString() == "lunar"))
    {
        m_showLunar = true;
    }
    else
    {
        m_showLunar = false;
    }
    return m_showLunar;
}

void LunarCalendarWidget::setShowLunar(bool showLunar)
{
    bool realShow = showLunar & yijistate;
    //日界面需要重绘
    for(auto i = 0 ; i < dayItems.size(); i++)
    {
        dayItems.at(i)->setShowLunar(showLunar);
    }
    //日期字符串要重新写
    lunarstate = realShow;
    //黄历信息不能再显示了
    yiLabel->setVisible(realShow);
    jiLabel->setVisible(realShow);
    //yijistate = realShow;
    labWidget->setVisible(showLunar);
    yijiWidget->setVisible(showLunar);
    //    yijichoose->setChecked(yijistate);
    //重新布局当前窗口
    Q_EMIT almanacChanged(realShow);
}



LunarCalendarWidget::CalendarStyle LunarCalendarWidget::getCalendarStyle() const
{
    return this->calendarStyle;
}

QDate LunarCalendarWidget::getDate() const
{
    return this->date;
}

QColor LunarCalendarWidget::getWeekTextColor() const
{
    return this->weekTextColor;
}

QColor LunarCalendarWidget::getWeekBgColor() const
{
    return this->weekBgColor;
}

QString LunarCalendarWidget::getBgImage() const
{
    return this->bgImage;
}

LunarCalendarWidget::SelectType LunarCalendarWidget::getSelectType() const
{
    return this->selectType;
}


QColor LunarCalendarWidget::getBorderColor() const
{
    return this->borderColor;
}
QColor LunarCalendarWidget::getWeekColor() const
{
    return this->weekColor;
}

QColor LunarCalendarWidget::getSuperColor() const
{
    return this->superColor;
}

QColor LunarCalendarWidget::getLunarColor() const
{
    return this->lunarColor;
}
QColor LunarCalendarWidget::getCurrentTextColor() const
{
    return this->currentTextColor;
}

QColor LunarCalendarWidget::getOtherTextColor() const
{
    return this->otherTextColor;
}
QColor LunarCalendarWidget::getSelectTextColor() const
{
    return this->selectTextColor;
}

QColor LunarCalendarWidget::getHoverTextColor() const
{
    return this->hoverTextColor;
}

QColor LunarCalendarWidget::getCurrentLunarColor() const
{
    return this->currentLunarColor;
}

QColor LunarCalendarWidget::getOtherLunarColor() const
{
    return this->otherLunarColor;
}

QColor LunarCalendarWidget::getSelectLunarColor() const
{
    return this->selectLunarColor;
}

QColor LunarCalendarWidget::getHoverLunarColor() const
{
    return this->hoverLunarColor;
}

QColor LunarCalendarWidget::getCurrentBgColor() const
{
    return this->currentBgColor;
}

QColor LunarCalendarWidget::getOtherBgColor() const
{
    return this->otherBgColor;
}

QColor LunarCalendarWidget::getSelectBgColor() const
{
    return this->selectBgColor;
}

QColor LunarCalendarWidget::getHoverBgColor() const
{
    return this->hoverBgColor;
}

QSize LunarCalendarWidget::sizeHint() const
{
    return QSize(600, 500);
}

QSize LunarCalendarWidget::minimumSizeHint() const
{
    return QSize(200, 150);
}

//显示上一年
void LunarCalendarWidget::showPreviousYear()
{
    int year = date.year();
    int month = date.month();
    int day = date.day();
    if (year <= 1901)
    {
        return;
    }

    year--;
    dateChanged(year, month, day);
}

//显示下一年
void LunarCalendarWidget::showNextYear()
{
    int year = date.year();
    int month = date.month();
    int day = date.day();
    if (year >= 2099)
    {
        return;
    }

    year++;
    dateChanged(year, month, day);
}

//显示上月日期
void LunarCalendarWidget::showPreviousMonth(bool date_clicked)
{
    int year = date.year();
    int month = date.month();
    int day = date.day();
    if (year <= 1901 && month == 1)
    {
        return;
    }

    //extra:
    if (date_clicked) month--;
    if (month < 1)
    {
        month = 12;
        year--;
    }
    dateChanged(year, month, day);
}

//显示下月日期
void LunarCalendarWidget::showNextMonth(bool date_clicked)
{
    int year = date.year();
    int month = date.month();
    int day = date.day();
    if (year >= 2099 )
    {
        return;
    }

    //extra
    if (date_clicked)month++;
    if (month > 12)
    {
        month = 1;
        year++;
    }

    dateChanged(year, month, day);
}
void LunarCalendarWidget::setBool(bool more)
{
    isMore = more;
}
bool LunarCalendarWidget::getBool()
{
    return isMore;
}
void LunarCalendarWidget::updateAllSchedule()
{
    CalendarDataBase::getInstance().searchDataSlot(date);
    m_markInfoList = CalendarDataBase::getInstance().getInfoList();
    m_markIdList = CalendarDataBase::getInstance().getIdList();
    m_num   = m_markInfoList.count();
    Q_EMIT scheduleNum(m_num); ;
    // 遍历所有 MarkInfo，提取开始小时和分钟
    QList<int> startTimes;
    for (MarkInfo info : m_markInfoList)
    {
        int startTime = info.hour * 60 + info.minute; // 将小时转换为分钟并加上分钟数
        startTimes.append(startTime);
    }

    // 使用提取的开始时间对 m_markInfoList 进行排序
    QList<int> sortedIndices;
    for (int i = 0; i < startTimes.size(); ++i)
        sortedIndices.append(i);

    std::sort(sortedIndices.begin(), sortedIndices.end(), [&](int a, int b)
    {
        return startTimes[a] < startTimes[b];
    });

    QList<MarkInfo> sortedMarkInfoList;
    for (int index : sortedIndices)
        sortedMarkInfoList.append(m_markInfoList[index]);

    m_markInfoList = sortedMarkInfoList;
    twiceInfo = m_markInfoList.mid(0, 2);
    remainInfo = m_markInfoList.mid(2);

}
bool LunarCalendarWidget::compare(MarkInfo info1, MarkInfo info2)
{
    if(info1.hour != info2.hour)
    {
        return info1.hour  < info2.hour;
    }
    return info1.minute < info2.minute;
}
//转到今天
void LunarCalendarWidget::showToday()
{
    widgetYearBody->hide();
    widgetmonthBody->hide();
    m_widgetDayBody->show();
    widgetWeek->show();
    date = QDate::currentDate();
    changeMode(0);
    initDate();
    s_clickDate = date;
    dayChanged(this->date, QDate());
    CalendarDataBase::getInstance().searchDataSlot(date);
    //    QPoint currentPos = this->pos();
    //    this->move(currentPos.x(),670);
    updateAllSchedule();
    updateLayout();
}

void LunarCalendarWidget::setCalendarStyle(const LunarCalendarWidget::CalendarStyle &calendarStyle)
{
    if (this->calendarStyle != calendarStyle)
    {
        this->calendarStyle = calendarStyle;
    }
}

void LunarCalendarWidget::setWeekNameFormat(bool FirstDayisSun)
{
    FirstdayisSun = FirstDayisSun;
    setLocaleCalendar();//在设置是从某星期作为起点时,再判断某区域下日历的显示
    initDate();
}

void LunarCalendarWidget::setDate(const QDate &date)
{
    if (this->date != date)
    {
        this->date = date;
        initDate();
    }
}

void LunarCalendarWidget::setWeekTextColor(const QColor &weekTextColor)
{
    if (this->weekTextColor != weekTextColor)
    {
        this->weekTextColor = weekTextColor;
        initStyle();
    }
}

void LunarCalendarWidget::setWeekBgColor(const QColor &weekBgColor)
{
    if (this->weekBgColor != weekBgColor)
    {
        this->weekBgColor = weekBgColor;
        initStyle();
    }
}

void LunarCalendarWidget::setBgImage(const QString &bgImage)
{
    if (this->bgImage != bgImage)
    {
        this->bgImage = bgImage;
        initStyle();
    }
}

void LunarCalendarWidget::setSelectType(const LunarCalendarWidget::SelectType &selectType)
{
    if (this->selectType != selectType)
    {
        this->selectType = selectType;
        initStyle();
    }
}

void LunarCalendarWidget::setBorderColor(const QColor &borderColor)
{
    if (this->borderColor != borderColor)
    {
        this->borderColor = borderColor;
        initStyle();
    }
}

void LunarCalendarWidget::setWeekColor(const QColor &weekColor)
{
    if (this->weekColor != weekColor)
    {
        this->weekColor = weekColor;
        initStyle();
    }
}

void LunarCalendarWidget::setSuperColor(const QColor &superColor)
{
    if (this->superColor != superColor)
    {
        this->superColor = superColor;
        initStyle();
    }
}

void LunarCalendarWidget::setLunarColor(const QColor &lunarColor)
{
    if (this->lunarColor != lunarColor)
    {
        this->lunarColor = lunarColor;
        initStyle();
    }
}

void LunarCalendarWidget::setCurrentTextColor(const QColor &currentTextColor)
{
    if (this->currentTextColor != currentTextColor)
    {
        this->currentTextColor = currentTextColor;
        initStyle();
    }
}

void LunarCalendarWidget::setOtherTextColor(const QColor &otherTextColor)
{
    if (this->otherTextColor != otherTextColor)
    {
        this->otherTextColor = otherTextColor;
        initStyle();
    }
}

void LunarCalendarWidget::setSelectTextColor(const QColor &selectTextColor)
{
    if (this->selectTextColor != selectTextColor)
    {
        this->selectTextColor = selectTextColor;
        initStyle();
    }
}

void LunarCalendarWidget::setHoverTextColor(const QColor &hoverTextColor)
{
    if (this->hoverTextColor != hoverTextColor)
    {
        this->hoverTextColor = hoverTextColor;
        initStyle();
    }
}

void LunarCalendarWidget::setCurrentLunarColor(const QColor &currentLunarColor)
{
    if (this->currentLunarColor != currentLunarColor)
    {
        this->currentLunarColor = currentLunarColor;
        initStyle();
    }
}

void LunarCalendarWidget::setOtherLunarColor(const QColor &otherLunarColor)
{
    if (this->otherLunarColor != otherLunarColor)
    {
        this->otherLunarColor = otherLunarColor;
        initStyle();
    }
}

void LunarCalendarWidget::setSelectLunarColor(const QColor &selectLunarColor)
{
    if (this->selectLunarColor != selectLunarColor)
    {
        this->selectLunarColor = selectLunarColor;
        initStyle();
    }
}

void LunarCalendarWidget::setHoverLunarColor(const QColor &hoverLunarColor)
{
    if (this->hoverLunarColor != hoverLunarColor)
    {
        this->hoverLunarColor = hoverLunarColor;
        initStyle();
    }
}

void LunarCalendarWidget::setCurrentBgColor(const QColor &currentBgColor)
{
    if (this->currentBgColor != currentBgColor)
    {
        this->currentBgColor = currentBgColor;
        initStyle();
    }
}

void LunarCalendarWidget::setOtherBgColor(const QColor &otherBgColor)
{
    if (this->otherBgColor != otherBgColor)
    {
        this->otherBgColor = otherBgColor;
        initStyle();
    }
}

void LunarCalendarWidget::setSelectBgColor(const QColor &selectBgColor)
{
    if (this->selectBgColor != selectBgColor)
    {
        this->selectBgColor = selectBgColor;
        initStyle();
    }
}

void LunarCalendarWidget::setHoverBgColor(const QColor &hoverBgColor)
{
    if (this->hoverBgColor != hoverBgColor)
    {
        this->hoverBgColor = hoverBgColor;
        initStyle();
    }
}
void LunarCalendarWidget::changeMonth(bool forward)
{
    int year = date.year();
    int month = date.month();
    int day = date.day();
    if (year <= 1901 && month == 1)
    {
        return;
    }
    if(forward)
    {
        month += 3;
        if(month > 12)
        {
            year += 1;
            month -= 12;
        }
    }
    else
    {
        month -= 3;
        if(month <= 0)
        {
            year -= 1;
            month += 12;
        }
    }

    dateChanged(year, month, day);
}
void LunarCalendarWidget::changeYear(bool forward)
{
    int year = date.year();
    int month = date.month();
    int day = date.day();
    if (year <= 1901 || year >= 2099)
    {
        return;
    }

    if(forward)
        year += 3;
    else
        year -= 3;

    dateChanged(year, month, day);
}
void LunarCalendarWidget::wheelEvent(QWheelEvent *event)
{

    if (scrollArea->underMouse())
    {
        if (scrollArea->TopOrButtom)
        {
            event->ignore();
            return;
        }
    }

    switch (m_currentType)
    {
    case 0:
        if (event->delta() > 100)
        {
            showPreviousMonth();
        }
        else if (event->delta() < -100)
        {
            showNextMonth();
        }
        break;

    case 2:
        if (event->delta() > 100)
        {
            //向前三年
            changeYear(false);
        }
        else if (event->delta() < -100)
        {
            //向后三年
            changeYear(true);
        }
        break;
    default:

    case 1:
        break;
    }
    event->accept();
    //    return QWidget::wheelEvent(event);

}

bool  LunarCalendarWidget::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mouse = static_cast<QMouseEvent*>(event);
        if(mouse != nullptr)
        {
            if(watched == this)
            {
                if(mouse->source() == Qt::MouseEventSynthesizedByQt)
                {
                    m_start_y = mouse->pos().y();
                }
                if(m_currentType == 0)
                {
                    QPoint pt = m_widgetDayBody->mapFromGlobal(QCursor::pos());
                    for(auto w : dayItems)
                    {
                        if(w->geometry().contains(pt))
                            m_clickedWidget = w;

                    }
                }
                else if(m_currentType == 2)
                {
                    QPoint pt = widgetYearBody->mapFromGlobal(QCursor::pos());
                    for(auto w : yearItems)
                    {
                        if(w->geometry().contains(pt))
                            m_clickedWidget = w;

                    }
                }
                else if(m_currentType == 1)
                {
                    QPoint pt = widgetmonthBody->mapFromGlobal(QCursor::pos());
                    for(auto w : monthItems)
                    {
                        if(w->geometry().contains(pt))
                            m_clickedWidget = w;
                    }
                }
                return false;
            }
        }
    }
    else if(event->type() == QEvent::MouseButtonRelease)
    {
        //return -1;
        QMouseEvent *mouse = static_cast<QMouseEvent*>(event);
        if(mouse != nullptr)
        {
            if(watched == this)
            {
                if(mouse->source() == Qt::MouseEventSynthesizedByQt)
                {
                    //触摸的释放
                    if(m_start_y == -1)
                        return false;

                    if( mouse->pos().y() - m_start_y > 100)
                    {
                        switch(m_currentType)
                        {
                        case 0:
                            showNextMonth();
                            break;
                        case 2:
                            changeYear(true);
                            break;
                        }
                        return false;
                    }
                    else if(mouse->pos().y() - m_start_y < -100)
                    {
                        switch(m_currentType)
                        {
                        case 0:
                            showPreviousMonth();
                            break;
                        case 2:
                            changeYear(false);
                            break;
                        }
                        return false;
                    }
                }
                //鼠标抬起
                if(m_clickedWidget == nullptr)
                {
                    return false;
                }
                if(m_clickedWidget->geometry().contains(m_widgetDayBody->mapFromGlobal(QCursor::pos())))
                {
                    LunarCalendarItem *item = qobject_cast<LunarCalendarItem*>(m_clickedWidget);
                    if(dayItems.contains(item))
                        item->onSelected();

                    LunarCalendarYearItem *yItem = qobject_cast<LunarCalendarYearItem*>(m_clickedWidget);
                    if(yearItems.contains(yItem))
                        yItem->onSelected();

                    LunarCalendarMonthItem *mItem = qobject_cast<LunarCalendarMonthItem*>(m_clickedWidget);
                    if(monthItems.contains(mItem))
                        mItem->onSelected();

                }
                return false;
            }
        }
    }
    else if (event->type() == QEvent::MouseMove)
    {
        QMouseEvent *mouse = static_cast<QMouseEvent*>(event);
        if(mouse != nullptr)
        {
            if(mouse->source() == Qt::MouseEventSynthesizedByQt)
            {
                return -1;
            }
        }
        if(m_currentType == 0)
        {
            for(auto w : dayItems)
            {
                if(w->geometry().contains(m_widgetDayBody->mapFromGlobal(QCursor::pos())))
                {
                    w->onEnter();
                }
                else
                {
                    w->onLeave();
                }
            }
        }
        else if(m_currentType == 2)
        {
            for(auto w : yearItems)
            {
                if(w->geometry().contains(widgetYearBody->mapFromGlobal(QCursor::pos())))
                {
                    w->onEnter();
                }
                else
                {
                    w->onLeave();
                }
            }
        }
        else if(m_currentType == 1)
        {
            for(auto w : monthItems)
            {
                if(w->geometry().contains(widgetmonthBody->mapFromGlobal(QCursor::pos())))
                {
                    w->onEnter();
                }
                else
                {
                    w->onLeave();
                }
            }
        }
        return false;
    }
    else if(event->type() == QEvent::Enter)
    {
        LunarCalendarItem *item = qobject_cast<LunarCalendarItem*>(watched);
        if(dayItems.contains(item) == true)
        {
            item->onEnter();
        }
        LunarCalendarYearItem *yItem = qobject_cast<LunarCalendarYearItem*>(watched);
        if(yearItems.contains(yItem))
        {
            yItem->onEnter();
        }
    }
    else if(event->type() == QEvent::Leave)
    {
        for(auto day : dayItems)
            day->onLeave();
        for (auto month : monthItems)
            month->onLeave();
        for(auto year : yearItems)
            year->onLeave();

    }
    else if(event->type() == QEvent::WindowDeactivate)
    {
        Q_EMIT NeedToHide();
        this->hide();
        return false;
    }

    return false;
}
void LunarCalendarWidget::showCalendar(bool needShowToday)
{
    KWindowSystem::setState(winId(), NET::SkipPager | NET::SkipTaskbar | NET::SkipSwitcher);

    show();
    KWindowSystem::setState(winId(), NET::SkipPager | NET::SkipTaskbar | NET::SkipSwitcher);
    //    KWindowEffects::slideWindow(winId(), KWindowEffects::BottomEdge, height());


    if(needShowToday)
    {
        showToday();
    }
    //    isClicked = false;

}
void LunarCalendarWidget::initTransparency()
{
    const QByteArray tranparency(TRANSPARENCY_SETTING);
    if (QGSettings::isSchemaInstalled(tranparency))
    {
        m_transparencySetting = new QGSettings(tranparency);
        m_trans = m_transparencySetting->get(TRANSPARENCY_KEY).toDouble() * 255;
        update();
        connect(m_transparencySetting, &QGSettings::changed, this, [ = ](const QString &key)
        {
            if(key == TRANSPARENCY_KEY)
            {
                m_trans = m_transparencySetting->get(TRANSPARENCY_KEY).toDouble() * 255;
                update();
            }
        });
    }
    else
    {
        m_trans = 0.75;
        update();
    }
}
void LunarCalendarWidget::paintEvent(QPaintEvent*e)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    QColor color = palette().color(QPalette::Base);
    color.setAlpha(m_trans);
    QBrush brush = QBrush(color);
    p.setBrush(brush);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    p.setPen(Qt::NoPen);
    p.drawRect(opt.rect);
    return QWidget::paintEvent(e);
}

PartLineWidget::PartLineWidget(QWidget *parent) : QWidget(parent)
{

}

void PartLineWidget::paintEvent(QPaintEvent *event)
{
    QPainter p(this);

    QRect rect = this->rect();

    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    QColor color = qApp->palette().color(QPalette::BrightText);
    p.setBrush(color);
    p.setOpacity(0.08);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(rect, 0, 0);

    QWidget::paintEvent(event);
}

statelabel::statelabel() : QLabel()
{


}

//鼠标点击事件
void statelabel::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton)
    {
        Q_EMIT labelclick();

    }
    return;
}
//快捷键
void LunarCalendarWidget::keyPressEvent(QKeyEvent *ev)
{
    if(ev->key() == Qt::Key_Escape)
    {
        this->hide();
    }
    QWidget::keyPressEvent(ev);
}

void LunarCalendarWidget::updateYearMonthString(int year, int month)
{
    if(cboxYearandMonthLabel == nullptr)
    {
        qInfo() << "widget is not initialized...";
        return ;
    }

    QDate date(year, month, 1);
    QString shortFmt = SDK2CPP(kdk_system_get_shortformat)();
    shortFmt.replace("d", "");
    if(shortFmt.lastIndexOf(".") != -1)
    {
        shortFmt.replace("..", ".");
    }
    else if(shortFmt.lastIndexOf("-") != -1)
    {
        shortFmt.replace("--", "-");
    }
    else if(shortFmt.lastIndexOf("/") != -1)
    {
        shortFmt.replace("//", "/");
    }
    else
    {
        qInfo() << "Wrong date format...";
        return ;
    }
    if(shortFmt.endsWith(".") || shortFmt.endsWith("/") || shortFmt.endsWith("-"))
    {
        shortFmt.chop(1);
    }
    cboxYearandMonthLabel->setText(date.toString(shortFmt));

}
void LunarCalendarWidget::updateLayout()
{
    updateLayout1();
    updateSchedule();
}
void LunarCalendarWidget::updateLayout1()
{
    if(scrollArea)
    {
        scrollArea->deleteLater();
        scrollArea = nullptr; // 将指针设置为nullptr
    }
    qDebug() << "no have such scrollArea";
    scrollArea = new CustomScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("background-color: transparent;");
    if(m_verLayoutCalendar)
    {
        delete m_verLayoutCalendar;
        m_verLayoutCalendar = new QVBoxLayout(this);
    }
    else if(!m_verLayoutCalendar)
    {
        m_verLayoutCalendar = new QVBoxLayout(this);
    }
    if(m_markInfoList.count() != 0)
    {
        if(label)
        {
            label->deleteLater();
            label = nullptr;
        }
        //        label = new ShowMoreLabel(this);
    }
    if(scheduleWidget)
    {
        scheduleWidget->deleteLater();
        scheduleWidget = nullptr;
    }
}
void LunarCalendarWidget::onScreenGeometryChanged()
{
    screen = qGuiApp->primaryScreen();
    if (sender() == screen)
    {
        updatePrimaryScreenHeight();
    }
}
void LunarCalendarWidget::updatePrimaryScreenHeight()
{

    screen = qGuiApp->primaryScreen();

    screenHeight = qApp->screenAt(QCursor::pos())->size().height() + qApp->screenAt(QCursor::pos())->geometry().y();
    panelHeight = screenHeight - panelSize;

    if(LESSTWOSHCEDULE_CALENDAR_HEIGHT < panelHeight && panelHeight < LESSTWOSHCEDULE_CALENDAR_HEIGHT + YIJI_HEIGHT)
    {
        ScrollAreaNeedChange = true;
    }
    else
    {
        ScrollAreaNeedChange = false;
    }
    if(panelHeight < LESSTWOSHCEDULE_CALENDAR_HEIGHT)
    {
        ScrollbarUnderTheResolution = true;
    }
    else
    {
        ScrollbarUnderTheResolution = false;
    }
    qDebug() << "Primary screen height changed to:" << panelSize;
}
void LunarCalendarWidget::onPrimaryScreenChanged(const QRect &newGeometry)
{
    qDebug() << "分辨率变化";
    screen = qobject_cast<QScreen *>(sender());
    // 当主屏幕变化时，更新主屏幕和高度
    qDebug() << "Screen geometry changed:" << screen->name() << newGeometry;
    //    screen = newScreen;
    primaryScreenHeight = qApp->screenAt(QCursor::pos())->size().height() + qApp->screenAt(QCursor::pos())->geometry().y();

    updatePrimaryScreenHeight();

}
void LunarCalendarWidget::onGSettingsChanged(const QString &key)
{
    qDebug() << "onGSettingsChanged";
    if(key == PANEL_POSITION_KEY)
    {
        int calendar_height = LESSTWOSHCEDULE_CALENDAR_HEIGHT;
        if(panelHeight < this->height() || panelHeight < LESSTWOSHCEDULE_CALENDAR_HEIGHT || isExceedScreenHeight)
        {
            calendar_height = panelHeight - SCROLLA;
        }
        int width = screen->size().width();
        int height = primaryScreenHeight;
        int m_panelPoistion = m_panelGsettings->get(PANEL_POSITION_KEY).toInt();
        qDebug() << "-----------------m_panelPoistion" << m_panelPoistion;
        position = m_panelPoistion;
        int  m_panelSize = m_panelGsettings->get(PANEL_SIZE_KEY).toInt();
        //3右 0下 2左 1上
        switch (m_panelPoistion)
        {
        case 0:
            this->move(width - ORIGIN_CALENDAR_WIDTH - WIDGET_GAP, panelHeight - calendar_height - WIDGET_GAP);
            if(yijistate)
            {
                this->move(width - ORIGIN_CALENDAR_WIDTH - WIDGET_GAP, panelHeight - calendar_height - YIJI_HEIGHT - WIDGET_GAP);
            }
            break;
        case 1:
            this->move(width - ORIGIN_CALENDAR_WIDTH - WIDGET_GAP, m_panelSize + WIDGET_GAP);
            break;
        case 2:
            this->move(qApp->screenAt(QCursor::pos())->geometry().x() + m_panelSize + WIDGET_GAP, height - calendar_height - WIDGET_GAP);
            if(yijistate)
            {
                this->move(qApp->screenAt(QCursor::pos())->geometry().x() + m_panelSize + WIDGET_GAP, height - calendar_height - YIJI_HEIGHT - WIDGET_GAP);
            }
            break;
        case 3:
            this->move(width - m_panelSize - ORIGIN_CALENDAR_WIDTH - WIDGET_GAP, height - calendar_height - WIDGET_GAP);
            if(yijistate)
            {
                this->move(width - m_panelSize - ORIGIN_CALENDAR_WIDTH - WIDGET_GAP, height - calendar_height - YIJI_HEIGHT - WIDGET_GAP);
            }
            break;
        default:
            break;
        }
    }
    if(key == PANEL_SIZE_KEY)
    {
        panelSize = m_panelGsettings->get(PANEL_SIZE_KEY).toInt();
        screenHeight = qApp->screenAt(QCursor::pos())->size().height() + qApp->screenAt(QCursor::pos())->geometry().y();
        panelHeight = primaryScreenHeight - panelSize; //这个值就是屏幕到任务栏的高度

    }
}
void LunarCalendarWidget::ThreeMoreSchedule()
{
    int num = m_markInfoList.count();
    QPoint currentPos = this->pos();
    this->setFixedSize(ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT); //900是三个以上日程的默认高度
    m_scheduleWidget = new QWidget(this);
    m_scheduleWidget->setContentsMargins(0, 0, 0, 23);
    m_scheduleWidget->setStyleSheet("background-color: transparent;");
    m_scheduleWidget->setFixedHeight(79 * num);
    labWidget->setContentsMargins(16, 0, 16, 0);
    yijiWidget->setContentsMargins(16, 0, 16, 0);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    //scrollArea->setFixedSize(ORIGIN_CALENDAR_WIDTH-20, 140);
    scrollArea->setFixedWidth(ORIGIN_CALENDAR_WIDTH);
    scrollArea->verticalScrollBar()->setStyleSheet(ScrollAreaStyleSheet);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //把前两个日程先显示出来 这部分不用修改
    foreach(info, m_markInfoList)
    {
        scheduleWidget = new schedule_item(this, true);
        QString time1;
        QString time2;
        if(timemodel == "12")
        {
            scheduleWidget->timemode = "12";
            if(info.hour <= 12)
            {
                time1 = tr("morning");
            }
            else
            {
                time1 = tr("afternoon");
                info.hour = info.hour - 12;
            }
            if(info.end_hour < 12)
            {
                time2 = tr("morning");
            }
            else
            {
                time2 = tr("afternoon");
                info.end_hour = info.end_hour - 12;
            }
        }
        else if(timemodel == "24")
        {
            time1 = "";
            time2 = "";
        }
        QString starttimeString = QString::number(info.hour) + ":" + QString::number(info.minute); // 确保这是有效的格式
        QTime m_startTime = QTime::fromString(starttimeString, "h:m");
        QString start = m_startTime.toString("hh:mm");
        QString timeString = QString::number(info.end_hour) + ":" + QString::number(info.end_minute); // 确保这是有效的格式
        QTime m_endTime = QTime::fromString(timeString, "h:m");
        QString end = m_endTime.toString("hh:mm");
        QString  lineOne = QString(" %1%2-%3%4 ").arg(time1).arg(start).arg(time2).arg(end)
                           + info.timeLong;
        if(info.m_isAllDay)
        {
            lineOne = tr("   allday");
        }
        scheduleWidget->setStartTime(lineOne);
        scheduleWidget->adjustDate(this->date);
        scheduleWidget->setEndTime(info.end_hour, info.end_minute, this->date, time2);
        scheduleWidget->setDescription(info.m_descript);
        scheduleWidget->setId(info.m_markId);
        connect(scheduleWidget, &schedule_item::ScheduleClick, this, &LunarCalendarWidget::slotEditSchedule);
        connect(scheduleWidget, &schedule_item::EditSchedule, this, &LunarCalendarWidget::slotEditSchedule);
        connect(scheduleWidget, &schedule_item::updateWindow, this, &LunarCalendarWidget::updateFromItem);
        item_layout->addWidget(scheduleWidget);
    }
    m_scheduleWidget->setLayout(item_layout);
    scrollArea->setWidget(m_scheduleWidget);
    m_horLayoutWidgetTop = new QHBoxLayout(this);
    m_horLayoutWidgetTop->setContentsMargins(26, 0, 24, 0);
    m_verLayoutCalendar->setMargin(0);
    m_verLayoutCalendar->setSpacing(0);
    m_horLayoutWidgetTop->addWidget(widgetTime);
    m_horLayoutWidgetTop->addWidget(m_createScheduleButton);
    m_verLayoutCalendar->addLayout(m_horLayoutWidgetTop);
    m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
    m_verLayoutCalendar->addWidget(lineUp);
    m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
    m_verLayoutCalendar->addWidget(m_widgetTop);
    m_verLayoutCalendar->addSpacing(18);
    m_verLayoutCalendar->addWidget(widgetWeek);
    m_verLayoutCalendar->addWidget(m_widgetDayBody, 1);
    m_verLayoutCalendar->addWidget(widgetYearBody, 1);
    m_verLayoutCalendar->addWidget(widgetmonthBody, 1);
    m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
    m_verLayoutCalendar->addWidget(lineDown);
    controlDownLine();
    m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
    m_verLayoutCalendar->addWidget(labWidget);
    m_verLayoutCalendar->addWidget(yijiWidget);
    m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
    lineSchedule->setFixedHeight(1);
    m_verLayoutCalendar->addWidget(lineSchedule);
    m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
    m_verLayoutCalendar->addWidget(scrollArea);
    if(ScrollAreaNeedChange)
    {
        qDebug() << "ScrollAreaNeedChange";

        if(yijistate)
        {
            scrollArea->setFixedSize(420, 80);
        }
        else
        {
            scrollArea->setFixedWidth(ORIGIN_CALENDAR_WIDTH);
        }
        //        scrollArea->setFixedWidth(ORIGIN_CALENDAR_WIDTH);
    }
    else
    {
        if(yijistate)
        {
            this->setFixedSize(ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT + YIJI_HEIGHT);
        }
        else
        {
            this->setFixedSize(ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT); //无日程的时候，日历高度固定600,不会根据分辨率变化大小
        }
        QRect rect;
        int totalHeight = qApp->screenAt(QCursor::pos())->size().height() + qApp->screenAt(QCursor::pos())->geometry().y();//屏幕高度
        int totalWidth = qApp->screenAt(QCursor::pos())->size().width() + qApp->screenAt(QCursor::pos())->geometry().x();
        switch(position)
        {
        case 0:
            if(yijistate)
            {
                rect.setRect(totalWidth - ORIGIN_CALENDAR_WIDTH - WIDGET_GAP, totalHeight - panelSize - LESSTWOSHCEDULE_CALENDAR_HEIGHT - YIJI_HEIGHT - WIDGET_GAP,
                             ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT + YIJI_HEIGHT);
            }
            else
            {
                rect.setRect(totalWidth - ORIGIN_CALENDAR_WIDTH - WIDGET_GAP, totalHeight - panelSize - LESSTWOSHCEDULE_CALENDAR_HEIGHT - WIDGET_GAP,
                             ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT);
                if(isRTLLanguage())
                {
                    qDebug() << "QLocale::system().language();" << QLocale::system().language();
                    rect.setRect(qApp->screenAt(QCursor::pos())->geometry().x() + WIDGET_GAP, totalHeight - LESSTWOSHCEDULE_CALENDAR_HEIGHT - WIDGET_GAP,
                                 ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT);
                }

            }
            kdk::WindowManager::setGeometry(this->windowHandle(), rect);
            break;
        case 1:
            if(yijistate)
            {
                rect.setRect(totalWidth - ORIGIN_CALENDAR_WIDTH - WIDGET_GAP, qApp->screenAt(QCursor::pos())->geometry().y() + panelSize + WIDGET_GAP,
                             ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT + YIJI_HEIGHT);
            }
            else
            {
                rect.setRect(totalWidth - ORIGIN_CALENDAR_WIDTH - WIDGET_GAP, qApp->screenAt(QCursor::pos())->geometry().y() + panelSize + WIDGET_GAP,
                             ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT);
            }
            kdk::WindowManager::setGeometry(this->windowHandle(), rect);
            break;
        case 2:
            if(yijistate)
            {
                rect.setRect(qApp->screenAt(QCursor::pos())->geometry().x() + panelSize + WIDGET_GAP, totalHeight - LESSTWOSHCEDULE_CALENDAR_HEIGHT - WIDGET_GAP - YIJI_HEIGHT,
                             ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT + YIJI_HEIGHT);
            }
            else
            {
                rect.setRect(qApp->screenAt(QCursor::pos())->geometry().x() + panelSize + WIDGET_GAP, totalHeight - LESSTWOSHCEDULE_CALENDAR_HEIGHT - WIDGET_GAP,
                             ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT);
            }
            kdk::WindowManager::setGeometry(this->windowHandle(), rect);
            break;
        case 3:
            if(yijistate)
            {
                rect.setRect(totalWidth - panelSize - ORIGIN_CALENDAR_WIDTH - WIDGET_GAP, totalHeight - LESSTWOSHCEDULE_CALENDAR_HEIGHT - WIDGET_GAP - YIJI_HEIGHT,
                             ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT + YIJI_HEIGHT);
            }
            else
            {
                rect.setRect(totalWidth - panelSize - ORIGIN_CALENDAR_WIDTH - WIDGET_GAP, totalHeight - LESSTWOSHCEDULE_CALENDAR_HEIGHT - WIDGET_GAP,
                             ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT);
            }
            kdk::WindowManager::setGeometry(this->windowHandle(), rect);
            break;
        default:
            break;
        }
    }
}
void LunarCalendarWidget::PrimaryScreenHeight(int height)
{
    qDebug() << "点击了不同屏幕" << height;
    primaryScreenHeight = height;

    updatePrimaryScreenHeight();
}
void LunarCalendarWidget::initGsettings()
{
    const QByteArray id(PANEL_SETTINGS);
    if (QGSettings::isSchemaInstalled(id))
    {
        m_panelGsettings = new QGSettings(id);
        if (m_panelGsettings != nullptr)
        {
            panelSize = m_panelGsettings->get(PANEL_SIZE_KEY).toInt();
            screenHeight = qApp->screenAt(QCursor::pos())->size().height() + qApp->screenAt(QCursor::pos())->geometry().y();
            panelHeight = primaryScreenHeight - panelSize; //这个值就是屏幕到任务栏的高度
            position = m_panelGsettings->get(PANEL_POSITION_KEY).toInt();
            connect(m_panelGsettings, &QGSettings::changed, this, &LunarCalendarWidget::onGSettingsChanged);
        }
    }
    connect(screen, &QScreen::geometryChanged, this, &LunarCalendarWidget::onPrimaryScreenChanged);
    widgetTime->setObjectName("widgetTime");

    const QByteArray calendar_id(PANEL_CONTROL_IN_CALENDAR);
    if(QGSettings::isSchemaInstalled(calendar_id))
    {
        calendar_gsettings = new QGSettings(calendar_id);
    }
    initWidget();
    if(calendar_gsettings != nullptr)
    {
        //农历切换监听与日期显示格式
        connect(calendar_gsettings, &QGSettings::changed, this, [ = ] (const QString &key)
        {
            if(key == LUNAR_KEY)
            {
                m_showLunar = getShowLunar();
                setShowLunar(m_showLunar);
                controlDownLine();
                _timeUpdate();
            }
            else if(key == "date")
            {
            }
            else if (key == FIRST_DAY_KEY)
            {
                setWeekNameFormat(calendar_gsettings->get(FIRST_DAY_KEY).toString() == "sunday");
            }
        });
        m_showLunar = getShowLunar();
        setShowLunar(m_showLunar);
        setWeekNameFormat(calendar_gsettings->get(FIRST_DAY_KEY).toString() == "sunday");
        //监听12/24小时制
        connect(calendar_gsettings, &QGSettings::changed, this, [ = ] (const QString &keys)
        {
            timemodel = calendar_gsettings->get("hoursystem").toString();
            _timeUpdate();
        });
        timemodel = calendar_gsettings->get("hoursystem").toString();

    }
    else
    {
        //无设置默认公历
        lunarstate = true;
    }

    //使用系统提供的sdk刷新时间显示
    QDBusConnection::sessionBus().connect(LINGMOSDK_DATESERVER,
                                          LINGMOSDK_DATEPATH,
                                          LINGMOSDK_DATEINTERFACE,
                                          "ShortDateSignal",
                                          this,
                                          SLOT([](QString)
    {
        updateYearMonthString(QDateTime::currentDateTime().date().year(), QDateTime::currentDateTime().date().month());
    }
                                              )
                                         );

    //判断图形字体是否存在,不存在则加入
    QFontDatabase fontDb;
    if (!fontDb.families().contains("FontAwesome"))
    {
        int fontId = fontDb.addApplicationFont(":/image/fontawesome-webfont.ttf");
        QStringList fontName = fontDb.applicationFontFamilies(fontId);
        if (fontName.count() == 0)
        {
            qDebug() << "load fontawesome-webfont.ttf error";
        }
    }

    if (fontDb.families().contains("FontAwesome"))
    {
        iconFont = QFont("FontAwesome");
#if (QT_VERSION >= QT_VERSION_CHECK(4,8,0))
        iconFont.setHintingPreference(QFont::PreferNoHinting);
#endif
    }
    //切换主题
    const QByteArray style_id(ORG_LINGMO_STYLE);
    QStringList stylelist;
    stylelist << STYLE_NAME_KEY_DARK << STYLE_NAME_KEY_BLACK;
    if(QGSettings::isSchemaInstalled(style_id))
    {
        style_settings = new QGSettings(style_id);
        if (style_settings != nullptr)
        {
            dark_style = stylelist.contains(style_settings->get(STYLE_NAME).toString());
            setColor(dark_style);
            m_fontName = style_settings->get(SYSTEM_FONT).toString();
            m_font.setFamily(m_fontName);
        }
    }
    cboxYearandMonthLabel->setFont(m_font);
    if (style_settings != nullptr)
    {
        connect(style_settings, &QGSettings::changed, this, [ = ] (const QString &key)
        {
            if(key == STYLE_NAME)
            {
                dark_style = stylelist.contains(style_settings->get(STYLE_NAME).toString());
                _timeUpdate();
                setColor(dark_style);
            }
            else if(key == SYSTEM_FONT)
            {
                m_fontName = style_settings->get(SYSTEM_FONT).toString();
                m_font.setFamily(m_fontName);
            }
            else
            {
                qDebug() << "key!=STYLE_NAME";
            }
        });
    }
    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
    timer->start(1000);

    locale = QLocale::system().name();

    setLocaleCalendar();//设置某区域下日历的显示
    initTransparency();
    setWindowOpacity(1);
    setAttribute(Qt::WA_TranslucentBackground, true); //设置窗口背景透明
    setProperty("useSystemStyleBlur", true);   //设置毛玻璃效果
    setProperty("useStyleWindowManager", false);

    KWindowEffects::enableBlurBehind(winId(), true);
    KWindowSystem::setState(winId(), NET::SkipPager | NET::SkipTaskbar | NET::SkipSwitcher);
    kdk::LingmoUIStyleHelper::self()->removeHeader(this);
    updateYearMonthString(QDateTime::currentDateTime().date().year(), QDateTime::currentDateTime().date().month());
    _timeUpdate();

    //加载繁中宜忌字典
    if (locale != "zh_HK")
    {
        return;
    }
    QFile dict("/usr/share/lingmo-panel/plugin-calendar/html/zh_HK.html");
    if (!dict.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "open field : /usr/share/lingmo-panel/plugin-calendar/html/zh_HK.html";
        return;
    }
    QString orig = dict.readLine();
    QString trans = dict.readLine();
    orig.remove("\n");
    for (int i = 0; i < orig.length(); i++)
    {
        if (i >= trans.length())
        {
            break;
        }
        m_zh_HK_dict.insert(orig.at(i), trans.at(i));
    }
}
void LunarCalendarWidget::DisplayAtResolution()
{
    this->setFixedSize(ORIGIN_CALENDAR_WIDTH, panelHeight - SCROLLA);
    int num = m_markInfoList.count();
    if(num == 0)
    {
        scrollArea->verticalScrollBar()->setStyleSheet(ScrollAreaStyleSheet);
        m_scheduleWidget = new QWidget(this);
        m_scheduleWidget->setContentsMargins(0, 0, 0, 23);
        m_scheduleWidget->setStyleSheet("background-color: transparent;");
        m_scheduleWidget->setFixedSize(ORIGIN_CALENDAR_WIDTH, 220);
        //scrollArea->setFixedSize(420, 140);
        scrollArea->setFixedWidth(ORIGIN_CALENDAR_WIDTH);
        scheduleWidget = new schedule_item(this, false);
        scheduleWidget->setStartTime("");
        scheduleWidget->setDescription("");
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        labWidget->setContentsMargins(16, 0, 16, 0);
        yijiWidget->setContentsMargins(16, 0, 16, 0);
        if(yijistate)
        {
            labWidget->setContentsMargins(5, 0, 10, 0);
            yijiWidget->setContentsMargins(5, 0, 0, 0);
        }
        //       item_layout->addItem(new QSpacerItem(6,6));
        item_layout->addWidget(labWidget);
        item_layout->addStretch(0);
        item_layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        item_layout->addWidget(yijiWidget);
        item_layout->addWidget(scheduleWidget);
        m_scheduleWidget->setLayout(item_layout);
        scrollArea->setWidget(m_scheduleWidget);
        m_horLayoutWidgetTop = new QHBoxLayout(this);
        m_horLayoutWidgetTop->setContentsMargins(26, 0, 24, 0);
        m_verLayoutCalendar->setMargin(0);
        m_verLayoutCalendar->setSpacing(0);

        m_horLayoutWidgetTop->addWidget(widgetTime);
        m_horLayoutWidgetTop->addWidget(m_createScheduleButton);
        m_verLayoutCalendar->addLayout(m_horLayoutWidgetTop);
        m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
        //          verLayoutCalendar->addWidget(lineUp);
        m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
        m_verLayoutCalendar->addWidget(m_widgetTop);
        m_verLayoutCalendar->addSpacing(18);
        m_verLayoutCalendar->addWidget(widgetWeek);
        m_verLayoutCalendar->addWidget(m_widgetDayBody, 1);
        m_verLayoutCalendar->addWidget(widgetYearBody, 1);
        m_verLayoutCalendar->addWidget(widgetmonthBody, 1);
        m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
        m_verLayoutCalendar->addWidget(lineDown);
        controlDownLine();
        m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
        lineSchedule->setFixedHeight(0.1);
        m_verLayoutCalendar->addWidget(lineSchedule);
        m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
        m_verLayoutCalendar->addWidget(scrollArea);
    }
    else
    {
        scrollArea->verticalScrollBar()->setStyleSheet(ScrollAreaStyleSheet);
        m_scheduleWidget = new QWidget(this);
        m_scheduleWidget->setContentsMargins(0, 0, 0, 23);
        m_scheduleWidget->setStyleSheet("background-color: transparent;");
        m_scheduleWidget->setFixedSize(ORIGIN_CALENDAR_WIDTH, 140 + 70 * num); //确保日程越多，可滑动区域越大
        //scrollArea->setFixedSize(420,140);
        scrollArea->setFixedWidth(ORIGIN_CALENDAR_WIDTH);
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        //        item_layout->addItem(new QSpacerItem(6,6));
        //item_layout->addWidget(lineSchedule);
        labWidget->setContentsMargins(16, 0, 16, 0);
        yijiWidget->setContentsMargins(16, 0, 16, 0);
        if(yijistate)
        {
            labWidget->setContentsMargins(5, 0, 10, 0);
            yijiWidget->setContentsMargins(5, 0, 0, 0);
        }

        item_layout->addWidget(labWidget);
        item_layout->addStretch(0);
        item_layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        item_layout->addWidget(yijiWidget);
        item_layout->addWidget(scheduleWidget);
        foreach(info, m_markInfoList)
        {
            scheduleWidget = new schedule_item(this, true);
            QString time1;
            QString time2;
            if(timemodel == "12")
            {
                scheduleWidget->timemode = "12";
                if(info.hour <= 12)
                {
                    time1 = tr("morning");
                }
                else
                {
                    time1 = tr("afternoon");
                    info.hour = info.hour - 12;
                    //            info.end_hour = info.end_hour-12;
                }
                if(info.end_hour < 12)
                {
                    time2 = tr("morning");
                }
                else
                {
                    time2 = tr("afternoon");
                    //            info.hour = info.hour-12;
                    info.end_hour = info.end_hour - 12;
                }

            }
            else if(timemodel == "24")
            {
                time1 = "";
                time2 = "";
            }

            QString starttimeString = QString::number(info.hour) + ":" + QString::number(info.minute); // 确保这是有效的格式
            QTime m_startTime = QTime::fromString(starttimeString, "h:m");
            QString start = m_startTime.toString("hh:mm");
            QString timeString = QString::number(info.end_hour) + ":" + QString::number(info.end_minute); // 确保这是有效的格式
            QTime m_endTime = QTime::fromString(timeString, "h:m");
            QString end = m_endTime.toString("hh:mm");

            QString  lineOne = QString(" %1%2-%3%4 ").arg(time1).arg(start).arg(time2).arg(end)
                               + info.timeLong;
            if(info.m_isAllDay)
            {
                lineOne = tr("   allday");
            }

            scheduleWidget->setStartTime(lineOne);
            scheduleWidget->adjustDate(this->date);
            scheduleWidget->setEndTime(info.end_hour, info.end_minute, this->date, time2);
            scheduleWidget->setDescription(info.m_descript);

            scheduleWidget->setId(info.m_markId);
            connect(scheduleWidget, &schedule_item::ScheduleClick, this, &LunarCalendarWidget::slotEditSchedule);
            connect(scheduleWidget, &schedule_item::EditSchedule, this, &LunarCalendarWidget::slotEditSchedule);
            //        connect(scheduleWidget,&schedule_item::updateSchedule1,this,&LunarCalendarWidget::updateFromItem);
            connect(scheduleWidget, &schedule_item::updateWindow, this, &LunarCalendarWidget::updateFromItem);
            item_layout->addWidget(scheduleWidget);
            item_layout->addSpacing(2);
        }
        m_scheduleWidget->setLayout(item_layout);
        scrollArea->setWidget(m_scheduleWidget);
        m_horLayoutWidgetTop = new QHBoxLayout(this);
        m_horLayoutWidgetTop->setContentsMargins(26, 0, 24, 0);
        m_verLayoutCalendar->setMargin(0);
        m_verLayoutCalendar->setSpacing(0);

        m_horLayoutWidgetTop->addWidget(widgetTime);
        m_horLayoutWidgetTop->addWidget(m_createScheduleButton);
        m_verLayoutCalendar->addLayout(m_horLayoutWidgetTop);
        m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
        m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
        m_verLayoutCalendar->addWidget(m_widgetTop);
        m_verLayoutCalendar->addSpacing(18);
        m_verLayoutCalendar->addWidget(widgetWeek);
        m_verLayoutCalendar->addWidget(m_widgetDayBody, 1);
        m_verLayoutCalendar->addWidget(widgetYearBody, 1);
        m_verLayoutCalendar->addWidget(widgetmonthBody, 1);
        m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
        m_verLayoutCalendar->addWidget(lineDown);
        controlDownLine();
        m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
        lineSchedule->setFixedHeight(0.1);
        m_verLayoutCalendar->addWidget(lineSchedule);
        m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
        m_verLayoutCalendar->addWidget(scrollArea);
    }

}
bool LunarCalendarWidget::isRTLLanguage()
{
    // 获取当前系统的语言代码
    QLocale locale = QLocale::system();
    QString languageCode = locale.name();
    QString languageName;
    int separatorIndex = languageCode.indexOf('_');
    if (separatorIndex == -1)
    {
        separatorIndex = languageCode.indexOf('-');
    }
    // 如果找到了分隔符，就剥离出语言代码部分
    if (separatorIndex != -1)
    {
        languageName =  languageCode.left(separatorIndex);
    }
    QStringList rtlLanguages = {"ar", "fa", "he", "ps", "ur", "kk", "ky", "ug"};
    // 检查当前语言代码是否在RTL语言列表中
    return rtlLanguages.contains(languageName);
}
