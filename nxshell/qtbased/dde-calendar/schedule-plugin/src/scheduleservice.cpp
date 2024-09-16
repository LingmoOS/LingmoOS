// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "scheduleservice.h"

#include "globaldef.h"
#include "task/schedulemanagetask.h"
#include "task/semanticanalysistask.h"

scheduleservice::scheduleservice()
{
}

scheduleservice::~scheduleservice()
{
}

QString scheduleservice::serviceName()
{
    return SERVICE_NAME;
}

int scheduleservice::servicePriority()
{
    return 0;
}

bool scheduleservice::canHandle(const QString &s)
{
    Q_UNUSED(s);
    return true;
}

IRet scheduleservice::service(const QString &semantic)
{
    //解析云端返回的数据,进行业务处理
    QString str = semantic;
    semanticAnalysisTask task;
    if (!task.resolveTaskJson(str)) {
        return ERR_FAIL;
    }
    ScheduleManageTask::getInstance()->process(task);
    return ERR_SUCCESS;
}

Reply &scheduleservice::getServiceReply()
{
    m_reply = ScheduleManageTask::getInstance()->getReply();
    return m_reply;
}

ScheduleManageTask *scheduleservice::getScheduleManageTask() const
{
    return ScheduleManageTask::getInstance();
}
