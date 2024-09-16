// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "compatibledata.h"
#include "units.h"
#include "icalformat.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QByteArray>
#include <QList>
#include <QDateTime>

const QMap<int, QString> mTypeMap {{1, "107c369e-b13a-4d45-9ff3-de4eb3c0475b"}, {2, "24cf3ae3-541d-487f-83df-f068416b56b6"}, {3, "403bf009-2005-4679-9c76-e73d9f83a8b4"}};

/**
 * @brief  JobToObject 将Job转换成QJsonObject
 * @param job Job结构体
 * @return QJsonObject
 */
QJsonObject JobToObject(const DSchedule::Ptr &job)
{
    QJsonObject obj;

    obj.insert("ID", job->compatibleID());
    obj.insert("Type", DDE_Calendar::getOldTypeIDByNewTypeID(job->scheduleTypeID()));
    obj.insert("Title", job->summary());
    obj.insert("Description", job->description());
    obj.insert("AllDay", job->allDay());
    obj.insert("Start", dtToString(job->dtStart()));
    obj.insert("End", dtToString(job->dtEnd()));
    obj.insert("RRule", job->recurrence()->defaultRRule()->rrule());
    obj.insert("Remind", DDE_Calendar::getOldRemindByAlarm(job->getAlarmType()));

    //将QString类型转换为QJsonArray类型，方便前端解析
    QJsonArray _ignoreJsonArray;
    for (int i = 0; i < job->recurrence()->exDateTimes().count(); i++) {
        _ignoreJsonArray.append(dtToString(job->recurrence()->exDateTimes().at(i)));
    }
    obj.insert("Ignore", _ignoreJsonArray);
    //TODO:
    obj.insert("RecurID", 0);

    return obj;
}

QString DDE_Calendar::getNewTypeIDByOldTypeID(int oldTypeID)
{
    if (mTypeMap.contains(oldTypeID)) {
        return mTypeMap.value(oldTypeID);
    } else {
        return mTypeMap.value(3);
    }
}

int DDE_Calendar::getOldTypeIDByNewTypeID(const QString &newTypeID)
{
    int oldTypeID = 0;
    QMap<int, QString>::const_iterator constIter = mTypeMap.constBegin();

    for (; constIter != mTypeMap.constEnd(); ++constIter) {
        if (constIter.value() == newTypeID) {
            oldTypeID = constIter.key();
            break;
        }
    }
    return oldTypeID == 0 ? 3 : oldTypeID;
}

QString DDE_Calendar::getExternalSchedule(const DSchedule::Map &scheduleMap)
{
    QString strJson;
    QJsonDocument doc;
    QJsonArray jsonarr;
    DSchedule::Map::const_iterator constIter = scheduleMap.constBegin();
    for (; constIter != scheduleMap.constEnd(); ++constIter) {
        QJsonObject obj;
        QJsonArray jobsJsonArr;
        QJsonObject objjob;
        obj.insert("Date", constIter.key().toString("yyyy-MM-dd"));
        foreach (DSchedule::Ptr schedule, constIter.value()) {
            objjob = JobToObject(schedule);
            jobsJsonArr.append(objjob);
        }
        obj.insert("Jobs", jobsJsonArr);
        jsonarr.append(obj);
    }
    doc.setArray(jsonarr);
    strJson = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
    return strJson;
}

void DDE_Calendar::setAlarmByOldRemind(const DSchedule::Ptr &schedule, const QString &remind)
{
    //提醒规则
    QStringList strList = remind.split(";", QString::SkipEmptyParts);

    int remindNum = strList.at(0).toInt();
    //小于0表示不提醒
    if (remindNum >= 0) {
        KCalendarCore::Alarm::Ptr alarm = KCalendarCore::Alarm::Ptr(new KCalendarCore::Alarm(schedule.data()));
        alarm->setEnabled(true);
        alarm->setType(KCalendarCore::Alarm::Display);
        alarm->setDisplayAlarm(schedule->summary());

        if (schedule->allDay()) {
            //提前多少秒
            int offset = 0;
            if (strList.size() > 1) {
                QTime time = QTime::fromString(strList.at(1), "hh:mm");
                offset = time.hour() * 60 * 60 + time.second() * 60;
            }
            KCalendarCore::Duration duration(-(24 * 60 * 60 * remindNum - offset));
            alarm->setStartOffset(duration);
        } else {
            KCalendarCore::Duration duration(-(60 * remindNum));
            alarm->setStartOffset(duration);
        }
        schedule->addAlarm(alarm);
    }
}

QString DDE_Calendar::getOldRemindByAlarm(const DSchedule::AlarmType &alarmType)
{
    QString _resultStr {""};
    switch (alarmType) {
    case DSchedule::Alarm_Begin:
        _resultStr = "0";
        break;
    case DSchedule::Alarm_15Min_Front:
        _resultStr = "15";
        break;
    case DSchedule::Alarm_30Min_Front:
        _resultStr = "30";
        break;
    case DSchedule::Alarm_1Hour_Front:
        _resultStr = "60";
        break;
    case DSchedule::Alarm_1Day_Front:
        _resultStr = "1440";
        break;
    case DSchedule::Alarm_2Day_Front:
        _resultStr = "2880";
        break;
    case DSchedule::Alarm_1Week_Front:
        _resultStr = "10080";
        break;
    case DSchedule::Alarm_9Hour_After:
        //日程当天9点(全天)
        _resultStr = "0;09:00";
        break;
    case DSchedule::Alarm_15Hour_Front:
        //一天前（全天），为日程开始前15小时
        _resultStr = "1;09:00";
        break;
    case DSchedule::Alarm_39Hour_Front:
        //2天前（全天），为日程开始前39小时
        _resultStr = "2;09:00";
        break;
    case DSchedule::Alarm_159Hour_Front:
        //一周前(全天)，为日程开始前159小时
        _resultStr = "7;09:00";
        break;
    default:
        break;
    }
    return _resultStr;
}

DSchedule::Ptr DDE_Calendar::getScheduleByExported(const QString &scheduleStr)
{
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(scheduleStr.toLocal8Bit(), &json_error));

    if (json_error.error != QJsonParseError::NoError) {
        return nullptr;
    }
    QJsonObject rootObj = jsonDoc.object();
    DSchedule::Ptr schedule(new DSchedule);

    //日程是否为全天
    if (rootObj.contains("AllDay")) {
        schedule->setAllDay(rootObj.value("AllDay").toBool());
    }
    //日程提醒规则
    if (rootObj.contains("Remind")) {
        DDE_Calendar::setAlarmByOldRemind(schedule, rootObj.value("Remind").toString());
    }
    //日程标题
    if (rootObj.contains("Title")) {
        schedule->setSummary(rootObj.value("Title").toString());
    }
    //日程描述
    if (rootObj.contains("Description")) {
        schedule->setDescription(rootObj.value("Description").toString());
    }
    //日程类型
    if (rootObj.contains("Type")) {
        schedule->setScheduleTypeID(DDE_Calendar::getNewTypeIDByOldTypeID(rootObj.value("Type").toInt()));
    }
    //日程开始时间
    if (rootObj.contains("Start")) {
        schedule->setDtStart(dtFromString(rootObj.value("Start").toString()));
    }
    //日程结束时间
    if (rootObj.contains("End")) {
        schedule->setDtEnd(dtFromString(rootObj.value("End").toString()));
    }
    //    //日程重复ID
    //    if (rootObj.contains("RecurID")) {
    //        _resultSchedule.setRecurID(rootObj.value("RecurID").toInt());
    //    }
    //日程重复规则
    if (rootObj.contains("RRule")) {
        DDE_Calendar::setRRuleByOldRRule(schedule, rootObj.value("RRule").toString());
    }
    //重复日程忽略日期集
    if (rootObj.contains("Ignore")) {
        DDE_Calendar::setExDate(schedule, rootObj.value("Ignore").toArray());
    }
    return schedule;
}

void DDE_Calendar::setRRuleByOldRRule(const DSchedule::Ptr &schedule, const QString &rrule)
{
    //重复规则
    KCalendarCore::Recurrence *recurrence = schedule->recurrence();
    KCalendarCore::ICalFormat ical;
    KCalendarCore::RecurrenceRule *rule = new KCalendarCore::RecurrenceRule;

    if (ical.fromString(rule, rrule)) {
        recurrence->addRRule(rule);
    }
}

void DDE_Calendar::setExDate(const DSchedule::Ptr &schedule, const QJsonArray &ignore)
{
    KCalendarCore::Recurrence *recurrence = schedule->recurrence();

    foreach (auto ignoreTime, ignore) {
        if (schedule->allDay()) {
            recurrence->addExDate(dtFromString(ignoreTime.toString()).date());
        } else {
            recurrence->addExDateTime(dtFromString(ignoreTime.toString()));
        }
    }
}
