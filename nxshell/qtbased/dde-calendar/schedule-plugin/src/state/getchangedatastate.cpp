// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "getchangedatastate.h"

#include "../globaldef.h"
#include "../data/changejsondata.h"
#include "../task/changescheduletask.h"

getChangeDataState::getChangeDataState(scheduleBaseTask *task)
    : scheduleState(task)
{
}

Reply getChangeDataState::getReplyByIntent(bool isOK)
{
    Q_UNUSED(isOK);
    return ErrEvent();
}

scheduleState::Filter_Flag getChangeDataState::eventFilter(const JsonData *jsonData)
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
        return Fileter_Err;
    if (jsonData->offset() > 0) {
        return Fileter_Err;
    }
    //类型转换
    JsonData *queryData = const_cast<JsonData *>(jsonData);
    changejsondata *mchangeJsonData = dynamic_cast<changejsondata *>(queryData);
    //如果存在form信息则表示一个新的修改
    if (mchangeJsonData->fromDateTime().suggestDatetime.size() > 0) {
        return Fileter_Init;
    }
    //如果存在修改的信息则为正常状态
    if (mchangeJsonData->toDateTime().suggestDatetime.size() > 0
        || !mchangeJsonData->toPlaceStr().isEmpty()) {
        return Fileter_Normal;
    } else {
        return Fileter_Err;
    }
}

Reply getChangeDataState::ErrEvent()
{
    Reply reply;
    //返回错误输入的回复语
    REPLY_ONLY_TTS(reply, G_ERR_TTS, G_ERR_TTS, true)
    return reply;
}

Reply getChangeDataState::normalEvent(const JsonData *jsonData)
{
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
}
