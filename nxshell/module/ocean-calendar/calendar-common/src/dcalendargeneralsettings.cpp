// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcalendargeneralsettings.h"
#include "commondef.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDebug>

DCalendarGeneralSettings::DCalendarGeneralSettings()
    : m_firstDayOfWeek(Qt::Sunday)
    , m_timeShowType(TwentyFour)
{
}

DCalendarGeneralSettings::DCalendarGeneralSettings(const DCalendarGeneralSettings &setting)
    : m_firstDayOfWeek(setting.firstDayOfWeek())
    , m_timeShowType(setting.timeShowType())
{
}

DCalendarGeneralSettings *DCalendarGeneralSettings::clone() const
{
    return new DCalendarGeneralSettings(*this);
}

Qt::DayOfWeek DCalendarGeneralSettings::firstDayOfWeek() const
{
    return m_firstDayOfWeek;
}

void DCalendarGeneralSettings::setFirstDayOfWeek(const Qt::DayOfWeek &firstDayOfWeek)
{
    m_firstDayOfWeek = firstDayOfWeek;
}

DCalendarGeneralSettings::TimeShowType DCalendarGeneralSettings::timeShowType() const
{
    return m_timeShowType;
}

void DCalendarGeneralSettings::setTimeShowType(const TimeShowType &timeShowType)
{
    m_timeShowType = timeShowType;
}

void DCalendarGeneralSettings::toJsonString(const Ptr &cgSet, QString &jsonStr)
{
    QJsonObject rootObject;
    rootObject.insert("firstDayOfWeek", cgSet->firstDayOfWeek());
    rootObject.insert("TimeShowType", cgSet->timeShowType());
    QJsonDocument jsonDoc;
    jsonDoc.setObject(rootObject);
    jsonStr = QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Compact));
}

bool DCalendarGeneralSettings::fromJsonString(Ptr &cgSet, const QString &jsonStr)
{
    QJsonParseError jsonError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonStr.toLocal8Bit(), &jsonError));
    if (jsonError.error != QJsonParseError::NoError) {
        qCWarning(CommonLogger) << "error:" << jsonError.errorString();
        return false;
    }

    QJsonObject rootObj = jsonDoc.object();
    if (rootObj.contains("firstDayOfWeek")) {
        cgSet->setFirstDayOfWeek(static_cast<Qt::DayOfWeek>(rootObj.value("firstDayOfWeek").toInt()));
    }

    if (rootObj.contains("TimeShowType")) {
        cgSet->setTimeShowType(static_cast<TimeShowType>(rootObj.value("TimeShowType").toInt()));
    }
    return true;
}
