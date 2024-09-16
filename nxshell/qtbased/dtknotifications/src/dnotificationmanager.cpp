// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dnotificationmanager.h"
#include "dnotificationmanager_p.h"
#include "dnotificationutils.h"
#include "dbus/dnotificationinterface.h"
#include <QDebug>

DNOTIFICATIONS_BEGIN_NAMESPACE

using DCORE_NAMESPACE::DError;
using DCORE_NAMESPACE::DExpected;
using DCORE_NAMESPACE::DUnexpected;

DNotificationManagerPrivate::DNotificationManagerPrivate(DNotificationManager *parent)
    : q_ptr(parent)
    , m_dNoticationInter(new DNotificationInterface(this))
{
}

DNotificationManager::DNotificationManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new DNotificationManagerPrivate(this))
{
    Q_D(const DNotificationManager);
    connect(d->m_dNoticationInter, &DNotificationInterface::actionInvoked, this, &DNotificationManager::actionInvoked);
    connect(d->m_dNoticationInter, &DNotificationInterface::notificationClosed, this, &DNotificationManager::notificationClosed);
    connect(d->m_dNoticationInter, &DNotificationInterface::recordAdded, this, &DNotificationManager::recordAdded);
    connect(d->m_dNoticationInter, &DNotificationInterface::appInfoChanged, this, &DNotificationManager::appNotificationConfigChanged);
    connect(d->m_dNoticationInter, &DNotificationInterface::systemInfoChanged, this, &DNotificationManager::dndModeNotificationConfigChanged);
    connect(d->m_dNoticationInter, &DNotificationInterface::appAdded, this, &DNotificationManager::appAdded);
    connect(d->m_dNoticationInter, &DNotificationInterface::appRemoved, this, &DNotificationManager::appRemoved);
}

DNotificationManager::~DNotificationManager() {}

DExpected<QList<DNotificationData> > DNotificationManager::allRecords() const
{
    Q_D(const DNotificationManager);
    auto reply = d->m_dNoticationInter->allRecords();
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    } else {
        return Utils::json2DNotificationDatas(reply.value());
    }
}

QList<QString> DNotificationManager::recordIds() const
{
    QList<QString> ids;
    auto records = allRecords();
    for (auto record : records.value()) {
        ids << record.id;
    }
    return ids;
}

DExpected<QList<DNotificationData>> DNotificationManager::getRecordsFromId(int rowCount, const QString &offsetId)
{
    Q_D(const DNotificationManager);
    auto reply = d->m_dNoticationInter->getRecordsFromId(rowCount, offsetId);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    } else {
        return Utils::json2DNotificationDatas(reply.value());
    }
}

DExpected<DNotificationData> DNotificationManager::getRecordById(const QString &id)
{
    Q_D(const DNotificationManager);
    auto reply = d->m_dNoticationInter->getRecordById(id);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    } else {
        auto result = Utils::json2DNotificationDatas(reply.value());
        if (!result.isEmpty())
            return result.at(0);
    }
    return DNotificationData();
}

DExpected<int> DNotificationManager::recordCount()
{
    Q_D(const DNotificationManager);
    auto reply = d->m_dNoticationInter->recordCount();
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    } else {
        return static_cast<int>(reply.value());
    }
}

DExpected<void> DNotificationManager::clearRecords()
{
    Q_D(const DNotificationManager);
    auto reply = d->m_dNoticationInter->clearRecords();
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    }
    return {};
}

DExpected<void> DNotificationManager::removeRecord(const QString &id)
{
    Q_D(const DNotificationManager);
    auto reply = d->m_dNoticationInter->removeRecord(id);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    }
    return {};
}

DExpected<ServerInformation> DNotificationManager::serverInformation()
{
    Q_D(const DNotificationManager);
    auto reply = d->m_dNoticationInter->serverInformation();
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    } else {
        ServerInformation serverInformation;
        if (reply.argumentAt(0).isValid()) {
            serverInformation.name = reply.argumentAt(0).toString();
        }
        if (reply.argumentAt(1).isValid()) {
            serverInformation.vendor = reply.argumentAt(1).toString();
        }
        if (reply.argumentAt(2).isValid()) {
            serverInformation.version = reply.argumentAt(2).toString();
        }
        if (reply.argumentAt(3).isValid()) {
            serverInformation.spec_version = reply.argumentAt(3).toString();
        }
        return serverInformation;
    }
}

DExpected<QStringList> DNotificationManager::capbilities()
{
    Q_D(const DNotificationManager);
    auto reply = d->m_dNoticationInter->capbilities();
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    } else {
        return reply.value();
    }
}

DExpected<uint> DNotificationManager::notify(const QString &appName, uint replacesId,
                                  const QString &appIcon, const QString &summary,
                                  const QString &body, const QStringList &actions,
                                  const QVariantMap hints, int expireTimeout)
{
    Q_D(const DNotificationManager);
    auto reply = d->m_dNoticationInter->notify(appName, replacesId,
                                               appIcon, summary,
                                               body, actions,
                                               hints, expireTimeout);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    } else {
        return reply.value();
    }
}

DExpected<void> DNotificationManager::closeNotification(uint id)
{
    Q_D(const DNotificationManager);
    auto reply = d->m_dNoticationInter->closeNotification(id);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    }
    return {};
}

DExpected<QStringList> DNotificationManager::appList()
{
    Q_D(const DNotificationManager);
    auto reply = d->m_dNoticationInter->appList();
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    } else {
        return reply.value();
    }
}

DNotificationAppConfigPtr DNotificationManager::notificationAppConfig(const QString &id) const
{
    DNotificationAppConfigPtr appNotificationConfigPtr(new DNotificationAppConfig(id));
    return appNotificationConfigPtr;
}

DNotificationDNDModeConfigPtr DNotificationManager::notificationDNDModeConfig() const
{
    DNotificationDNDModeConfigPtr notificationDNDModeConfigPtr(new DNotificationDNDModeConfig("dndMode"));
    return notificationDNDModeConfigPtr;
}

QList<DAbstractNotificationModeConfigPtr> DNotificationManager::notificationModeConfigs() const
{
    QList<DAbstractNotificationModeConfigPtr> notificationModeConfigs;
    DAbstractNotificationModeConfigPtr dndModeConfig(new DNotificationDNDModeConfig("dndMode"));
    notificationModeConfigs << dndModeConfig;
    return notificationModeConfigs;
}

DExpected<void> DNotificationManager::toggleNotificationCenter()
{
    Q_D(const DNotificationManager);
    auto reply = d->m_dNoticationInter->toggle();
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    }
    return {};
}

DExpected<void> DNotificationManager::showNotificationCenter()
{
    Q_D(const DNotificationManager);
    auto reply = d->m_dNoticationInter->show();
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    }
    return {};
}

DExpected<void> DNotificationManager::hideNotificationCenter()
{
    Q_D(const DNotificationManager);
    auto reply = d->m_dNoticationInter->hide();
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    }
    return {};
}

DNOTIFICATIONS_END_NAMESPACE
