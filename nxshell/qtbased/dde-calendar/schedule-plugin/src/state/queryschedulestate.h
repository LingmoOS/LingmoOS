// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef QUERYSCHEDULESTATE_H
#define QUERYSCHEDULESTATE_H

#include "schedulestate.h"
#include "accountitem.h"

class queryScheduleState : public scheduleState
{
public:
    explicit queryScheduleState(scheduleBaseTask *task);
    Reply getReplyByIntent(bool isOK) override;

protected:
    Filter_Flag eventFilter(const JsonData *jsonData) override;
    Reply ErrEvent() override;
    Reply normalEvent(const JsonData *jsonData) override;

protected:
};

#endif // QUERYSCHEDULESTATE_H
