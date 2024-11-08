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

#include "lingmocalendarwidget.h"
#include "lingmosdk/applications/windowmanager/windowmanager.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include "lunarcalendarwidget/calendardatabase.h"
#include "../lunarcalendarwidget/lunarcalendarwidget.h"
#define WEBVIEW_WIDTH (454)
#define WEBVIEW_MAX_HEIGHT (704)
#define WEBVIEW_MIN_HEIGHT (600)
#define POPUP_BORDER_SPACING 4
#include "../lunarcalendarwidget/morelabel.h"
#define CALENDAR_BUTTON_WIDTH  300

#define HOUR_SYSTEM_CONTROL "org.lingmo.control-center.panel.plugins"
#define DATA_FORMAT     "date"          //日期格式：yyyy/MM/dd、yyyy-MM-dd
#define TIME_FORMAT     "hoursystem"    //时间格式：12小时制、24小时制
#define WEEK_FORMAT     "firstday"      //一周第一天：星期一、星期日
#define CALENDAR_FORMAT "calendar"      //日历格式：公历-solarlunar、农历-lunar

#define PANEL_CONFIG "/usr/share/lingmo/lingmo-panel/panel-commission.ini"

const int WIDGET_GAP = 8;

LINGMOCalendarWidget::LINGMOCalendarWidget(ILINGMOPanelPlugin *plugin, QWidget *parent) :
    m_parent(parent),
    m_plugin(plugin)
{
    //加载翻译
    translator();
    QTranslator *translator1 = new QTranslator();
    if(translator1->load("/usr/share/qt5/translations/qt_"+QLocale::system().name())){
        qApp->installTranslator(translator1);
    }

    //实例化日历的窗口
    m_frmLunarWidget = new frmLunarCalendarWidget();  //新版日历
    m_widget = new LunarCalendarWidget();
    connect(m_widget,&LunarCalendarWidget::yijiChangeUp,m_frmLunarWidget,&frmLunarCalendarWidget::changeUpSize);
    connect(m_widget,&LunarCalendarWidget::Up,m_frmLunarWidget,&frmLunarCalendarWidget::changeUp);
//    connect(m_widget,&LunarCalendarWidget::newschdule,m_frmLunarWidget,&frmLunarCalendarWidget::changeUpSizeFromSchedule);
    //connect(label,&ShowMoreLabel::clicked,m_frmLunarWidget,&frmLunarCalendarWidget::changeUpSizeFromSchedule);
    connect(m_widget,&LunarCalendarWidget::yijiChangeDown,m_frmLunarWidget,&frmLunarCalendarWidget::changeDownSize);
    connect(m_widget,&LunarCalendarWidget::almanacChanged,m_frmLunarWidget,&frmLunarCalendarWidget::showAlmanac);
    connect(m_frmLunarWidget,&frmLunarCalendarWidget::yijiFChangeDown, this, [=] (){ //日历的宜忌按钮取消
        changeWidowpos();
    });
    connect(m_frmLunarWidget,&frmLunarCalendarWidget::yijiFChangeUp, this, [=] (){ //日历的宜忌按钮勾选
        changeWidowpos();
    });
    connect(m_frmLunarWidget,&frmLunarCalendarWidget::Up,this,&LINGMOCalendarWidget::change);

    //实例化任务栏日期按钮
    m_calendarButton = new CalendarButton(plugin, this);
    connect(m_calendarButton,&CalendarButton::pressShowHideCalendar,[=](){   //点击显示新版日历主窗口
        if(!m_widget->isHidden()) {
            m_widget->hide();
        } else {
            changeWidowpos(true);
        }
    });

    //按钮布局
    m_layout=new QHBoxLayout(this);
    m_layout->addWidget(m_calendarButton);
    m_layout->setAlignment(Qt::AlignCenter);
    m_layout->setContentsMargins(0, 0, 0, 0);
}

LINGMOCalendarWidget::~LINGMOCalendarWidget()
{
    delete m_widget;
    delete m_frmLunarWidget;
    delete m_calendarButton ;
}

void LINGMOCalendarWidget::realign()
{
    if (m_plugin->panel()->isHorizontal()) {
        this->setMaximumSize(CALENDAR_BUTTON_WIDTH,m_plugin->panel()->panelSize()-5);
    } else {
        this->setMaximumSize(m_plugin->panel()->panelSize()-5,CALENDAR_BUTTON_WIDTH);
    }
}

void LINGMOCalendarWidget::translator()
{
    QTranslator *translator = new QTranslator(this);
    translator->load(QLocale(), "calendar", "_", CALENDAR_TRANSLATION_DIR);
    QCoreApplication::installTranslator(translator);
}

/* 显示新版日历窗口 */
void LINGMOCalendarWidget::changeWidowpos(bool restore)
{
    int totalHeight = qApp->screenAt(QCursor::pos())->size().height() + qApp->screenAt(QCursor::pos())->geometry().y();//屏幕高度
    int totalWidth = qApp->screenAt(QCursor::pos())->size().width() + qApp->screenAt(QCursor::pos())->geometry().x();
    qDebug()<<"changeWidowpos里面的total高度"<<totalHeight;
    QRect rect;
    switch (m_plugin->panel()->position()) {
    case ILINGMOPanel::PositionBottom:
        rect.setRect(totalWidth-m_frmLunarWidget->width() - WIDGET_GAP,
                     totalHeight-m_plugin->panel()->panelSize()-m_frmLunarWidget->height()-WIDGET_GAP+100,
                     m_frmLunarWidget->width(),
                     m_frmLunarWidget->height()-100);
        qDebug()<<"-----m_frmLunarWidget->height()"<<m_frmLunarWidget->height();
        break;
    case ILINGMOPanel::PositionTop:
        rect.setRect(totalWidth-m_frmLunarWidget->width()-WIDGET_GAP,
                     qApp->screenAt(QCursor::pos())->geometry().y()+m_plugin->panel()->panelSize()+WIDGET_GAP,
                     m_frmLunarWidget->width(),
                     m_frmLunarWidget->height());
        break;
    case ILINGMOPanel::PositionLeft:
        rect.setRect(qApp->screenAt(QCursor::pos())->geometry().x()+m_plugin->panel()->panelSize()+WIDGET_GAP,
                     totalHeight-m_frmLunarWidget->height()-WIDGET_GAP,
                     m_frmLunarWidget->width(),
                     m_frmLunarWidget->height());
        break;
    case ILINGMOPanel::PositionRight:
        rect.setRect(totalWidth-m_plugin->panel()->panelSize()-m_frmLunarWidget->width()-WIDGET_GAP,
                     totalHeight-m_frmLunarWidget->height()-WIDGET_GAP,
                     m_frmLunarWidget->width(),
                     m_frmLunarWidget->height());
        break;
    default:
        rect.setRect(qApp->screenAt(QCursor::pos())->geometry().x()+m_plugin->panel()->panelSize()+WIDGET_GAP,
                     totalHeight-m_frmLunarWidget->height(),
                     m_frmLunarWidget->width(),
                     m_frmLunarWidget->height());
        break;
    }
    m_widget->setFixedSize(rect.width(),rect.height());
    qDebug()<<"大小是"<<rect.width()<<rect.height();
    if(restore) {
//        Q_EMIT(m_frmLunarWidget->onShowToday());
        m_widget->showCalendar(true);
    } else {
        m_widget->showCalendar(false);
    }


    kdk::WindowManager::setGeometry(m_widget->windowHandle(),rect);
}
/* 显示新版日历窗口 */
void LINGMOCalendarWidget::change(int n)
{
    int totalHeight = qApp->screenAt(QCursor::pos())->size().height() + qApp->screenAt(QCursor::pos())->geometry().y();
    int totalWidth = qApp->screenAt(QCursor::pos())->size().width() + qApp->screenAt(QCursor::pos())->geometry().x();
    QRect rect;
    switch (m_plugin->panel()->position()) {
    case ILINGMOPanel::PositionBottom:
        rect.setRect(totalWidth-m_frmLunarWidget->width() - WIDGET_GAP,
                     totalHeight-m_plugin->panel()->panelSize()-m_frmLunarWidget->height()-WIDGET_GAP,
                     m_frmLunarWidget->width(),
                     m_frmLunarWidget->height());
        break;
    case ILINGMOPanel::PositionTop:
        rect.setRect(totalWidth-m_frmLunarWidget->width()-WIDGET_GAP,
                     qApp->screenAt(QCursor::pos())->geometry().y()+m_plugin->panel()->panelSize()+WIDGET_GAP,
                     m_frmLunarWidget->width(),
                     m_frmLunarWidget->height());
        break;
    case ILINGMOPanel::PositionLeft:
        rect.setRect(qApp->screenAt(QCursor::pos())->geometry().x()+m_plugin->panel()->panelSize()+WIDGET_GAP,
                     totalHeight-m_frmLunarWidget->height()-WIDGET_GAP,
                     m_frmLunarWidget->width(),
                     m_frmLunarWidget->height());
        break;
    case ILINGMOPanel::PositionRight:
        rect.setRect(totalWidth-m_plugin->panel()->panelSize()-m_frmLunarWidget->width()-WIDGET_GAP,
                     totalHeight-m_frmLunarWidget->height()-WIDGET_GAP,
                     m_frmLunarWidget->width(),
                     m_frmLunarWidget->height());
        break;
    default:
        rect.setRect(qApp->screenAt(QCursor::pos())->geometry().x()+m_plugin->panel()->panelSize()+WIDGET_GAP,
                     totalHeight-m_frmLunarWidget->height(),
                     m_frmLunarWidget->width(),
                     m_frmLunarWidget->height());
        break;
    }
    m_widget->setFixedSize(rect.width(),rect.height()+40*n);
    qDebug()<<"大小是"<<rect.width()<<rect.height();
//        Q_EMIT(m_frmLunarWidget->onShowToday());
        m_widget->showCalendar(true);



    kdk::WindowManager::setGeometry(m_widget->windowHandle(),rect);
}
