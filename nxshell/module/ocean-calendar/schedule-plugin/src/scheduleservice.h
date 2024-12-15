// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SCHEDULESERVICE_H
#define SCHEDULESERVICE_H

#include "interface/service.h"

class ScheduleManageTask;
class scheduleservice : public IService
{
    Q_OBJECT
public:
    scheduleservice();
    ~scheduleservice() override;
    QString serviceName() override;
    int servicePriority() override;
    bool canHandle(const QString &s) override;
    IRet service(const QString &semantic) override;
    Reply &getServiceReply() override;
    ScheduleManageTask *getScheduleManageTask() const;

private:
    Reply m_reply;
};

#endif // SCHEDULESERVICE_H
