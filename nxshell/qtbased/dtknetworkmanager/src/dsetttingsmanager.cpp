// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dsettingsmanager.h"
#include "dsettingsmanager_p.h"
#include "dnmutils.h"
DNETWORKMANAGER_BEGIN_NAMESPACE

using DCORE_NAMESPACE::DUnexpected;
using DCORE_NAMESPACE::emplace_tag;

DSettingsManagerPrivate::DSettingsManagerPrivate(DSettingsManager *parent)
    : q_ptr(parent)
    , m_settings(new DSettingsManagerInterface(this))
{
}

DSettingsManager::DSettingsManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new DSettingsManagerPrivate(this))
{
    Q_D(const DSettingsManager);
    connect(d->m_settings, &DSettingsManagerInterface::ConnectionRemoved, this, [this](const QDBusObjectPath &path) {
        Q_EMIT this->ConnectionRemoved(getIdFromObjectPath(path));
    });
    connect(d->m_settings, &DSettingsManagerInterface::NewConnection, this, [this](const QDBusObjectPath &path) {
        Q_EMIT this->NewConnection(getIdFromObjectPath(path));
    });
}

DSettingsManager::~DSettingsManager() = default;

DExpected<QList<quint64>> DSettingsManager::listConnections() const
{
    Q_D(const DSettingsManager);
    auto reply = d->m_settings->listConnections();
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    QList<quint64> ret;
    for (const auto &it : reply.value())
        ret.append(getIdFromObjectPath(it));
    return ret;
}

DExpected<quint64> DSettingsManager::getConnectionByUUID(const QUuid &UUID) const
{
    Q_D(const DSettingsManager);
    auto reply = d->m_settings->getConnectionByUUID(UUID.toByteArray(QUuid::StringFormat::WithoutBraces));
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return getIdFromObjectPath(reply.value());
}

DExpected<quint64> DSettingsManager::addConnection(const SettingDesc &conn) const
{
    Q_D(const DSettingsManager);
    auto reply = d->m_settings->addConnection(conn);
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return getIdFromObjectPath(reply.value());
}

DNETWORKMANAGER_END_NAMESPACE
