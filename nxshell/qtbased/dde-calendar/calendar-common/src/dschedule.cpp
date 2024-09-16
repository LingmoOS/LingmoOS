// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dschedule.h"
#include "commondef.h"

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
