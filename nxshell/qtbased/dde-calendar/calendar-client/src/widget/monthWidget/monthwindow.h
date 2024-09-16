// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MONTHWINDOW_H
#define MONTHWINDOW_H

#include "cschedulebasewidget.h"

#include <DPushButton>
#include <DFrame>
#include <DMainWindow>

#include <QWidget>
#include <QDate>
#include <QLabel>
#include <QSpacerItem>

DWIDGET_USE_NAMESPACE
class CScheduleSearchView;
class CMonthView;
class CMonthDayView;
class QHBoxLayout;
class CTodayButton;
class CMonthWindow: public CScheduleBaseWidget
{
    Q_OBJECT
public:
    explicit CMonthWindow(QWidget *parent = nullptr);
    ~CMonthWindow() override;
    //设置是否显示阴历信息
    void setLunarVisible(bool state);
    //根据系统主题类型设置颜色
    void setTheMe(int type = 0);
    //设置年份显示
    void setYearData() override;
    //更新显示时间
    void updateShowDate(const bool isUpdateBar = true) override;
    //设置当前时间
    void setCurrentDateTime(const QDateTime &currentDate) override;
    //更新日程显示
    void updateShowSchedule() override;
    //更新显示农历信息
    void updateShowLunar() override;
    //更新界面搜索日程显示
    void updateSearchScheduleInfo() override;
    //设置选中搜索日程
    void setSelectSearchScheduleInfo(const DSchedule::Ptr &info) override;
    //设置是否显示搜索界面
    void setSearchWFlag(bool flag);
    //删除选中日程
    void deleteselectSchedule() override;
public slots:
    //选择上一个月份
    void previousMonth();
    //选择下一个月份
    void nextMonth();
private:
    //初始化界面
    void initUI();
    //初始化信号和槽的连接
    void initConnection();
    //切换月份，并更新信息
    void slideMonth(bool next);
signals:
    /**
     * @brief signalsCurrentScheduleDate
     * @param date
     */
    void signalsCurrentScheduleDate(QDate date);

public slots:
    //隐藏日程浮框
    void slotScheduleHide();
    //接受滚动事件滚动相对量
    void slotAngleDelta(int delta);
    //设置选择时间切换日视图
    void slotViewSelectDate(const QDate &date);
    void slotSwitchPrePage();
    void slotSwitchNextPage();

protected:
    void resizeEvent(QResizeEvent *event) override;
private slots:
    //返回当前时间
    void slottoday();
    //设置选择的月份
    void slotSetSelectDate(const QDate &date);
private:
    CMonthView              *m_monthView = nullptr;
    CMonthDayView           *m_monthDayView = nullptr;
    CTodayButton            *m_today = nullptr;
    QDate                   m_startDate;
    QDate                   m_stopDate;
    QLabel                  *m_YearLabel = nullptr;
    QLabel                  *m_YearLunarLabel = nullptr;
    QSpacerItem             *m_spaceitem = nullptr;
    DWidget                 *m_gridWidget = nullptr;
    bool m_searchFlag = false;
    bool m_isSwitchStatus = false;
    QHBoxLayout *m_tMainLayout = nullptr;
};

#endif // YEARWINDOW_H
