// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WEEKWINDOW_H
#define WEEKWINDOW_H

#include "customframe.h"
#include "dschedule.h"
#include "cschedulebasewidget.h"

#include <DFrame>

#include <QWidget>
#include <QDate>
#include <QLabel>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

class CWeekHeadView;
class CaLunarDayInfo;
class CWeekView;
class CScheduleView;
class CScheduleSearchView;
class CTodayButton;
class CWeekWindow: public CScheduleBaseWidget
{
    Q_OBJECT
public:
    explicit CWeekWindow(QWidget *parent = nullptr);
    ~CWeekWindow() override;
    //设置是否显示阴历信息
    void setLunarVisible(bool state);
    //初始化界面设置
    void initUI();
    //初始化信号和槽的连接
    void initConnection();
    //根据系统主题类型设置颜色
    void setTheMe(int type = 0);
    //设置CScheduleView的时间
    void setTime(QTime time);
    //设置搜索标志
    void setSearchWFlag(bool flag);
    //更新全天区域高度
    void updateHeight();
    //设置年显示和今天按钮显示
    void setYearData() override;
    //更新显示时间
    void updateShowDate(const bool isUpdateBar = true) override;
    //更新日程显示
    void updateShowSchedule() override;
    //更新显示农历信息
    void updateShowLunar() override;
    //更新界面搜索日程显示
    void updateSearchScheduleInfo() override;
    //设置选中搜索日程
    void setSelectSearchScheduleInfo(const DSchedule::Ptr &info) override;
    //删除选中日程
    void deleteselectSchedule() override;
signals:
    /**
     * @brief signalCurrentDate 设置时间信号
     * @param date 时间
     */
    void signalCurrentDate(QDate date);
    /**
     * @brief signalsViewSelectDate 选择时间的信号
     * @param date 时间
     */
    void signalsViewSelectDate(QDate date);
public slots:
    //判断是否可以拖拽
    void slotIsDragging(bool &isDragging);
    //切换日视图并设置选择时间
    void slotViewSelectDate(const QDate &date);
    void slotSwitchPrePage();
    void slotSwitchNextPage();
public slots:
    //隐藏日程浮框
    void slotScheduleHide();
private slots:
    //切换上一周
    void slotprev();
    //切换下一周
    void slotnext();
    //返回到当前时间
    void slottoday();
    //修改选择时间
    void slotSelectDate(const QDate &date);
    //接受处理滚动相对量
    void slotAngleDelta(int delta);
private:
    //切换选择时间
    void switchDate(const QDate &date);
protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
private:
    CWeekHeadView *m_weekHeadView = nullptr; //周视图头
    CTodayButton *m_today = nullptr;
    QDate              m_currentdate;
    QLabel *m_YearLabel = nullptr;
    QLabel *m_YearLunarLabel = nullptr;
    QLabel *m_weekLabel = nullptr;
    //显示周数
    CWeekView         *m_weekview = nullptr;
    CustomFrame *m_todayframe = nullptr;
    CScheduleView *m_scheduleView = nullptr;
    QVBoxLayout *m_mainHLayout = nullptr;
    QString m_searchText;
    QHBoxLayout *m_tMainLayout = nullptr;
    bool m_searchFlag = false;
    bool m_isSwitchStatus = false;
    QDate       m_startDate;
    QDate       m_stopDate;
};

#endif // YEARWINDOW_H
