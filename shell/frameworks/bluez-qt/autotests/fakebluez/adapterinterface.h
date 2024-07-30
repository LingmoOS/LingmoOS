/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef ADAPTERINTERFACE_H
#define ADAPTERINTERFACE_H

#include "object.h"

#include <QDBusAbstractAdaptor>

class QDBusObjectPath;

class GattManagerInterface;
class LEAdvertisingManagerInterface;
class MediaInterface;

class AdapterObject : public QObject
{
public:
    explicit AdapterObject(const QDBusObjectPath &path, QObject *parent = nullptr);
};

class AdapterInterface : public QDBusAbstractAdaptor, public Object
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.Adapter1")
    Q_PROPERTY(QString Address READ address)
    Q_PROPERTY(QString Name READ name)
    Q_PROPERTY(QString Alias READ alias WRITE setAlias)
    Q_PROPERTY(quint32 Class READ adapterClass)
    Q_PROPERTY(bool Powered READ powered WRITE setPowered)
    Q_PROPERTY(bool Discoverable READ discoverable WRITE setDiscoverable)
    Q_PROPERTY(bool Pairable READ pairable WRITE setPairable)
    Q_PROPERTY(quint32 PairableTimeout READ pairableTimeout WRITE setPairableTimeout)
    Q_PROPERTY(quint32 DiscoverableTimeout READ discoverableTimeout WRITE setDiscoverableTimeout)
    Q_PROPERTY(bool Discovering READ discovering)
    Q_PROPERTY(QStringList UUIDs READ uuids)
    Q_PROPERTY(QString Modalias READ modalias)

public:
    explicit AdapterInterface(const QDBusObjectPath &path, const QVariantMap &properties, QObject *parent = nullptr);

    QString address() const;

    QString name() const;

    QString alias() const;
    void setAlias(const QString &alias);

    quint32 adapterClass() const;

    bool powered() const;
    void setPowered(bool powered);

    bool discoverable() const;
    void setDiscoverable(bool discoverable);

    bool pairable() const;
    void setPairable(bool pairable);

    quint32 pairableTimeout() const;
    void setPairableTimeout(quint32 timeout);

    quint32 discoverableTimeout() const;
    void setDiscoverableTimeout(quint32 timeout);

    bool discovering() const;

    QStringList uuids() const;

    QString modalias() const;

    MediaInterface *media() const;

    LEAdvertisingManagerInterface *leAdvertisingManager() const;

    GattManagerInterface *gattManager() const;

public Q_SLOTS:
    void StartDiscovery();
    void StopDiscovery();
    void RemoveDevice(const QDBusObjectPath &device);
    void SetDiscoveryFilter(const QVariantMap &filter);
    QStringList GetDiscoveryFilters();

private Q_SLOTS:
    void resetPairable();
    void resetDiscoverable();

private:
    MediaInterface *m_media;
    LEAdvertisingManagerInterface *m_leAdvertisingManager;
    GattManagerInterface *m_gattManager;
};

#endif // ADAPTERINTERFACE_H
