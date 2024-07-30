/*
    SPDX-FileCopyrightText: 2010 Michael Zanetti <mzanetti@kde.org>
    SPDX-FileCopyrightText: 2010-2012 Lukáš Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2012 Dan Vrátil <dvratil@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef UDISKSDEVICEBACKEND_H
#define UDISKSDEVICEBACKEND_H

#include <QDBusObjectPath>
#include <QObject>
#include <QStringList>
#include <QThreadStorage>

#include "udisks2.h"

namespace Solid
{
namespace Backends
{
namespace UDisks2
{
class DeviceBackend : public QObject
{
    Q_OBJECT

public:
    static DeviceBackend *backendForUDI(const QString &udi, bool create = true);
    static void destroyBackend(const QString &udi);

    DeviceBackend(const QString &udi);
    ~DeviceBackend() override;

    QVariant prop(const QString &key) const;
    bool propertyExists(const QString &key) const;
    QVariantMap allProperties() const;

    QStringList interfaces() const;
    const QString &udi() const;

    void invalidateProperties();
Q_SIGNALS:
    void propertyChanged(const QMap<QString, int> &changeMap);
    void changed();

private Q_SLOTS:
    void slotInterfacesAdded(const QDBusObjectPath &object_path, const VariantMapMap &interfaces_and_properties);
    void slotInterfacesRemoved(const QDBusObjectPath &object_path, const QStringList &interfaces);
    void slotPropertiesChanged(const QString &ifaceName, const QVariantMap &changedProps, const QStringList &invalidatedProps);

private:
    void initInterfaces();
    QString introspect() const;
    void checkCache(const QString &key) const;
    void cacheProperty(const QString &key, const QVariant &value) const;

    // NOTE: make sure to insert items only through cacheProperty
    mutable QVariantMap m_propertyCache;
    QStringList m_interfaces;
    QString m_udi;

    static QThreadStorage<QMap<QString, DeviceBackend *>> s_backends;
};

} /* namespace UDisks2 */
} /* namespace Backends */
} /* namespace Solid */

#endif /* UDISKSDEVICEBACKEND_H */
