// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CSCHEDULEBASEWIDGET_H
#define CSCHEDULEBASEWIDGET_H

#include "calendarmanage.h"
#include "cdialogiconbutton.h"
#include "schedulemanager.h"
#include "lunarmanager.h"

#include <QWidget>

class CScheduleBaseWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CScheduleBaseWidget(QWidget *parent = nullptr);
    ~CScheduleBaseWidget();
    //设置选择时间
    static bool setSelectDate(const QDate &selectDate, const bool isSwitchYear = false, const QWidget *widget = nullptr);
    bool setSelectDate(const QDate &selectDate, const QWidget *widget);
    //获取选择时间
    static QDate getSelectDate();
    //设置当前时间
    virtual void setCurrentDateTime(const QDateTime &currentDate);
    //获取当前时间
    QDateTime getCurrendDateTime() const;
    //获取是否显示农历信息
    static bool getShowLunar();
    //更新所有数据，显示时间和日程数据
    void updateData();
    //更新dbus数据
    static void updateDBusData();
    //更新界面搜索日程显示
    virtual void updateSearchScheduleInfo();
    //设置年显示
    virtual void setYearData() = 0;
    //更新显示时间
    virtual void updateShowDate(const bool isUpdateBar = true) = 0;
    //更新显示日程数据
    virtual void updateShowSchedule() = 0;
    //更新农历信息显示
    virtual void updateShowLunar() = 0;
    //设置选中搜索日程
    virtual void setSelectSearchScheduleInfo(const DSchedule::Ptr &info) = 0;
    //删除选中日程
    virtual void deleteselectSchedule();
protected:
    //获取选择时间的农历信息
    CaHuangLiDayInfo getLunarInfo();
signals:
    //切换视图信号    0:跳转上一个视图  1：月视图  2：周视图 3:日视图
    void signalSwitchView(const int viewIndex = 0);

public slots:
    void slotScheduleUpdate();
    void slotSearchedScheduleUpdate();

private:
    void initConnect();

protected:
    static CalendarManager *m_calendarManager;
    QString             m_lunarYear;
    QString             m_lunarDay;

    CDialogIconButton *m_dialogIconButton = nullptr;    //时间跳转控件
};

#endif // CSCHEDULEBASEWIDGET_H
