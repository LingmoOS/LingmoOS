// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSCHEDULEQUERYPAR_H
#define DSCHEDULEQUERYPAR_H

#include <QDateTime>
#include <QString>
#include <QSharedPointer>

//查询日程相关参数
class DScheduleQueryPar
{
public:
    typedef QSharedPointer<DScheduleQueryPar> Ptr;

    enum QueryType {
        Query_None,
        Query_RRule, //查询重复规则
        Query_Top, //查询前多少个
        Query_ScheduleID //查询日程ID
    };

    enum RRuleType {
        RRule_None, //从不
        RRule_Day, //每天
        RRule_Work, //每工作日
        RRule_Week, //每周
        RRule_Month, //每月
        RRule_Year, //每年
    };

    DScheduleQueryPar();

    QDateTime dtStart() const;
    void setDtStart(const QDateTime &dtStart);

    QDateTime dtEnd() const;
    void setDtEnd(const QDateTime &dtEnd);

    QString key() const;
    void setKey(const QString &key);

    static DScheduleQueryPar::Ptr fromJsonString(const QString &queryStr);
    static QString toJsonString(const DScheduleQueryPar::Ptr &queryPar);

    QueryType queryType() const;
    void setQueryType(const QueryType &queryType);

    int queryTop() const;
    void setQueryTop(int queryTop);

    RRuleType rruleType() const;
    void setRruleType(const RRuleType &rruleType);

private:
    QString m_key; //查询关键字，如果查询类型为日程ID，则表示日程ID
    int m_queryTop; //查询范围内前多少个日程
    RRuleType m_rruleType; //查询对应重复规则的日程
    QueryType m_queryType; //查询的类型
    QDateTime m_dtStart; //查询的起始时间
    QDateTime m_dtEnd; //查询的截止时间
};

#endif // DSCHEDULEQUERYPAR_H
