// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SELECTINQUIRYSTATE_H
#define SELECTINQUIRYSTATE_H

#include "schedulestate.h"
#include "dschedule.h"

class selectInquiryState : public scheduleState
{
public:
    explicit selectInquiryState(scheduleBaseTask *task);
    Reply getReplyByIntent(bool isOK) override;

protected:
    Filter_Flag eventFilter(const JsonData *jsonData) override;
    Reply ErrEvent() override;
    Reply normalEvent(const JsonData *jsonData) override;

private:
};

#endif // SELECTINQUIRYSTATE_H
