// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SELECTANDQUERYSTATE_H
#define SELECTANDQUERYSTATE_H

#include "schedulestate.h"

class SelectAndQueryState : public scheduleState
{
public:
    explicit SelectAndQueryState(scheduleBaseTask *task);
    Reply getReplyByIntent(bool isOK) override;

protected:
    Filter_Flag eventFilter(const JsonData *jsonData) override;
    Reply ErrEvent() override;
    Reply normalEvent(const JsonData *jsonData) override;
};

#endif // SELECTANDQUERYSTATE_H
