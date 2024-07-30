// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#pragma once

#include <QByteArray>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QtDBus>

#include "org.freedesktop.DBus.ObjectManager.h" // needed for typedefs

class DeviceModel : public QAbstractListModel
{
    Q_OBJECT
    /// Whether kded is available and connected
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)
    /// Whether we are waiting for GetManagedObjects (i.e. initial listing)
    Q_PROPERTY(bool waiting READ waiting NOTIFY waitingChanged)
public:
    enum ItemRole {
        ObjectRole = Qt::UserRole + 1,
    };

    QList<QObject *> m_objects;

    DeviceModel(QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const final;

    int rowCount(const QModelIndex &parent = QModelIndex()) const final;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    Q_SCRIPTABLE int role(const QByteArray &roleName) const;

    bool valid() const;
    bool waiting() const;

Q_SIGNALS:
    void validChanged();
    void waitingChanged();

private Q_SLOTS:
    void addObject(const QDBusObjectPath &dbusPath, const KDBusObjectManagerInterfacePropertiesMap &interfacePropertyMap);
    void removeObject(const QDBusObjectPath &dbusPath);
    void reset();
    void reload();

private:
    void initRoleNames(QObject *object);

    QHash<int, QByteArray> m_roles;
    QHash<int, QByteArray> m_objectPoperties;
    QHash<int, int> m_signalIndexToProperties;

    OrgFreedesktopDBusObjectManagerInterface *m_iface = nullptr;
    QDBusPendingCallWatcher *m_getManagedObjectsWatcher = nullptr;
};

Q_DECLARE_METATYPE(DeviceModel *)
