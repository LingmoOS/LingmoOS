// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "createscheduletask.h"

#include "../globaldef.h"
#include "commondef.h"
#include "dscheduledatamanager.h"

createScheduleTask::createScheduleTask()
    : scheduleBaseTask()
{
}

Reply createScheduleTask::SchedulePress(semanticAnalysisTask &semanticTask)
{
    //创建
    CreateJsonData *createJsonData = dynamic_cast<CreateJsonData *>(semanticTask.getJsonData());
    //如果转换失败则返回错误消息
    if (createJsonData == nullptr)
        return errorMessage();
    Reply m_reply;
    if (createJsonData->getPropertyStatus() != JsonData::PRO_NONE
            || createJsonData->offset() > -1) {
        REPLY_ONLY_TTS(m_reply, CREATE_ERR_TTS, CREATE_ERR_TTS, true);
        return m_reply;
    }
    //如果时间无效
    if (createJsonData->getDateTimeInvalid()) {
        REPLY_ONLY_TTS(m_reply, DATETIME_ERR_TTS, DATETIME_ERR_TTS, true);
        return m_reply;
    }

    m_widget = new createSchedulewidget();
    //设置日程时间
    setDateTime(createJsonData);
    if (m_begintime > m_endtime) {
        //日程的开始时间大于结束时间，
        qCritical("error: schedule begindatetime is after the enddatetime!");
    }
    if (shouldEndSession(createJsonData)) {
        if (!isValidDateTime) {
            qCInfo(CommonLogger) << "schedule begintime or endtime is not valided!";
            //无效时间，直接返回回复语
            REPLY_ONLY_TTS(m_reply, replyNotValidDT, replyNotValidDT, true);
            //设置时间有效标志为true
            isValidDateTime = true;
            return m_reply;
        } else if (!beginDateTimeIsinHalfYear()) {
            if (beginDateTimeBeforeCurrent()) {
                qCInfo(CommonLogger) << "schedule begintime is before currenttime!";
                //"我现在有点慌，因为我还不会制定过去的提醒"
                REPLY_ONLY_TTS(m_reply, createJsonData->SuggestMsg(), createJsonData->SuggestMsg(), true);
            } else if (beginDateTimeOutHalfYear()) {
                qCInfo(CommonLogger) << "schedule begintime is after halfyear!";
                //"只能创建未来半年的日程"
                REPLY_ONLY_TTS(m_reply, CREATE_TIME_OUT_TTS, CREATE_TIME_OUT_TTS, true);
            }
            return m_reply;
        } else {
            //设置日程titlename
            setScheduleTitleName(createJsonData);
            //创建日程和插件
            QString scheduleID = createScheduleWithRepeatStatus(createJsonData);
            creareScheduleUI(scheduleID);
            //带有插件的回复语
            REPLY_WIDGET_TTS(m_reply, m_widget, getReply(createJsonData), getReply(createJsonData), true);
        }
    } else {
        //如果需要进行多轮，需要先将此标志设置为true
        isValidDateTime = true;
        //开始date为今天，没有给time(默认为00：00,小于当前time)，需要进行多轮，设置默认回复语
        //只有回复语
        REPLY_ONLY_TTS(m_reply, createJsonData->SuggestMsg(), createJsonData->SuggestMsg(), false);
    }
    return m_reply;
}

void createScheduleTask::setDateTime(CreateJsonData *createJsonData)
{
    //助手返回时间的个数
    int DateTimeSize = createJsonData->getDateTime().suggestDatetime.size();
    switch (DateTimeSize) {
    case 1: {
        //只有一个时间，为日程开始时间，结束时间为开始时间一个小时后
        m_begintime = createJsonData->getDateTime().suggestDatetime.at(0).datetime;
        m_endtime = m_begintime.addSecs(60 * 60);
        if (!validDateTime(m_begintime)) {
            //日程时间是否有效
            QString strDateTime = createJsonData->getDateTime().suggestDatetime.at(0).strDateTime;
            isValidDateTime = false;
            replyNotValidDT = QString(ISVALID_DATE_TIME).arg(strDateTime);
            break;
        }
        if (!createJsonData->getDateTime().suggestDatetime.at(0).hasTime) {
            m_begintime.setTime(QTime::currentTime());
            m_endtime = m_begintime.addSecs(60 * 60);
        }
    } break;
    case 2: {
        //日程是否带有具体时间
        bool firstScheduleHasTime = createJsonData->getDateTime().suggestDatetime.at(0).hasTime;
        bool secondScheduleHasTime = createJsonData->getDateTime().suggestDatetime.at(1).hasTime;
        //有两个时间
        m_begintime = createJsonData->getDateTime().suggestDatetime.at(0).datetime;
        m_endtime = createJsonData->getDateTime().suggestDatetime.at(1).datetime;
        //日程开始时间是否有效
        if (!validDateTime(m_begintime)) {
            QString strDateTime = createJsonData->getDateTime().suggestDatetime.at(0).strDateTime;
            isValidDateTime = false;
            replyNotValidDT = QString(ISVALID_DATE_TIME).arg(strDateTime);
            break;
        }
        //日程结束时间是否有效
        if (!validDateTime(m_endtime)) {
            QString endDateTime = createJsonData->getDateTime().suggestDatetime.at(1).strDateTime;
            isValidDateTime = false;
            replyNotValidDT = QString(ISVALID_DATE_TIME).arg(endDateTime);
            break;
        }
        //开始时间为当天的，如果开始日期小于当天不做处理，
        if (m_begintime.date() == QDateTime::currentDateTime().date()) {
            if (m_begintime.time() > QTime::currentTime()) {
                //跨天日程，如果日程开始时间大于当前时间，则开始时间为当天
                m_begintime.setDate(m_begintime.date().addDays(m_begintime.date().daysTo(QDate::currentDate())));
            } else {
                //跨天日程，如果日程开始时间小于等于当前时间，则开始时间加一天
                m_begintime.setDate(m_begintime.date().addDays(m_begintime.date().daysTo(QDate::currentDate()) + 1));
            }
        }
        //开始时间没有具体时间点，设置为当前时间点
        if (firstScheduleHasTime && !secondScheduleHasTime) {
            m_endtime.setTime(m_begintime.time());
        } else if (!firstScheduleHasTime && secondScheduleHasTime) {
            m_begintime.setTime(m_endtime.time());
        }
    } break;
    default: {
        //其他情况视为错误输入
        isValidDateTime = false;
        replyNotValidDT = "您输入的时间不正确，请重新输入";
    } break;
    }
}

void createScheduleTask::setScheduleTitleName(CreateJsonData *createJsonData)
{
    if (createJsonData->TitleName().isEmpty())
        //如果没有titlename，默认为：新建日程
        m_widget->setTitleName(NEW_SCHEDULE);
    else
        //设置日程titlename
        m_widget->setTitleName(createJsonData->TitleName());
}

QString createScheduleTask::createScheduleWithRepeatStatus(CreateJsonData *createJsonData)
{
    //创建的日程信息
    QString scheduleID;
    //重复日程的时间
    QVector<int> getDayNum = createJsonData->getRepeatNum();
    //根据不同类型分别创建日程
    switch (createJsonData->getRepeatStatus()) {
    case CreateJsonData::NONE: {
        //非重复日程，不能创建过期日程
        if (m_begintime >= QDateTime::currentDateTime() && m_begintime < QDateTime::currentDateTime().addMonths(6)) {
            scheduleID = getNotRepeatDaySchedule();
        }
    }
    break;
    case CreateJsonData::EVED:
        //每天重复日程
        scheduleID = getEveryDaySchedule();
        break;
    case CreateJsonData::EVEW: {
        //每周重复日程
        scheduleID = getEveryWeekSchedule(getDayNum);
    }
    break;
    case CreateJsonData::EVEM: {
        //每月重复日程
        scheduleID = getEveryMonthSchedule(getDayNum);
    }
    break;
    case CreateJsonData::EVEY: {
        //每年重复日程,不能创建过期或者超过半年的日程
        if (m_begintime > QDateTime::currentDateTime() && m_begintime < QDateTime::currentDateTime().addMonths(6))
            scheduleID = getEveryYearSchedule();
    }
    break;
    case CreateJsonData::WORKD:
        //工作日
        scheduleID = getEveryWorkDaySchedule();
        break;
    case CreateJsonData::RESTD: {
        //休息日
        scheduleID = getEveryRestDaySchedule();
    }
    break;
    }
    return scheduleID;
}

void createScheduleTask::creareScheduleUI(const QString &scheduleID)
{
    if (!scheduleID.isEmpty()) {
        m_widget->scheduleEmpty(true);
        //更新界面
        m_widget->updateUI(scheduleID);
    } else {
        qCritical("Creat ScheduleInfo is Empty!");
    }
}

QString createScheduleTask::getReply(CreateJsonData *createJsonData)
{
    QString str_reply;
    //为特殊情况拼接回复语
    if (createJsonData->getRepeatStatus() == CreateJsonData::RESTD
            && createJsonData->getDateTime().suggestDatetime.size() > 0
            && createJsonData->getDateTime().suggestDatetime.at(0).hasTime) {
        //如果为休息日，并且有开始时间，拼接回复语
        str_reply = QString(EVERY_WEEKEND_TTS).arg(m_begintime.toString("hh:mm"));
    } else if (createJsonData->getRepeatStatus() == CreateJsonData::NONE
               && createJsonData->getDateTime().suggestDatetime.size() > 0
               && createJsonData->getDateTime().suggestDatetime.at(0).hasTime
               && createJsonData->getDateTime().suggestDatetime.at(0).datetime < QDateTime::currentDateTime()
               && createJsonData->getDateTime().suggestDatetime.size() == 2
               && createJsonData->ShouldEndSession()) {
        //对于跨天日程，开始datetime小于当前datetime，则开始date增加一天，为其拼接回复语
        str_reply = QString(BEGINDATETIME_LATER_THAN_CURRENTDATETIME_ACROSS_THE_DAY_TTS).arg(m_begintime.toString("hh:mm"));
    } else {
        //没有特殊情况使用默认回复语
        str_reply = createJsonData->SuggestMsg();
    }

    return str_reply;
}

bool createScheduleTask::beginDateTimeIsinHalfYear()
{
    //判断日程开始时间是否在半年内
    if (m_begintime < QDateTime::currentDateTime() || m_begintime > QDateTime::currentDateTime().addMonths(6))
        return false;
    else
        return true;
}

bool createScheduleTask::beginDateTimeOutHalfYear()
{
    //日程开始时间不在半年范围内
    if (m_begintime > QDateTime::currentDateTime().addMonths(6))
        return true;
    else
        return false;
}

bool createScheduleTask::beginDateTimeBeforeCurrent()
{
    //日程开始时间早于当前时间
    if (m_begintime < QDateTime::currentDateTime())
        return true;
    else
        return false;
}

QString createScheduleTask::getNotRepeatDaySchedule()
{
    //设置重复类型
    m_widget->setRpeat(0);
    //创建日程
    DSchedule::Ptr scheduleinfo = setDateTimeAndGetSchedule(m_begintime, m_endtime);
    return DScheduleDataManager::getInstance()->createSchedule(scheduleinfo);
}

QString createScheduleTask::getEveryDaySchedule()
{
    //设置重复类型
    m_widget->setRpeat(1);
    DSchedule::Ptr scheduleinfo = setDateTimeAndGetSchedule(m_begintime, m_endtime);
    //设置完成后，将everyDayState设置为false
    everyDayState = false;
    //创建日程
    return DScheduleDataManager::getInstance()->createSchedule(scheduleinfo);
}

QString createScheduleTask::getEveryWeekSchedule(QVector<int> dateRange)
{
    QStringList scheduleIDs;
    //设置重复类型
    m_widget->setRpeat(3);
    QVector<QDateTime> beginDateTime {};
    //获取解析时间
    beginDateTime = analysisEveryWeekDate(dateRange);
    //每天重复
    if (everyDayState)
        return getEveryDaySchedule();

    for (int i = 0; i < beginDateTime.count(); i++) {
        //设置日程结束时间
        m_endtime.setDate(beginDateTime.at(i).date());
        //创建日程
        scheduleIDs.append(DScheduleDataManager::getInstance()->createSchedule(setDateTimeAndGetSchedule(beginDateTime.at(i), m_endtime)));
    }
    //创建多个日程返回第一个
    return scheduleIDs.isEmpty() ? QString() : scheduleIDs.first();
}

QString createScheduleTask::getEveryMonthSchedule(QVector<int> dateRange)
{
    QVector<QDateTime> beginDateTime {};
    QStringList scheduleIDs;
    //设置重复类型
    m_widget->setRpeat(4);
    //获取解析日期
    beginDateTime = analysisEveryMonthDate(dateRange);
    //每天重复
    if (everyDayState)
        return getEveryDaySchedule();

    for (int i = 0; i < beginDateTime.count(); i++) {
        //设置日程结束时间
        m_endtime.setDate(beginDateTime.at(i).date());
        //创建日程
        scheduleIDs.append(DScheduleDataManager::getInstance()->createSchedule(setDateTimeAndGetSchedule(beginDateTime.at(i), m_endtime)));
    }
    //创建多个日程返回第一个
    return scheduleIDs.isEmpty() ? QString() : scheduleIDs.first();
}

QString createScheduleTask::getEveryYearSchedule()
{
    //设置重复类型
    m_widget->setRpeat(5);
    //创建日程
    return DScheduleDataManager::getInstance()->createSchedule(setDateTimeAndGetSchedule(m_begintime, m_endtime));
}

QString createScheduleTask::getEveryWorkDaySchedule()
{
    //    QVector<QDateTime> beginDateTime {};
    //    DSchedule::List schedule;

    //获取解析日期
    //    beginDateTime = analysisWorkDayDate();
    //设置重复类型
    m_widget->setRpeat(2);
    //创建日程
    return DScheduleDataManager::getInstance()->createSchedule(setDateTimeAndGetSchedule(m_begintime, m_endtime));

    //    for (int i = 0; i < beginDateTime.count(); i++) {
    //        //设置日程结束时间
    //        m_endtime.setDate(beginDateTime.at(i).date());
    //        //创建日程
    //        DScheduleDataManager::getInstance()->createSchedule(setDateTimeAndGetSchedule(beginDateTime.at(i), m_endtime));
    //        //将所有日程添加到日程容器中
    //        schedule.append(setDateTimeAndGetSchedule(beginDateTime.at(i), m_endtime));
    //    }

    //    return schedule;
}

QString createScheduleTask::getEveryRestDaySchedule()
{
    QVector<QDateTime> beginDateTime {};
    QStringList scheduleIDs;
    //设置重复类型
    m_widget->setRpeat(3);
    //获取解析的时间
    beginDateTime = analysisRestDayDate();

    for (int i = 0; i < beginDateTime.count(); i++) {
        //设置日程结束时间
        m_endtime.setDate(beginDateTime.at(i).date());
        //创建日程
        scheduleIDs.append(DScheduleDataManager::getInstance()->createSchedule(setDateTimeAndGetSchedule(beginDateTime.at(i), m_endtime)));
    }

    //创建多个日程返回第一个
    return scheduleIDs.isEmpty() ? QString() : scheduleIDs.first();
}

DSchedule::Ptr createScheduleTask::getFirstSchedule(DSchedule::List scheduleInfo)
{
    //第一个日程的时间
    QDate earlyDate = scheduleInfo.at(0)->dtStart().date();
    //第一个日程的索引
    int index = 0;
    for (int i = 1; i < scheduleInfo.count(); i++) {
        if (earlyDate > scheduleInfo.at(i)->dtStart().date()) {
            earlyDate = scheduleInfo.at(i)->dtStart().date();
            index = i;
        }
    }
    return scheduleInfo.at(index);
}

int createScheduleTask::getCreatesDays(int firstDay, int secondday, bool containsToday)
{
    if (containsToday)
        //如果包含今天+1
        return secondday - firstDay + 1;
    else
        //不包含今天
        return secondday - firstDay;
}

QVector<QDateTime> createScheduleTask::getNoneWeekNumDate()
{
    QVector<QDateTime> beginDateTime {};

    if (m_begintime.time() > QTime::currentTime()) {
        //日程开始时间大于当天时间
        m_begintime.setDate(QDate::currentDate());
    } else {
        //日程开始时间小于等于当天时间
        m_begintime.setDate(QDate::currentDate().addDays(1));
    }
    beginDateTime.append(m_begintime);

    return beginDateTime;
}

QVector<QDateTime> createScheduleTask::getOneWeekNumDate(int firstWeekNum)
{
    QVector<QDateTime> beginDateTime {};
    int currentDayofWeek = QDate::currentDate().dayOfWeek();

    if (firstWeekNum >= currentDayofWeek) {
        //开始周数大于等于今天所在周数
        if (firstWeekNum == currentDayofWeek && m_begintime.time() <= QTime::currentTime()) {
            //开始周数等于今天所在周数，并且日程开始时间小于等于当前时间
            m_begintime.setDate(QDate::currentDate().addDays(firstWeekNum + 7 - currentDayofWeek));
        } else {
            //日程开始时间大于当前时间
            m_begintime.setDate(QDate::currentDate().addDays(firstWeekNum - currentDayofWeek));
        }
    } else {
        //开始周数小于今天所在周数
        m_begintime.setDate(QDate::currentDate().addDays(firstWeekNum + 7 - currentDayofWeek));
    }
    beginDateTime.append(m_begintime);

    return beginDateTime;
}

QVector<QDateTime> createScheduleTask::getTwoWeekNumDate(int firstWeekNum, int secondWeekNum)
{
    QVector<QDateTime> beginDateTime {};

    if (firstWeekNum == secondWeekNum || secondWeekNum - firstWeekNum == 6) {
        //开始周数等于结束周数，或者结束周数是7开始周数是1,为每天
        everyDayState = true;
        return beginDateTime;
    } else if (firstWeekNum < secondWeekNum) {
        //开始周数小于结束周数
        beginDateTime = firstWeekNumLessThanSecond(firstWeekNum, secondWeekNum);
    } else {
        //开始周数大于结束周数
        if (firstWeekNum - secondWeekNum == 1) {
            //开始周数比结束周数大1,为每天
            everyDayState = true;
            return beginDateTime;
        } else {
            //除去每天的其他情况
            beginDateTime = firstWeekNumGreaterThanSecondButEveryDay(firstWeekNum, secondWeekNum);
        }
    }

    return beginDateTime;
}

QVector<QDateTime> createScheduleTask::firstWeekNumLessThanSecond(int firstWeekNum, int secondWeekNum)
{
    //对于工作日和休息日的情况，助手已自动区分，此处不再讨论
    //对获取到的时间范围分情况讨论
    QVector<QDateTime> beginDateTime {};
    int currentDayofWeek = QDate::currentDate().dayOfWeek();

    //当前日期所在周数小于开始的周数
    if (currentDayofWeek < firstWeekNum) {
        beginDateTime.append(getWeekAllDateTime(QDate::currentDate().addDays(firstWeekNum - currentDayofWeek), firstWeekNum, secondWeekNum));
    } else if (currentDayofWeek >= firstWeekNum && currentDayofWeek <= secondWeekNum) {
        if (m_begintime.time() > QTime::currentTime()) {
            //开始时间大于当前时间
            beginDateTime.append(getWeekBackPartDateTime(QDate::currentDate(), secondWeekNum, true));
            beginDateTime.append(getWeekFrontPartDateTime(QDate::currentDate(), firstWeekNum, false));
        } else {
            //开始时间小于等于当前时间
            beginDateTime.append(getWeekBackPartDateTime(QDate::currentDate().addDays(1), secondWeekNum, false));
            beginDateTime.append(getWeekFrontPartDateTime(QDate::currentDate(), firstWeekNum, true));
        }
    } else if (currentDayofWeek > secondWeekNum) {
        beginDateTime.append(getWeekAllDateTime(QDate::currentDate().addDays(firstWeekNum + 7 - currentDayofWeek), firstWeekNum, secondWeekNum));
    }

    return beginDateTime;
}

QVector<QDateTime> createScheduleTask::firstWeekNumGreaterThanSecondButEveryDay(int firstWeekNum, int secondWeekNum)
{
    QVector<QDateTime> beginDateTime {};
    int currentDayofWeek = QDate::currentDate().dayOfWeek();

    if (currentDayofWeek >= firstWeekNum) {
        //今天所在周数大于等于开始周数
        if (m_begintime.time() > QTime::currentTime()) {
            //日程开始时间大于当前时间
            beginDateTime.append(getWeekBackPartDateTime(QDate::currentDate(), 7, true));
            beginDateTime.append(getWeekAllDateTime(QDate::currentDate().addDays(1 + 7 - currentDayofWeek), 1, secondWeekNum));
            beginDateTime.append(getWeekFrontPartDateTime(QDate::currentDate(), firstWeekNum, false));
        } else {
            //日程开始时间小于等于当前时间
            beginDateTime.append(getWeekBackPartDateTime(QDate::currentDate().addDays(1), 7, false));
            beginDateTime.append(getWeekAllDateTime(QDate::currentDate().addDays(1 + 7 - currentDayofWeek), 1, secondWeekNum));
            beginDateTime.append(getWeekFrontPartDateTime(QDate::currentDate(), firstWeekNum, true));
        }
    } else if (currentDayofWeek < firstWeekNum && currentDayofWeek > secondWeekNum) {
        //今天所在周数小于开始周数，大于结束周数
        beginDateTime.append(getWeekAllDateTime(QDate::currentDate().addDays(firstWeekNum - currentDayofWeek), firstWeekNum, 7));
        beginDateTime.append(getWeekAllDateTime(QDate::currentDate().addDays(1 + 7 - currentDayofWeek), 1, secondWeekNum));
    } else if (currentDayofWeek <= secondWeekNum) {
        //今天所在周数小于等于结束周数
        if (m_begintime.time() > QTime::currentTime()) {
            //日程开始时间大于当前时间
            beginDateTime.append(getWeekBackPartDateTime(QDate::currentDate(), secondWeekNum, true));
            beginDateTime.append(getWeekAllDateTime(QDate::currentDate().addDays(firstWeekNum - currentDayofWeek), firstWeekNum, 7));
            beginDateTime.append(getWeekFrontPartDateTime(QDate::currentDate(), 1, false));
        } else {
            //日程开始时间小于等于当前时间
            beginDateTime.append(getWeekBackPartDateTime(QDate::currentDate().addDays(1), secondWeekNum, false));
            beginDateTime.append(getWeekAllDateTime(QDate::currentDate().addDays(firstWeekNum - currentDayofWeek), firstWeekNum, 7));
            beginDateTime.append(getWeekFrontPartDateTime(QDate::currentDate(), 1, true));
        }
    }

    return beginDateTime;
}

QVector<QDateTime> createScheduleTask::getWeekAllDateTime(QDate BeginDate, int firstWeekNum, int secondWeekNum)
{
    QVector<QDateTime> beginDateTime;

    for (int i = 0; i < secondWeekNum - firstWeekNum + 1; i ++) {
        m_begintime.setDate(BeginDate.addDays(i));
        beginDateTime.append(m_begintime);
    }

    return beginDateTime;
}

QVector<QDateTime> createScheduleTask::getWeekFrontPartDateTime(QDate BeginDate, int firstWeekNum, bool containsToday)
{
    QVector<QDateTime> beginDateTime {};
    int currentDayofWeek = QDate::currentDate().dayOfWeek();
    int addDays = getCreatesDays(firstWeekNum, currentDayofWeek, containsToday);

    for (int i = 0; i < addDays; i++) {
        m_begintime.setDate(BeginDate.addDays(firstWeekNum + 7 - currentDayofWeek + i));
        beginDateTime.append(m_begintime);
    }

    return beginDateTime;
}

QVector<QDateTime> createScheduleTask::getWeekBackPartDateTime(QDate BeginDate, int secondWeekNum, bool containsToday)
{
    QVector<QDateTime> beginDateTime {};
    int currentDayofWeek = QDate::currentDate().dayOfWeek();
    int addDays = getCreatesDays(currentDayofWeek, secondWeekNum, containsToday);

    for (int i = 0; i < addDays; i++) {
        m_begintime.setDate(BeginDate.addDays(i));
        beginDateTime.append(m_begintime);
    }

    return beginDateTime;
}

DSchedule::Ptr createScheduleTask::setDateTimeAndGetSchedule(QDateTime beginDateTime, QDateTime endDateTime)
{
    m_widget->setDateTime(beginDateTime, endDateTime);
    m_widget->setschedule();

    return m_widget->getScheduleDtailInfo();
}

QVector<QDateTime> createScheduleTask::analysisWorkDayDate()
{
    QVector<QDateTime> beginDateTime {};

    if (m_begintime.date().dayOfWeek() == 6)
        m_begintime.setDate(m_begintime.date().addDays(2));
    if (m_begintime.date().dayOfWeek() == 7)
        m_begintime.setDate(m_begintime.date().addDays(1));
    beginDateTime.append(m_begintime);

    return beginDateTime;
}

QDate createScheduleTask::getValidDate(QDate viewDate, int viewDateDay)
{
    //设置一个无效时间
    QDate validDate(QDate(0, 0, 0));
    //初始化年
    int month = viewDate.month();
    //判断未来半年的时间是否合法
    for (int i = month; i <= month + 6; i++) {
        //设置月
        validDate = viewDate.addMonths(i - month);
        //设置时间
        validDate.setDate(validDate.year(), validDate.month(), viewDateDay);
        //判断日期是否合法
        if (validDate.isValid())
            return validDate;
    }
    //返回判断时间
    return validDate;
}

/**
 * @brief createScheduleTask::shouldEndSession 是否需要进行多轮
 * @return
 */
bool createScheduleTask::shouldEndSession(CreateJsonData *createjsondate)
{
    return createjsondate->ShouldEndSession();
}

/**
 * @brief createScheduleTask::validDateTime 是否是有效时间
 * @param datetime 查询的时间
 * @return
 */
bool createScheduleTask::validDateTime(QDateTime datetime)
{
    return datetime.isValid();
}

QVector<QDateTime> createScheduleTask::analysisEveryWeekDate(QVector<int> dateRange)
{
    QVector<QDateTime> beginDateTime {};

    switch (dateRange.size()) {
    case 0: {
        //每周重复日程，没有具体周数
        beginDateTime = getNoneWeekNumDate();
    }
    break;
    case 1: {
        //每周重复日程，只有一个周数
        beginDateTime = getOneWeekNumDate(dateRange[0]);
    }
    break;
    case 2: {
        //每周重复日程，有两个周数
        beginDateTime = getTwoWeekNumDate(dateRange[0], dateRange[1]);
    }
    break;
    }

    return beginDateTime;
}

QVector<QDateTime> createScheduleTask::analysisEveryMonthDate(QVector<int> dateRange)
{
    QVector<QDateTime> beginDateTime {};

    switch (dateRange.size()) {
    case 0: {
        //每月重复日程，没有具体日期
        beginDateTime = getNoneMonthNumDate();
    }
    break;
    case 1: {
        //每月重复日程，只有一个日期
        beginDateTime = getOneMonthNumDate(dateRange[0]);
    }
    break;
    case 2: {
        //每月重复日程，有两个日期
        beginDateTime = getTwoMonthNumDate(dateRange[0], dateRange[1]);
    }
    break;
    }

    return beginDateTime;
}

QVector<QDateTime> createScheduleTask::getNoneMonthNumDate()
{
    QVector<QDateTime> beginDateTime {};

    if (m_begintime.time() > QTime::currentTime()) {
        //日程开始时间大于当前时间
        m_begintime.setDate(QDate::currentDate());
    } else {
        //日程开始时间小于等于当前时间
        m_begintime.setDate(QDate::currentDate().addDays(1));
    }
    beginDateTime.append(m_begintime);

    return beginDateTime;
}

QVector<QDateTime> createScheduleTask::getOneMonthNumDate(int firstMonthNum)
{
    QVector<QDateTime> beginDateTime {};
    //今天
    int currentDayofMonth = QDate::currentDate().day();

    if (currentDayofMonth < firstMonthNum) {
        //今天小于开始日期
        //获取合法日期
        QDate validDate = getValidDate(QDate::currentDate(), firstMonthNum);
        //日期合法，设置日期
        if (validDate.isValid())
            m_begintime.setDate(validDate);
    } else if (currentDayofMonth > firstMonthNum) {
        //获取合法日期
        QDate validDate = getValidDate(QDate::currentDate().addMonths(1), firstMonthNum);
        //日期合法，设置日期
        if (validDate.isValid())
            m_begintime.setDate(validDate);
    } else {
        //当前日期等于开始日期
        if (m_begintime.time() > QTime::currentTime()) {
            //日程开始时间大于当前时间
            m_begintime.setDate(QDate::currentDate());
        } else {
            //日程开始时间小于等于当前时间
            m_begintime.setDate(QDate::currentDate().addMonths(1));
        }
    }
    beginDateTime.append(m_begintime);

    return beginDateTime;
}

QVector<QDateTime> createScheduleTask::getTwoMonthNumDate(int firstMonthNum, int secondMonthNum)
{
    QVector<QDateTime> beginDateTime {};

    if (firstMonthNum == secondMonthNum) {
        //开始日期等于结束日期，为每天
        everyDayState = true;
        return beginDateTime;
    } else if (firstMonthNum < secondMonthNum) {
        //开始日期小于结束日期
        beginDateTime.append(firstMonthNumLessThanSecond(firstMonthNum, secondMonthNum));
    } else {
        //开始日期大于结束日期
        if (firstMonthNum - secondMonthNum == 1) {
            //开始日期和结束日期差一天，为每天
            everyDayState = true;
            return beginDateTime;
        } else {
            //开始日期大于结束日期的其他情况
            beginDateTime.append(firstMonthNumGreaterThanSecondButEveryDay(firstMonthNum, secondMonthNum));
        }
    }

    return beginDateTime;
}

QVector<QDateTime> createScheduleTask::firstMonthNumLessThanSecond(int firstMonthNum, int secondWMonthNum)
{
    QVector<QDateTime> beginDateTime {};
    int currentDayofMonth = QDate::currentDate().day();

    if (currentDayofMonth < firstMonthNum) {
        //今天的日期小于开始日期
        beginDateTime.append(getMonthAllDateTime(QDate::currentDate().addDays(firstMonthNum - currentDayofMonth), firstMonthNum, secondWMonthNum));
    } else if (currentDayofMonth >= firstMonthNum && currentDayofMonth <= secondWMonthNum) {
        //今天的日期大于等于开始日期，小于等于结束日期
        if (m_begintime.time() > QTime::currentTime()) {
            //日程开始时间大于当前时间
            beginDateTime.append(getMonthBackPartDateTime(QDate::currentDate(), secondWMonthNum, true));
            beginDateTime.append(getMonthFrontPartDateTime(QDate::currentDate(), firstMonthNum, false));
        } else {
            //日程开始时间小于等于当前时间
            beginDateTime.append(getMonthBackPartDateTime(QDate::currentDate().addDays(1), secondWMonthNum, false));
            beginDateTime.append(getMonthFrontPartDateTime(QDate::currentDate(), firstMonthNum, true));
        }
    } else if (currentDayofMonth > secondWMonthNum) {
        //今天的日程大于结束日期
        beginDateTime.append(getMonthAllDateTime(QDate::currentDate().addDays(firstMonthNum - currentDayofMonth).addMonths(1), firstMonthNum, secondWMonthNum));
    }

    return beginDateTime;
}

QVector<QDateTime> createScheduleTask::getMonthAllDateTime(QDate BeginDate, int firstMonthNum, int secondMonthNum)
{
    QVector<QDateTime> beginDateTime;
    //设置日期
    for (int i = firstMonthNum; i <= secondMonthNum; i++) {
        //日期是否合法
        QDate validDate = getValidDate(BeginDate, i);
        //合法日期，设置日期
        if (validDate.isValid()) {
            m_begintime.setDate(validDate);
            beginDateTime.append(m_begintime);
        }
    }

    return beginDateTime;
}

QVector<QDateTime> createScheduleTask::getMonthFrontPartDateTime(QDate BeginDate, int firstMonthNum, bool containsToday)
{
    QVector<QDateTime> beginDateTime {};
    int currentDayofMonth = QDate::currentDate().day();
    //包含今天
    if (containsToday)
        currentDayofMonth += 1;
    //设置日期
    for (int i = firstMonthNum; i < currentDayofMonth; i++) {
        //获取合法日期
        QDate validDate = getValidDate(BeginDate.addDays(firstMonthNum - currentDayofMonth).addMonths(1), i);
        //合法日期
        if (validDate.isValid()) {
            m_begintime.setDate(validDate);
            beginDateTime.append(m_begintime);
        }
    }

    return beginDateTime;
}

QVector<QDateTime> createScheduleTask::getMonthBackPartDateTime(QDate BeginDate, int secondMonthNum, bool containsToday)
{
    QVector<QDateTime> beginDateTime {};
    int currentDayofMonth = QDate::currentDate().day();
    //不包含今天
    if (!containsToday)
        currentDayofMonth += 1;
    //设置日期
    for (int i = currentDayofMonth; i < secondMonthNum + 1; i++) {
        //获取合法日期
        QDate validDate = getValidDate(BeginDate, i);
        //日期合法
        if (validDate.isValid()) {
            m_begintime.setDate(validDate);
            beginDateTime.append(m_begintime);
        }
    }

    return beginDateTime;

}

QVector<QDateTime> createScheduleTask::firstMonthNumGreaterThanSecondButEveryDay(int firstMonthNum, int secondMonthNum)
{
    QVector<QDateTime> beginDateTime {};
    int currentDayofMonth = QDate::currentDate().day();

    if (currentDayofMonth >= firstMonthNum) {
        //今天的日期大于等于开始的日期
        if (m_begintime.time() > QTime::currentTime()) {
            //日程开始时间大于当前时间
            beginDateTime.append(getMonthBackPartDateTime(QDate::currentDate(), QDate::currentDate().daysInMonth(), true));
            beginDateTime.append(getMonthAllDateTime(QDate::currentDate().addDays(1 - currentDayofMonth), 1, secondMonthNum));
            beginDateTime.append(getMonthFrontPartDateTime(QDate::currentDate(), firstMonthNum, false));
        } else {
            //日程开始时间小于等于当前时间
            beginDateTime.append(getMonthBackPartDateTime(QDate::currentDate().addDays(1), QDate::currentDate().daysInMonth(), false));
            beginDateTime.append(getMonthAllDateTime(QDate::currentDate().addDays(1 - currentDayofMonth), 1, secondMonthNum));
            beginDateTime.append(getMonthFrontPartDateTime(QDate::currentDate(), firstMonthNum, true));
        }
    } else if (currentDayofMonth < firstMonthNum && currentDayofMonth > secondMonthNum) {
        //今天的日期小于开始日期，大于结束日期
        beginDateTime.append(getMonthAllDateTime(QDate::currentDate().addDays(firstMonthNum - currentDayofMonth), firstMonthNum, QDate::currentDate().daysInMonth()));
        beginDateTime.append(getMonthAllDateTime(QDate::currentDate().addDays(1 - currentDayofMonth).addMonths(1), 1, secondMonthNum));
    } else if (currentDayofMonth <= secondMonthNum) {
        //今天的日期小于等于结束日期
        if (m_begintime.time() > QTime::currentTime()) {
            //日程开始时间大于当前时间
            beginDateTime.append(getMonthBackPartDateTime(QDate::currentDate(), secondMonthNum, true));
            beginDateTime.append(getMonthAllDateTime(QDate::currentDate().addDays(firstMonthNum - currentDayofMonth), firstMonthNum, QDate::currentDate().daysInMonth()));
            beginDateTime.append(getMonthFrontPartDateTime(QDate::currentDate(), 1, false));
        } else {
            //日程开始时间小于等于当前时间
            beginDateTime.append(getMonthBackPartDateTime(QDate::currentDate().addDays(1), secondMonthNum, false));
            beginDateTime.append(getMonthAllDateTime(QDate::currentDate().addDays(firstMonthNum - currentDayofMonth), firstMonthNum, QDate::currentDate().daysInMonth()));
            beginDateTime.append(getMonthFrontPartDateTime(QDate::currentDate(), 1, true));
        }
    }

    return beginDateTime;
}

QVector<QDateTime> createScheduleTask::analysisRestDayDate()
{
    QVector<QDateTime> beginDateTime {};
    //周六的日程开始时间
    QDateTime beginDateTimeSat = m_begintime;
    //周日的日程开始时间
    QDateTime beginDateTimeSun = m_begintime;
    int currentDayofWeek = QDate::currentDate().dayOfWeek();
    //设置周六的时间
    beginDateTimeSat.setDate(QDate::currentDate().addDays(6 - currentDayofWeek));
    //设置周日的时间
    beginDateTimeSun.setDate(QDate::currentDate().addDays(7 - currentDayofWeek));
    //如果周六的时间小于当前时间，设置日程开始时间为下一周的周六
    if (beginDateTimeSat < QDateTime::currentDateTime())
        beginDateTimeSat.setDate(beginDateTimeSat.date().addDays(7));
    //如果周日的时间小于当前时间，设置日程开始时间为下一周的周日
    if (beginDateTimeSun < QDateTime::currentDateTime())
        beginDateTimeSun.setDate(beginDateTimeSun.date().addDays(7));
    //将周末的日程开始时间放到时间容器里面
    beginDateTime.append(beginDateTimeSat);
    beginDateTime.append(beginDateTimeSun);

    return beginDateTime;
}
