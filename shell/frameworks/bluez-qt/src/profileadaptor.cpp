/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "profileadaptor.h"
#include "device.h"
#include "manager.h"
#include "profile.h"

#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusUnixFileDescriptor>

namespace BluezQt
{
ProfileAdaptor::ProfileAdaptor(Profile *parent, Manager *manager)
    : QDBusAbstractAdaptor(parent)
    , m_profile(parent)
    , m_manager(manager)
{
}

void ProfileAdaptor::NewConnection(const QDBusObjectPath &device, const QDBusUnixFileDescriptor &fd, const QVariantMap &properties, const QDBusMessage &msg)
{
    msg.setDelayedReply(true);
    Request<> req(OrgBluezProfile, msg);

    DevicePtr dev = m_manager->deviceForUbi(device.path());
    if (!dev) {
        req.cancel();
        return;
    }

    m_profile->newConnection(dev, fd, properties, req);
}

void ProfileAdaptor::RequestDisconnection(const QDBusObjectPath &device, const QDBusMessage &msg)
{
    msg.setDelayedReply(true);
    Request<> req(OrgBluezProfile, msg);

    DevicePtr dev = m_manager->deviceForUbi(device.path());
    if (!dev) {
        req.cancel();
        return;
    }

    m_profile->requestDisconnection(dev, req);
}

void ProfileAdaptor::Release()
{
    m_profile->release();
}

} // namespace BluezQt

#include "moc_profileadaptor.cpp"
