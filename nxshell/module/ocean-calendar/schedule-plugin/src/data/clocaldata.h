// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CLOCALDATA_H
#define CLOCALDATA_H

#include "dschedule.h"
#include "timedatastruct.h"

#include <QSharedPointer>

class CLocalData
{
public:
    typedef QSharedPointer<CLocalData> Ptr;
    CLocalData();
    DSchedule::List scheduleInfoVector() const;
    void setScheduleInfoVector(const DSchedule::List &scheduleInfoVector);
    DSchedule::Ptr SelectInfo() const;
    void setSelectInfo(const DSchedule::Ptr &SelectInfo);
    void setToTitleName(const QString &title);
    QString getToTitleName() const;
    /**
     * @brief setToTime     设置修改的日程时间
     * @param timeInfo      日程时间
     */
    void setToTime(const SemanticsDateTime &timeInfo);
    /**
     * @brief getToTime     获取修改的日程时间
     * @return
     */
    SemanticsDateTime getToTime() const;
    int getOffet() const;
    void setOffset(int offset);
    void setNewInfo(const DSchedule::Ptr &newInfo);
    DSchedule::Ptr getNewInfo() const;

private:
    DSchedule::List m_scheduleInfoVector {};
    DSchedule::Ptr m_SelectInfo {};
    DSchedule::Ptr m_NewInfo {};
    QString m_ToTitleName {""};
    SemanticsDateTime m_ToTime {};
    int m_offset {-1};
};

#endif // CLOCALDATA_H
