// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "scheduleplugin.h"

#include "scheduleservice.h"
#include "globaldef.h"
#include "task/schedulemanagetask.h"

Scheduleplugin::Scheduleplugin(QObject *parent)
{
    Q_UNUSED(parent);
}

Scheduleplugin::~Scheduleplugin()
{
    ScheduleManageTask::releaseInstance();
}

int Scheduleplugin::init()
{
    return 0;
}

void Scheduleplugin::uninit()
{
}

QStringList Scheduleplugin::getSupportService()
{
    return QStringList {SERVICE_NAME};
}

IService *Scheduleplugin::createService(const QString &service)
{
    if (!getSupportService().contains(service)) {
        return nullptr;
    }
    QMutexLocker lock(&m_ServiceLock);
    auto pService = new scheduleservice();
    connect(pService->getScheduleManageTask(), &ScheduleManageTask::signaleSendMessage,
            this, &Scheduleplugin::slotSendMessage, Qt::UniqueConnection);
    m_ServiceSet.insert(pService);
    return pService;
}

void Scheduleplugin::releaseService(IService *service)
{
    QMutexLocker lock(&m_ServiceLock);
    for (auto iter = m_ServiceSet.begin(); iter != m_ServiceSet.end(); iter++) {
        if (service && service == *iter) {
            m_ServiceSet.erase(iter);
            break;
        }
    }
    if (service) {
        delete service;
    }
}

bool Scheduleplugin::needRunInDifferentThread()
{
    return false;
}

bool Scheduleplugin::isAutoDetectSession()
{
    //设置为不自动检测是否多论
    return false;
}

void Scheduleplugin::slotSendMessage(Reply reply)
{
    if (this->m_messageHandle)
        this->m_messageHandle(this, reply);
}
