// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef DSYSTEMTIME_P_H
#define DSYSTEMTIME_P_H

#include "dsystemtime.h"

DSYSTEMTIME_BEGIN_NAMESPACE
class TimeDate1Interface;
class TimeSync1Interface;

class DSystemTimePrivate : public QObject
{
    Q_OBJECT
public:
    explicit DSystemTimePrivate(DSystemTime *parent)
        : q_ptr(parent)
    {
    }

    virtual ~DSystemTimePrivate() { }

public:
    TimeDate1Interface *m_timedate_inter;
    TimeSync1Interface *m_timesync_inter;
    DSystemTime *q_ptr;
    Q_DECLARE_PUBLIC(DSystemTime)
};

DSYSTEMTIME_END_NAMESPACE

#endif
