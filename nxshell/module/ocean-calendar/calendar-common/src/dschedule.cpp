// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dschedule.h"
#include "commondef.h"
#include "lunarandfestival/lunardateinfo.h"

#include "icalformat.h"
#include "memorycalendar.h"
#include "units.h"
#include "commondef.h"

#include <QtDBus/QtDBus>

#define Duration_Min 60
#define Duration_Hour 60 * 60
#define Duration_Day 24 * 60 * 60
#define Duration_Week 7 * 24 * 60 * 60

DSchedule::DSchedule()
    : KCalendarCore::Event()
    , m_fileName("")
    , m_scheduleTypeID("")
    , m_compatibleID(0)
{
}

DSchedule::DSchedule(const DSchedule &schedule)
    : KCalendarCore::Event(schedule)
    , m_fileName("")
    , m_scheduleTypeID("")
    , m_compatibleID(0)
{
    this->setScheduleTypeID(schedule.scheduleTypeID());
}

DSchedule::DSchedule(const KCalendarCore::Event &event)
    : KCalendarCore::Event(event)
    , m_fileName("")
    , m_scheduleTypeID("")
    , m_compatibleID(0)
{
}

DSchedule *DSchedule::clone() const
{
    return new DSchedule(*this);
}

QString DSchedule::scheduleTypeID() const
{
    return m_scheduleTypeID;
}

void DSchedule::setScheduleTypeID(const QString &typeID)
{
    m_scheduleTypeID = typeID;
}

bool DSchedule::isMoved()
{
    return this->m_moved;
}

void DSchedule::setMoved(bool moved)
{
    this->m_moved = moved;
}

bool DSchedule::isValid() const
{
    //TODO:添加判断日程是否有效
    return true;
}

bool DSchedule::isMultiDay() const
{
    //如果日期不一致则为跨天日程
    return dtStart().date() != dtEnd().date();
}

bool DSchedule::operator==(const DSchedule &schedule) const
{
    return this->instanceIdentifier() == schedule.instanceIdentifier();
}

bool operator<(const DSchedule::Ptr &s1, const DSchedule::Ptr &s2)
{
    if (s1.isNull() || s2.isNull()) {
        return false;
    }
    return *s1.data() < *s2.data();
}

bool DSchedule::operator<(const DSchedule &schedule) const
{
    if (this->allDay() != schedule.allDay()) {
        return this->allDay() > schedule.allDay();
    }
    if (this->dtStart() != schedule.dtStart()) {
        return this->dtStart() < schedule.dtStart();
    }
    if (this->created() != schedule.created()) {
        return this->created() < schedule.created();
    }
    if (this->summary() != schedule.summary()) {
        return this->summary() < schedule.summary();
    }
    return true;
}

void DSchedule::setAlarmType(const DSchedule::AlarmType &alarmType)
{
    //如果提醒规则没有变化则退出
    if (alarmType == getAlarmType()) {
        return;
    }

    //清除提醒规则
    this->clearAlarms();
    //如果为从不则退出
    if (alarmType == AlarmType::Alarm_None || alarmType == AlarmType::Alarm_AllDay_None)
        return;

    QMap<int, AlarmType> alarmMap = getAlarmMap();
    QMap<int, AlarmType>::const_iterator iter = alarmMap.constBegin();
    for (; iter != alarmMap.constEnd(); ++iter) {
        if (iter.value() == alarmType) {
            KCalendarCore::Alarm::Ptr alarm = KCalendarCore::Alarm::Ptr(new KCalendarCore::Alarm(this));
            alarm->setEnabled(true);
            alarm->setType(KCalendarCore::Alarm::Display);
            alarm->setDisplayAlarm(this->summary());
            KCalendarCore::Duration duration(iter.key());
            alarm->setStartOffset(duration);
            addAlarm(alarm);
            break;
        }
    }
}

DSchedule::AlarmType DSchedule::getAlarmType()
{
    AlarmType alarmType = allDay() ? Alarm_AllDay_None : Alarm_None;
    KCalendarCore::Alarm::List alarmList = this->alarms();
    if (alarmList.size() > 0) {
        KCalendarCore::Duration duration = alarmList.at(0)->startOffset();
        QMap<int, AlarmType> alarmMap = getAlarmMap();
        if (alarmMap.contains(duration.asSeconds())) {
            alarmType = alarmMap[duration.asSeconds()];
        }
    }
    return alarmType;
}

void DSchedule::setRRuleType(const DSchedule::RRuleType &rtype)
{
    if (getRRuleType() == rtype)
        return;
    clearRecurrence();

    QString rules;
    switch (rtype) {
    case RRule_Year:
        rules = "FREQ=YEARLY";
        break;
    case RRule_Month:
        rules = "FREQ=MONTHLY";
        break;
    case RRule_Week:
        rules = "FREQ=WEEKLY";
        break;
    case RRule_Work:
        rules = "FREQ=DAILY;BYDAY=MO,TU,WE,TH,FR";
        break;
    case RRule_Day:
        rules = "FREQ=DAILY";
        break;
    default:
        rules = "";
        break;
    }
    if (!rules.isEmpty()) {
        KCalendarCore::Recurrence *recurrence = this->recurrence();
        KCalendarCore::RecurrenceRule *rrule = new KCalendarCore::RecurrenceRule();

        KCalendarCore::ICalFormat ical;
        if (ical.fromString(rrule, rules)) {
            recurrence->addRRule(rrule);
        }
    }
}

DSchedule::RRuleType DSchedule::getRRuleType()
{
    RRuleType rtype = RRule_None;
    if (this->recurs()) {
        KCalendarCore::RecurrenceRule *rrule = this->recurrence()->defaultRRuleConst();
        switch (rrule->recurrenceType()) {
        case KCalendarCore::RecurrenceRule::rYearly: {
            rtype = RRule_Year;
        } break;
        case KCalendarCore::RecurrenceRule::rMonthly: {
            rtype = RRule_Month;
        } break;
        case KCalendarCore::RecurrenceRule::rWeekly: {
            rtype = RRule_Week;
        } break;
        case KCalendarCore::RecurrenceRule::rDaily: {
            if (rrule->rrule().contains("BYDAY=MO,TU,WE,TH,FR")) {
                rtype = RRule_Work;
            } else {
                rtype = RRule_Day;
            }
        } break;
        default:
            rtype = RRule_None;
            break;
        }
    }
    return rtype;
}

int DSchedule::numberOfRepetitions(const Ptr &scheudle, const QDateTime &datetime)
{
    return scheudle->recurrence()->durationTo(datetime);
}

bool DSchedule::fromJsonString(DSchedule::Ptr &schedule, const QString &json)
{
    if (schedule.isNull()) {
        schedule = DSchedule::Ptr(new DSchedule);
    }
    QJsonParseError jsonError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(json.toLocal8Bit(), &jsonError));
    if (jsonError.error != QJsonParseError::NoError) {
        qCWarning(CommonLogger) << "error:" << jsonError.errorString();
        return false;
    }
    bool resBool = false;
    QJsonObject rootObj = jsonDoc.object();
    if (rootObj.contains("schedule")) {
        QString str = rootObj.value("schedule").toString();
        if (fromIcsString(schedule, str)) {
            if (rootObj.contains("type")) {
                schedule->setScheduleTypeID(rootObj.value("type").toString());
            }
            if (rootObj.contains("compatibleID")) {
                schedule->setcompatibleID(rootObj.value("compatibleID").toInt());
            }
            resBool = true;
        }
    }
    return resBool;
}

bool DSchedule::toJsonString(const DSchedule::Ptr &schedule, QString &json)
{
    if (schedule.isNull()) {
        qCWarning(CommonLogger) << "hold a reference to a null pointer.";
        return false;
    }
    QJsonObject rootObject;
    rootObject.insert("type", schedule->scheduleTypeID());
    rootObject.insert("schedule", toIcsString(schedule));
    rootObject.insert("compatibleID", schedule->compatibleID());
    QJsonDocument jsonDoc;
    jsonDoc.setObject(rootObject);
    json = QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Compact));
    return true;
}

bool DSchedule::fromIcsString(Ptr &schedule, const QString &string)
{
    bool resBool = false;
    KCalendarCore::ICalFormat icalformat;
    QTimeZone timezone = QDateTime::currentDateTime().timeZone();
    KCalendarCore::MemoryCalendar::Ptr _cal(new KCalendarCore::MemoryCalendar(timezone));
    if (icalformat.fromString(_cal, string)) {
        KCalendarCore::Event::List eventList = _cal->events();
        if (eventList.size() > 0) {
            schedule = DSchedule::Ptr(new DSchedule(*eventList.at(0).data())); // eventList.at(0).staticCast<DSchedule>();
            resBool = true;
        }
    }
    return resBool;
}

QString DSchedule::toIcsString(const DSchedule::Ptr &schedule)
{
    KCalendarCore::ICalFormat icalformat;
    KCalendarCore::MemoryCalendar::Ptr _cal(new KCalendarCore::MemoryCalendar(nullptr));
    _cal->addEvent(schedule);
    return icalformat.toString(_cal.staticCast<KCalendarCore::Calendar>());
}

QMap<QDate, DSchedule::List> DSchedule::fromMapString(const QString &json)
{
    QMap<QDate, DSchedule::List> scheduleMap;
    QJsonParseError jsonError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(json.toLocal8Bit(), &jsonError));
    if (jsonError.error != QJsonParseError::NoError) {
        qCWarning(CommonLogger) << "error:" << jsonError.errorString();
        return scheduleMap;
    }
    QJsonArray rootArray = jsonDoc.array();
    QDate date;
    foreach (auto jsonValue, rootArray) {
        QJsonObject jsonObj = jsonValue.toObject();
        if (jsonObj.contains("Date")) {
            date = dateFromString(jsonObj.value("Date").toString());
        }
        if (jsonObj.contains("schedule")) {
            QJsonArray jsonArray = jsonObj.value("schedule").toArray();
            foreach (auto scheduleValue, jsonArray) {
                QString scheduleStr = scheduleValue.toString();
                DSchedule::Ptr schedule = DSchedule::Ptr(new DSchedule);
                DSchedule::fromJsonString(schedule, scheduleStr);
                scheduleMap[date].append(schedule);
            }
        }
    }
    return scheduleMap;
}

QString DSchedule::toMapString(const QMap<QDate, DSchedule::List> &scheduleMap)
{
    QJsonArray rootArray;
    QMap<QDate, DSchedule::List>::const_iterator iter = scheduleMap.constBegin();
    for (; iter != scheduleMap.constEnd(); ++iter) {
        QJsonObject jsonObj;
        jsonObj.insert("Date", dateToString(iter.key()));
        QJsonArray jsonArray;
        foreach (auto &schedule, iter.value()) {
            QString scheduleStr;
            DSchedule::toJsonString(schedule, scheduleStr);
            jsonArray.append(scheduleStr);
        }
        jsonObj.insert("schedule", jsonArray);
        rootArray.append(jsonObj);
    }
    QJsonDocument jsonDoc;
    jsonDoc.setArray(rootArray);
    return QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Compact));
}

QPair<QString, DSchedule::List> DSchedule::fromListString(const QString &json)
{
    QPair<QString, DSchedule::List> schedulePair;
    QJsonParseError jsonError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(json.toLocal8Bit(), &jsonError));
    if (jsonError.error != QJsonParseError::NoError) {
        qCWarning(CommonLogger) << "error:" << jsonError.errorString();
        return schedulePair;
    }

    QJsonObject jsonObj = jsonDoc.object();
    DSchedule::List scheduleList;
    if (jsonObj.contains("query")) {
        schedulePair.first = jsonObj.value("query").toString();
    }
    if (jsonObj.contains("schedules")) {
        QJsonArray jsonArray = jsonObj.value("schedules").toArray();
        foreach (auto scheduleValue, jsonArray) {
            QString scheduleStr = scheduleValue.toString();
            DSchedule::Ptr schedule = DSchedule::Ptr(new DSchedule);
            DSchedule::fromJsonString(schedule, scheduleStr);
            scheduleList.append(schedule);
        }
    }
    schedulePair.second = scheduleList;

    return schedulePair;
}

QString DSchedule::toListString(const QString &query, const DSchedule::List &scheduleList)
{
    QJsonObject jsonObj;
    jsonObj.insert("query", query);
    QJsonArray jsonArray;
    foreach (auto &schedule, scheduleList) {
        QString scheduleStr;
        DSchedule::toJsonString(schedule, scheduleStr);
        jsonArray.append(scheduleStr);
    }
    jsonObj.insert("schedules", jsonArray);


    QJsonDocument jsonDoc;
    jsonDoc.setObject(jsonObj);
    return QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Compact));
}

void DSchedule::expendRecurrence(DSchedule::Map &scheduleMap, const DSchedule::Ptr &schedule, const QDateTime &dtStart, const QDateTime &dtEnd)
{
    QDateTime queryDtStart = dtStart;
    //如果日程为全天日程，则查询的开始时间设置为0点，因为全天日程的开始和结束时间都是0点
    if(schedule->allDay()){
        queryDtStart.setTime(QTime(0,0,0));
    }
    if (schedule->recurs()) {
        //获取日程的开始结束时间差
        qint64 interval = schedule->dtStart().secsTo(schedule->dtEnd());
        QList<QDateTime> dtList = schedule->recurrence()->timesInInterval(queryDtStart, dtEnd);
        foreach (auto &dt, dtList) {
            QDateTime scheduleDtEnd = dt.addSecs(interval);
            DSchedule::Ptr newSchedule = DSchedule::Ptr(schedule->clone());
            newSchedule->setDtStart(dt);
            newSchedule->setDtEnd(scheduleDtEnd);

            //只有重复日程设置RecurrenceId
            if (schedule->dtStart() != dt) {
                newSchedule->setRecurrenceId(dt);
            }
            scheduleMap[dt.date()].append(newSchedule);
        }
    } else {
        if (!(schedule->dtStart() > dtEnd || schedule->dtEnd() < queryDtStart)) {
            scheduleMap[schedule->dtStart().date()].append(schedule);
        }
    }
}

QMap<QDate, DSchedule::List> DSchedule::convertSchedules(const DScheduleQueryPar::Ptr &queryPar, const DSchedule::List &scheduleList)
{
    QDateTime dtStart = queryPar->dtStart();
    QDateTime dtEnd = queryPar->dtEnd();
    bool extend = queryPar->queryType() == DScheduleQueryPar::Query_None;

    QMap<QDate, DSchedule::List> scheduleMap;
    foreach (auto &schedule, scheduleList) {
        //获取日程的开始结束时间差
        qint64 interval = schedule->dtStart().secsTo(schedule->dtEnd());
        //如果存在重复日程
        if (schedule->recurs()) {
            //如果为农历日程
            if (schedule->lunnar()) {
                //农历重复日程计算
                LunarDateInfo lunardate(schedule->recurrence()->defaultRRuleConst(), interval);

                QMap<int, QDate> ruleStartDate = lunardate.getRRuleStartDate(dtStart.date(), dtEnd.date(), schedule->dtStart().date());

                QDateTime recurDateTime;
                recurDateTime.setTime(schedule->dtStart().time());
                QDateTime copyEnd;
                QMap<int, QDate>::ConstIterator iter = ruleStartDate.constBegin();
                for (; iter != ruleStartDate.constEnd(); iter++) {
                    recurDateTime.setDate(iter.value());
                    //如果在忽略时间列表中,则忽略
                    if (schedule->recurrence()->exDateTimes().contains(recurDateTime))
                        continue;
                    copyEnd = recurDateTime.addSecs(interval);
                    DSchedule::Ptr newSchedule = DSchedule::Ptr(new DSchedule(*schedule.data()));
                    newSchedule->setDtStart(recurDateTime);
                    newSchedule->setDtEnd(copyEnd);
                    //只有重复日程设置RecurrenceId
                    if (schedule->dtStart() != recurDateTime) {
                        newSchedule->setRecurrenceId(recurDateTime);
                    }
                    scheduleMap[recurDateTime.date()].append(newSchedule);
                }
            } else {
                //非农历日程
                expendRecurrence(scheduleMap, schedule, dtStart, dtEnd);
            }
        } else {
            //普通日程
            //如果在查询时间范围内
            QDateTime queryDtStart = dtStart;
            //如果日程为全天日程，则查询的开始时间设置为0点，因为全天日程的开始和结束时间都是0点
            if(schedule->allDay()){
                queryDtStart.setTime(QTime(0,0,0));
            }
            if (!(schedule->dtEnd() < queryDtStart || schedule->dtStart() > dtEnd)) {
                if (extend && schedule->isMultiDay()) {
                    //需要扩展的天数
                    int extenddays = static_cast<int>(schedule->dtStart().daysTo(schedule->dtEnd()));
                    for (int i = 0; i <= extenddays; ++i) {
                        //如果扩展的日期在查询范围内则添加，否则退出
                        if(scheduleMap.contains(schedule->dtStart().date().addDays(i))){
                            scheduleMap[schedule->dtStart().date().addDays(i)].append(schedule);
                        } else {
                            break;
                        }
                    }
                } else {
                    scheduleMap[schedule->dtStart().date()].append(schedule);
                }
            }
        }
    }

    //如果为查询前N个日程，则取前N个日程
    if (queryPar->queryType() == DScheduleQueryPar::Query_Top) {
        int scheduleNum = 0;
        DSchedule::Map filterSchedule;
        DSchedule::Map::const_iterator iter = scheduleMap.constBegin();
        for (; iter != scheduleMap.constEnd(); ++iter) {
            if (iter.value().size() == 0) {
                continue;
            }
            if (scheduleNum + iter.value().size() > queryPar->queryTop()) {
                DSchedule::List scheduleList;
                int residuesNum = queryPar->queryTop() - scheduleNum;
                for (int i = 0; i < residuesNum; ++i) {
                    scheduleList.append(iter.value().at(i));
                }
                filterSchedule[iter.key()] = scheduleList;
            } else {
                filterSchedule[iter.key()] = iter.value();
            }
        }
        scheduleMap = filterSchedule;
    }

    return scheduleMap;
}

QMap<QDate, DSchedule::List> DSchedule::fromQueryResult(const QString &query)
{
    QMap<QDate, DSchedule::List> scheduleMap;
    QPair<QString, DSchedule::List> pair = fromListString(query);
    DScheduleQueryPar::Ptr queryPar = DScheduleQueryPar::fromJsonString(pair.first);
    if (queryPar.isNull()) {
        return scheduleMap;
    }

    scheduleMap = DSchedule::convertSchedules(queryPar, pair.second);
    return scheduleMap;
}

bool operator==(const DSchedule::Ptr &s1, const DSchedule::Ptr &s2)
{
    return s1.isNull() || s2.isNull() ? s1.isNull() && s2.isNull() : s1->instanceIdentifier() == s2->instanceIdentifier();
}

QMap<int, DSchedule::AlarmType> DSchedule::getAlarmMap()
{
    static QMap<int, DSchedule::AlarmType> alarmMap {
        {0, Alarm_Begin},
        {-15 * Duration_Min, Alarm_15Min_Front},
        {-30 * Duration_Min, Alarm_30Min_Front},
        {-Duration_Hour, Alarm_1Hour_Front},
        {-Duration_Day, Alarm_1Day_Front},
        {-Duration_Day * 2, Alarm_2Day_Front},
        {-Duration_Week, Alarm_1Week_Front},
        {9 * Duration_Hour, Alarm_9Hour_After},
        {-15 * Duration_Hour, Alarm_15Hour_Front},
        {-39 * Duration_Hour, Alarm_39Hour_Front},
        {-159 * Duration_Hour, Alarm_159Hour_Front}};
    return alarmMap;
}

QString DSchedule::fileName() const
{
    return m_fileName;
}

void DSchedule::setFileName(const QString &fileName)
{
    m_fileName = fileName;
}

int DSchedule::compatibleID() const
{
    return m_compatibleID;
}

void DSchedule::setcompatibleID(int compatibleID)
{
    m_compatibleID = compatibleID;
}

QDebug operator<<(QDebug debug, const DSchedule &scheduleJsonData)
{
    QDebugStateSaver saver(debug);
    debug.noquote() << "dtStart:" << dtToString(scheduleJsonData.dtStart())
                    << " ,dtEnd:" << dtToString(scheduleJsonData.dtEnd())
                    << " ,dtCreate:" << dtToString(scheduleJsonData.created())
                    << " ,summary:" << scheduleJsonData.summary()
                    << " ,scheduleTypeID:" << scheduleJsonData.scheduleTypeID()
                    << ",Uid:" << scheduleJsonData.uid()
                    << " ,rrule:" << scheduleJsonData.recurrence()->defaultRRule()->rrule();
    return debug;
}
