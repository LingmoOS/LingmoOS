// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SCHEDULEMANAGER_H
#define SCHEDULEMANAGER_H

#include "accountmanager.h"
#include "lunarmanager.h"
#include <QObject>

class ScheduleManager : public QObject
{
    Q_OBJECT
public:
    explicit ScheduleManager(QObject *parent = nullptr);

    static ScheduleManager* getInstace();

    //获取一年的所有日程
    QMap<QDate, DSchedule::List> getAllScheduleMap();
    //根据起始时间和结束时间获取日程
    QMap<QDate, DSchedule::List> getScheduleMap(const QDate &startDate, const QDate &stopDate) const;
    //获取所有的搜索到的日程（按日期分类）
    QMap<QDate, DSchedule::List> getAllSearchedScheduleMap();
    //获取所有搜索到的日程
    DSchedule::List getAllSearchedScheduleList();

    //获取所有有日程的时间
    QSet<QDate> getAllScheduleDate();
    //获取所有有被搜索日程的时间
    QSet<QDate> getAllSearchedScheduleDate();
    //获取某一天的日程
    DSchedule::List getScheduleByDay(QDate);

    //根据日程id获取日程类型
    DScheduleType::Ptr getAccountByScheduleId(const QString& id);
    DScheduleType::Ptr getScheduleTypeByScheduleId(const QString& id);

    //搜索日程
    void searchSchedule(const QString &key, const QDateTime &startTime, const QDateTime &endTime);
    //清空搜索的日程
    void clearSearchSchedule();

    //重置一年的日程信息
    void resetSchedule(int year);
    void resetSchedule(const QDateTime& start, const QDateTime& end);

signals:
    //日程数据更新信号
    void signalScheduleUpdate();
    //被搜索日程数据更新信号
    void signalSearchScheduleUpdate();

public slots:
    //日程数据更新事件
    void slotScheduleUpdate();
    void slotSearchUpdate();

private:
    void initconnect();

    //更新日程
    void updateSchedule();
    //更新被搜索的日程
    void updateSearchSchedule();

private:
    QMap<QDate, DSchedule::List> m_scheduleMap;     //一年的日程数据
    QMap<QDate, DSchedule::List> m_searchScheduleMap;   //被搜索的日程数据
    DScheduleQueryPar::Ptr m_searchQuery;   //上一次搜索的条件


    LunarManager m_lunarManager;
};
#define gScheduleManager ScheduleManager::getInstace()
#endif // SCHEDULEMANAGER_H
