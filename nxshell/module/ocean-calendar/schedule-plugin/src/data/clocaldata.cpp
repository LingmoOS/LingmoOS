// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "clocaldata.h"

CLocalData::CLocalData()
{
}

DSchedule::List CLocalData::scheduleInfoVector() const
{
    return m_scheduleInfoVector;
}

void CLocalData::setScheduleInfoVector(const DSchedule::List &scheduleInfoVector)
{
    m_scheduleInfoVector = scheduleInfoVector;
}

DSchedule::Ptr CLocalData::SelectInfo() const
{
    return m_SelectInfo;
}

void CLocalData::setSelectInfo(const DSchedule::Ptr &SelectInfo)
{
    m_SelectInfo = SelectInfo;
}

void CLocalData::setToTitleName(const QString &title)
{
    m_ToTitleName = title;
}

QString CLocalData::getToTitleName() const
{
    return m_ToTitleName;
}

void CLocalData::setToTime(const SemanticsDateTime &timeInfo)
{
    m_ToTime = timeInfo;
}

SemanticsDateTime CLocalData::getToTime() const
{
    return m_ToTime;
}

int CLocalData::getOffet() const
{
    return m_offset;
}

void CLocalData::setOffset(int offset)
{
    m_offset = offset;
}

void CLocalData::setNewInfo(const DSchedule::Ptr &newInfo)
{
    m_NewInfo = newInfo;
}

DSchedule::Ptr CLocalData::getNewInfo() const
{
    return m_NewInfo;
}
