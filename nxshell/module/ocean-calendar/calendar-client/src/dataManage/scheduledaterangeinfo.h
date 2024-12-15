// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SCHEDULEDATERANGEINFO_H
#define SCHEDULEDATERANGEINFO_H
#include "dschedule.h"

typedef struct _tagMScheduleDateRangeInfo {
    QDate bdate;
    QDate edate;
    bool state;
    int  num;
    DSchedule::Ptr tData;

    bool operator <(const _tagMScheduleDateRangeInfo &info)const
    {
        if (bdate == info.bdate) {
            if (bdate.daysTo(edate) == info.bdate.daysTo(info.edate)) {
                return tData < info.tData;
            } else {
                return bdate.daysTo(edate) > info.bdate.daysTo(info.edate);
            }
        } else {
            return bdate < info.bdate;
        }
    }

    bool operator ==(const _tagMScheduleDateRangeInfo &info)const
    {
        return bdate == info.bdate &&
               edate == info.edate &&
               tData == info.tData &&
               state == info.state &&
               num == info.num;
    }
} MScheduleDateRangeInfo;
#endif // SCHEDULEDATERANGEINFO_H
