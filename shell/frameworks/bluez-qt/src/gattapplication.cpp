/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "gattapplication.h"

#include "gattapplication_p.h"
#include "gattcharacteristic.h"
#include "gattcharacteristicadaptor.h"
#include "gattdescriptor.h"
#include "gattdescriptoradaptor.h"
#include "gattservice.h"
#include "gattserviceadaptor.h"

#include <QDBusObjectPath>
#include <QMetaProperty>

namespace BluezQt
{
GattApplication::GattApplication(QObject *parent)
    : GattApplication(QStringLiteral("/org/kde/bluezqt"), parent)
{
}

GattApplication::GattApplication(const QString &objectPathPrefix, QObject *parent)
    : QObject(parent)
    , d(new GattApplicationPrivate(objectPathPrefix, this))
{
}

GattApplication::~GattApplication() = default;

DBusManagerStruct GattApplicationPrivate::getManagedObjects() const
{
    DBusManagerStruct objects;

    const auto serviceAdaptors = q->findChildren<GattServiceAdaptor *>();
    const auto charcAdaptors = q->findChildren<GattCharacteristicAdaptor *>();
    const auto descriptorAdaptors = q->findChildren<GattDescriptorAdaptor *>();

    for (const GattServiceAdaptor *serviceAdaptor : serviceAdaptors) {
        QVariantMap properties;
        for (int i = serviceAdaptor->metaObject()->propertyOffset(); i < serviceAdaptor->metaObject()->propertyCount(); ++i) {
            auto propertyName = serviceAdaptor->metaObject()->property(i).name();
            properties.insert(QString::fromLatin1(propertyName), serviceAdaptor->property(propertyName));
        }

        GattService *service = qobject_cast<GattService *>(serviceAdaptor->parent());
        if (service) {
            objects[service->objectPath()].insert(QStringLiteral("org.bluez.GattService1"), properties);
        }
    }

    for (const GattCharacteristicAdaptor *charcAdaptor : charcAdaptors) {
        QVariantMap properties;
        for (int i = charcAdaptor->metaObject()->propertyOffset(); i < charcAdaptor->metaObject()->propertyCount(); ++i) {
            auto propertyName = charcAdaptor->metaObject()->property(i).name();
            properties.insert(QString::fromLatin1(propertyName), charcAdaptor->property(propertyName));
        }

        GattCharacteristic *charc = qobject_cast<GattCharacteristic *>(charcAdaptor->parent());
        if (charc) {
            objects[charc->objectPath()].insert(QStringLiteral("org.bluez.GattCharacteristic1"), properties);
        }
    }

    for (const GattDescriptorAdaptor *descAdaptor : descriptorAdaptors) {
        QVariantMap properties;
        for (int i = descAdaptor->metaObject()->propertyOffset(); i < descAdaptor->metaObject()->propertyCount(); ++i) {
            auto propertyName = descAdaptor->metaObject()->property(i).name();
            properties.insert(QString::fromLatin1(propertyName), descAdaptor->property(propertyName));
        }

        GattDescriptor *desc = qobject_cast<GattDescriptor *>(descAdaptor->parent());
        if (desc) {
            objects[desc->objectPath()].insert(QStringLiteral("org.bluez.GattDescriptor1"), properties);
        }
    }

    return objects;
}

QDBusObjectPath GattApplication::objectPath() const
{
    return d->m_objectPath;
}

} // namespace BluezQt

#include "moc_gattapplication.cpp"
