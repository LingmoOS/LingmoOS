/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "leadvertisementadaptor.h"
#include "leadvertisement.h"

#include <QDBusMetaType>
#include <QDBusObjectPath>

namespace BluezQt
{
LEAdvertisementAdaptor::LEAdvertisementAdaptor(LEAdvertisement *parent)
    : QDBusAbstractAdaptor(parent)
    , m_advertisement(parent)
{
    qDBusRegisterMetaType<QHash<QString, QVariant>>();
    qDBusRegisterMetaType<QHash<quint16, QDBusVariant>>();
}

QString LEAdvertisementAdaptor::type() const
{
    return QStringLiteral("peripheral");
}

QStringList LEAdvertisementAdaptor::serviceUuids() const
{
    return m_advertisement->serviceUuids();
}

QHash<QString, QVariant> LEAdvertisementAdaptor::serviceData() const
{
    // bluez wants the value wrapped into a variant...
    QHash<QString, QVariant> data;
    const auto sd = m_advertisement->serviceData();
    data.reserve(sd.size());
    for (auto it = sd.begin(); it != sd.end(); ++it) {
        data.insert(it.key(), it.value());
    }
    return data;
}

QHash<quint16, QDBusVariant> LEAdvertisementAdaptor::manufacturerData() const
{
    QHash<quint16, QDBusVariant> data;

    const auto &md = m_advertisement->manufacturerData();
    data.reserve(md.size());

    for (auto it = md.cbegin(); it != md.cend(); ++it) {
        data.insert(it.key(), QDBusVariant(QVariant::fromValue(it.value())));
    }

    return data;
}

void LEAdvertisementAdaptor::Release()
{
    m_advertisement->release();
}

} // namespace BluezQt

#include "moc_leadvertisementadaptor.cpp"
