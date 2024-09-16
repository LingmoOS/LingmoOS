// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef REPEATFEEDBACKSTATE_H
#define REPEATFEEDBACKSTATE_H
#include "schedulestate.h"

class repeatfeedbackstate : public scheduleState
{
public:
    explicit repeatfeedbackstate(scheduleBaseTask *task);
    Reply getReplyByIntent(bool isOK) override;

protected:
    Filter_Flag eventFilter(const JsonData *jsonData) override;
    Reply ErrEvent() override;
    Reply normalEvent(const JsonData *jsonData) override;
};

#endif // REPEATFEEDBACKSTATE_H
