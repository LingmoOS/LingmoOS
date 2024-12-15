// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MONTHVIEW_H
#define MONTHVIEW_H

#include "monthweekview.h"
#include "scheduleRemindWidget.h"
#include "monthgraphiview.h"
#include "dschedule.h"

#include <DWidget>

#include <QDate>
#include <QAction>

DWIDGET_USE_NAMESPACE

class CMonthView: public DWidget
{
    Q_OBJECT
public:
    //根据系统主题类型设置颜色
    void setTheMe(int type = 0);
    /**
     * @brief isDragging 是否可以拖拽
     * @return 是否可以拖拽
     */
    bool isDragging()const
    {
        return m_monthGraphicsView->getDragStatus() != 4;
    }
public:
    explicit CMonthView(QWidget *parent = nullptr);
    ~CMonthView() override;
    //设置每周首日
    void setFirstWeekday(Qt::DayOfWeek weekday);
    //设置显示日期
    void setShowDate(const QVector<QDate> &showDate);
    //设置黄历信息
    void setHuangLiInfo(const QMap<QDate, CaHuangLiDayInfo> &huangLiInfo);
    //设置班休信息
    void setFestival(const QMap<QDate, int> &festivalInfo);
    //设置显示日程
    void setScheduleInfo(const QMap<QDate, DSchedule::List> &scheduleInfo);
    //设置搜索日程
    void setSearchScheduleInfo(const DSchedule::List &searchScheduleInfo);
    //设置当前时间
    void setCurrentDate(const QDate &currentDate);
    void setRemindWidgetTimeFormat(QString timeformat);
    void deleteSelectSchedule();
signals:
    /**
     * @brief signalsViewSelectDate 切换视图
     * @param date 日期
     */
    void signalsViewSelectDate(QDate date);
public slots:
    /**
     * @brief setLunarVisible 设置是否显示阴历信息
     * @param visible 是否显示阴历信息
     */
    void setLunarVisible(bool visible);
    /**
     * @brief setSelectSchedule 设置选择的日程
     * @param scheduleInfo 日程信息
     */
    void setSelectSchedule(const DSchedule::Ptr &scheduleInfo);
public slots:
    /**
     * @brief slotScheduleRemindWidget 日程浮框
     * @param isShow 是否显示日程浮框
     * @param out 日程信息
     */
    void slotScheduleRemindWidget(const bool isShow, const DSchedule::Ptr &out = DSchedule::Ptr());
signals:
    /**
     * @brief signalAngleDelta      发送滚动信号滚动相对量
     * @param delta     滚动相对量
     */
    void signalAngleDelta(int delta);
    void signalSwitchPrePage();
    void signalSwitchNextPage();

protected:
    /**
     * @brief resizeEvent 窗口大小调整
     * @param event 窗口大小调整事件
     */
    void resizeEvent(QResizeEvent *event) override;
    /**
     * @brief mousePressEvent 鼠标单击
     * @param event 鼠标事件
     */
    void mousePressEvent(QMouseEvent *event) override;

    bool event(QEvent *event) override;

private:
    DSchedule::Ptr getScheduleInfo(const QDate &beginDate, const QDate &endDate);

private:
    CMonthGraphicsview *m_monthGraphicsView = nullptr;
    QVector<QDate>                  m_showDate;
    QDate                           m_currentDate;
    QDate                           m_createDate;
    CMonthWeekView                  *m_weekIndicator = nullptr;
    int                             m_firstWeekDay = 0;
    QAction                         *m_createAction = nullptr; // 创建日程
    QVBoxLayout                     *m_mainLayout = nullptr;
    bool                            m_sflag = true;
    ScheduleRemindWidget *m_remindWidget = nullptr;

    QPoint                  m_PressPoint;
    bool                    isCreate;
    QDate                   m_PressDate;
    QDate                   m_MoveDate;
};

#endif // MYCALENDARWIDGET_H
