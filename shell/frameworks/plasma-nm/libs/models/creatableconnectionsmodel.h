/*
    SPDX-FileCopyrightText: 2016-2018 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_CREATABLE_CONNECTIONS_MODEL_H
#define PLASMA_NM_CREATABLE_CONNECTIONS_MODEL_H

#include "plasmanm_internal_export.h"

#include <NetworkManagerQt/ConnectionSettings>
#include <QAbstractListModel>

#include <qqmlregistration.h>

class PLASMANM_INTERNAL_EXPORT CreatableConnectionItem : public QObject
{
    Q_OBJECT
public:
    explicit CreatableConnectionItem(const QString &typeName,
                                     const QString &typeSection, // Visible properties
                                     const QString &description,
                                     const QString &icon, // Visible properties
                                     NetworkManager::ConnectionSettings::ConnectionType type, // Properties needed for creating
                                     const QString &vpnType = QString(), // Properties needed for creating
                                     const QString &specificType = QString(), // Properties needed for creating
                                     bool shared = false, // Properties needed for creating
                                     QObject *parent = nullptr);
    explicit CreatableConnectionItem(QObject *parent = nullptr);
    ~CreatableConnectionItem() override;

    NetworkManager::ConnectionSettings::ConnectionType connectionType() const;
    void setConnectionType(NetworkManager::ConnectionSettings::ConnectionType type);

    QString description() const;
    void setDescription(const QString &description);

    QString icon() const;
    void setIcon(const QString &icon);

    QString specificType() const;
    void setSpecificType(const QString &specificType);

    bool shared() const;
    void setShared(bool shared);

    QString typeName() const;
    void setTypeName(const QString &typeName);

    QString typeSection() const;
    void setTypeSection(const QString &typeSection);

    QString vpnType() const;
    void setVpnType(const QString &vpnType);

private:
    bool m_shared;
    NetworkManager::ConnectionSettings::ConnectionType m_connectionType;
    QString m_description;
    QString m_icon;
    QString m_specificType;
    QString m_typeName;
    QString m_typeSection;
    QString m_vpnType;
};

class PLASMANM_INTERNAL_EXPORT CreatableConnectionsModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit CreatableConnectionsModel(QObject *parent = nullptr);
    ~CreatableConnectionsModel() override;

    enum ItemRole {
        ConnectionDescription = Qt::UserRole + 1,
        ConnectionIcon,
        ConnectionSpeficType,
        ConnectionShared,
        ConnectionType,
        ConnectionTypeName,
        ConnectionTypeSection,
        ConnectionVpnType,
    };

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    void populateModel();
    QList<CreatableConnectionItem *> m_list;
};

#endif // PLASMA_NM_CREATABLE_CONNECTIONS_MODEL_H
