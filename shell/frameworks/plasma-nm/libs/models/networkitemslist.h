/*
    SPDX-FileCopyrightText: 2013-2018 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_MODEL_NETWORK_ITEMS_LIST_H
#define PLASMA_NM_MODEL_NETWORK_ITEMS_LIST_H

#include <QAbstractListModel>

#include <NetworkManagerQt/ConnectionSettings>

class NetworkModelItem;

class NetworkItemsList : public QObject
{
    Q_OBJECT
public:
    enum FilterType {
        ActiveConnection,
        Connection,
        Device,
        Name,
        Ssid,
        Uuid,
        Type,
    };

    explicit NetworkItemsList(QObject *parent = nullptr);
    ~NetworkItemsList() override;

    bool contains(const FilterType type, const QString &parameter) const;
    int count() const;
    int indexOf(NetworkModelItem *item) const;
    NetworkModelItem *itemAt(int index) const;
    QList<NetworkModelItem *> items() const;
    QList<NetworkModelItem *> returnItems(const FilterType type, const QString &parameter, const QString &additionalParameter = QString()) const;
    QList<NetworkModelItem *> returnItems(const FilterType type, NetworkManager::ConnectionSettings::ConnectionType typeParameter) const;

    void insertItem(NetworkModelItem *item);
    void removeItem(NetworkModelItem *item);

private:
    QList<NetworkModelItem *> m_items;
};

#endif // PLASMA_NM_MODEL_NETWORK_ITEMS_LIST_H
