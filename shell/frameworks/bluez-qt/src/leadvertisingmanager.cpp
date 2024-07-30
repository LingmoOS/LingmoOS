/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "leadvertisingmanager.h"
#include "debug.h"
#include "leadvertisement.h"
#include "leadvertisementadaptor.h"
#include "leadvertisingmanager_p.h"
#include "pendingcall.h"
#include "utils.h"

namespace BluezQt
{
LEAdvertisingManager::LEAdvertisingManager(const QString &path, QObject *parent)
    : QObject(parent)
    , d(new LEAdvertisingManagerPrivate())
{
    d->m_path = path;
    d->m_bluezLEAdvertisingManager = new BluezLEAdvertisingManager(Strings::orgBluez(), path, DBusConnection::orgBluez(), this);
}

LEAdvertisingManager::~LEAdvertisingManager() = default;

PendingCall *LEAdvertisingManager::registerAdvertisement(LEAdvertisement *advertisement)
{
    Q_ASSERT(advertisement);

    if (!d->m_bluezLEAdvertisingManager) {
        return new PendingCall(PendingCall::InternalError, QStringLiteral("LEAdvertisingManager not operational!"));
    }

    new LEAdvertisementAdaptor(advertisement);

    if (!DBusConnection::orgBluez().registerObject(advertisement->objectPath().path(), advertisement)) {
        qCDebug(BLUEZQT) << "Cannot register object" << advertisement->objectPath().path();
    }

    return new PendingCall(d->m_bluezLEAdvertisingManager->RegisterAdvertisement(advertisement->objectPath(), QVariantMap()), PendingCall::ReturnVoid, this);
}

PendingCall *LEAdvertisingManager::unregisterAdvertisement(LEAdvertisement *advertisement)
{
    Q_ASSERT(advertisement);

    if (!d->m_bluezLEAdvertisingManager) {
        return new PendingCall(PendingCall::InternalError, QStringLiteral("LEAdvertisingManager not operational!"));
    }

    DBusConnection::orgBluez().unregisterObject(advertisement->objectPath().path());

    return new PendingCall(d->m_bluezLEAdvertisingManager->UnregisterAdvertisement(advertisement->objectPath()), PendingCall::ReturnVoid, this);
}

} // namespace BluezQt

#include "moc_leadvertisingmanager.cpp"
