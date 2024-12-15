// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "selectandquerystate.h"
#include "../globaldef.h"
#include "commondef.h"
#include "../task/schedulebasetask.h"
#include "../data/changejsondata.h"
#include <qloggingcategory.h>

SelectAndQueryState::SelectAndQueryState(scheduleBaseTask *task)
    : scheduleState(task)
{
}

Reply SelectAndQueryState::getReplyByIntent(bool isOK)
{
    Q_UNUSED(isOK)
    Reply reply;
    REPLY_ONLY_TTS(reply, G_ERR_TTS, G_ERR_TTS, true)
    return reply;
}

scheduleState::Filter_Flag SelectAndQueryState::eventFilter(const JsonData *jsonData)
{
    //如果语义包含全部关键字则为修改初始状态
    if (jsonData->getPropertyStatus() == JsonData::ALL
        //如果语义包含下一个关键字则为修改初始状态
        || jsonData->getPropertyStatus() == JsonData::NEXT
        || jsonData->isVaild()
        //如果语义包含时间则为修改初始状态
        || jsonData->getDateTime().suggestDatetime.size() > 0
        // 如果语义包含内容则为修改初始状态
        || !jsonData->TitleName().isEmpty()
        //如果语义包含重复类型则为修改初始状态
        || jsonData->getRepeatStatus() != JsonData::NONE) {
        return Filter_Flag::Fileter_Init;
    }
    if (jsonData->getPropertyStatus() == JsonData::LAST)
        return Fileter_Normal;
    JsonData *queryData = const_cast<JsonData *>(jsonData);
    changejsondata *mchangeJsonData = dynamic_cast<changejsondata *>(queryData);
    if (mchangeJsonData->fromDateTime().suggestDatetime.size() > 0) {
        return Filter_Flag::Fileter_Init;
    }
    //根据列表编号判断
    if (m_localData->getOffet() < 0 && jsonData->offset() < 0) {
        return Fileter_Err;
    }
    bool showOpenWidget = m_localData->scheduleInfoVector().size() > ITEM_SHOW_NUM;
    const int showcount = showOpenWidget ? ITEM_SHOW_NUM : m_localData->scheduleInfoVector().size();
    if (jsonData->offset() > showcount) {
        return Fileter_Err;
    }
    return Fileter_Normal;
}

Reply SelectAndQueryState::ErrEvent()
{
    Reply reply;
    REPLY_ONLY_TTS(reply, QUERY_ERR_TTS, QUERY_ERR_TTS, true)
    return reply;
}

Reply SelectAndQueryState::normalEvent(const JsonData *jsonData)
{
    bool showOpenWidget = m_localData->scheduleInfoVector().size() > ITEM_SHOW_NUM;
    const int showcount = showOpenWidget ? ITEM_SHOW_NUM : m_localData->scheduleInfoVector().size();
    int offset = 0;
    //获取第N个日程
    if (jsonData->getPropertyStatus() == JsonData::LAST) {
        offset = showcount;
    } else {
        offset = jsonData->offset();
    }
    if (offset > 0) {
        m_localData->setOffset(offset);
        m_localData->setSelectInfo(m_localData->scheduleInfoVector().at(offset - 1));
        DSchedule::Ptr info = m_localData->SelectInfo();
        //如果语义为“第xx个修改到xxx”，添加对修改信息的获取
        //类型转换
        JsonData *queryData = const_cast<JsonData *>(jsonData);
        changejsondata *mchangeJsonData = dynamic_cast<changejsondata *>(queryData);
        //如果有修改时间的信息则赋值
        if (mchangeJsonData->toDateTime().suggestDatetime.size() > 0) {
            m_localData->setToTime(mchangeJsonData->toDateTime());
        }
        //如果有修改内容的信息则获取
        if (!mchangeJsonData->toPlaceStr().isEmpty()) {
            m_localData->setToTitleName(mchangeJsonData->toPlaceStr());
        }
        return m_Task->getReplyBySelectSchedule(m_localData->SelectInfo());
    } else {
        qCWarning(CommonLogger) << "offset <=0";
        Reply reply;
        REPLY_ONLY_TTS(reply, G_ERR_TTS, G_ERR_TTS, false);
        return reply;
    }
}
