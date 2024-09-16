// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAYWINDOW_H
#define DAYWINDOW_H

#include "dschedule.h"
#include "huangliData/dbusdatastruct.h"
#include "cschedulebasewidget.h"

#include <DMainWindow>
#include <DFrame>

#include <QWidget>
#include <QDate>
#include <QLabel>
DWIDGET_USE_NAMESPACE

class CDayMonthView;
class CScheduleView;
class CScheduleSearchView;
class QHBoxLayout;
class CustomFrame;
class CDayWindow: public CScheduleBaseWidget
{
    Q_OBJECT
public:
    explicit CDayWindow(QWidget *parent = nullptr);
    ~CDayWindow() override;
    void setSearchWFlag(bool flag);
    void setLunarVisible(bool state);
    void setTheMe(int type = 0);
    //设置显示的时间位置
    void setTime(const QTime time = QTime());
    //更新全天和非全天高度
    void updateHeight();
    //设置当前时间
    void setCurrentDateTime(const QDateTime &currentDate) override;
    //设置选择时间年信息显示
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
private:
    void initUI();
    void initConnection();
    //获取界面显示定位时间位置
    void setMakeTime(QMap<QDate, DSchedule::List> &info);
public slots:
    void slotScheduleHide();
    //更新选择时间
    void slotChangeSelectDate(const QDate &date);

private slots:
    void slotIsDragging(bool &isDragging);
    //切换选择时间
    void slotSwitchPrePage();
    //切换选择时间
    void slotSwitchNextPage();
signals:
    //拖拽结束信号
    void signalNewSlot();
private:
    CDayMonthView *m_daymonthView = nullptr;
    CustomFrame *m_leftground = nullptr;
    DVerticalLine *m_verline = nullptr;
    QLabel *m_YearLabel = nullptr;
    QLabel *m_LunarLabel = nullptr;
    QLabel *m_SolarDay = nullptr;
    CScheduleView *m_scheduleView = nullptr;
    QString m_searchText;
    bool m_searchFlag = false;
    QHBoxLayout *m_mainLayout = nullptr;
    QTime           m_makeTime{};   //界面显示定位时间位置
};

#endif // YEARWINDOW_H
