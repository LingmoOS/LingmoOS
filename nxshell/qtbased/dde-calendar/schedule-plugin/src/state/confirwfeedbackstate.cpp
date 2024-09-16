// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "confirwfeedbackstate.h"

#include "../globaldef.h"
#include "../task/schedulebasetask.h"
#include "../data/clocaldata.h"
#include "../data/changejsondata.h"

confirwFeedbackState::confirwFeedbackState(scheduleBaseTask *task)
    : scheduleState(task)
{
}

Reply confirwFeedbackState::getReplyByIntent(bool isOK)
{
    return m_Task->confirmInfo(isOK);
}

scheduleState::Filter_Flag confirwFeedbackState::eventFilter(const JsonData *jsonData)
{
    if (jsonData->getPropertyStatus() == JsonData::NEXT) {
        return Fileter_Init;
    }
    if (jsonData->getPropertyStatus() == JsonData::LAST
        || jsonData->offset() > 0) {
        return Fileter_Err;
    }
    Filter_Flag result = changeDateErrJudge(jsonData, Fileter_Init);
    return result;
}

Reply confirwFeedbackState::ErrEvent()
{
    Reply reply;
    REPLY_ONLY_TTS(reply, G_ERR_TTS, G_ERR_TTS, true)
    return reply;
}

Reply confirwFeedbackState::normalEvent(const JsonData *jsonData)
{
    Q_UNUSED(jsonData);
    return m_Task->confirwScheduleHandle(m_localData->SelectInfo());
}
