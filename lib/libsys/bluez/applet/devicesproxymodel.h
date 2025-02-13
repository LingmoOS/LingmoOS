/*
    SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef DEVICESPROXYMODEL_H
#define DEVICESPROXYMODEL_H

#include <BluezQt/DevicesModel>
#include <QSortFilterProxyModel>

class DevicesProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString connectedName READ connectedName NOTIFY connectedNameChanged)
    Q_PROPERTY(QString connectedAdress READ connectedAdress NOTIFY connectedAdressChanged)

public:
    enum AdditionalRoles {
        SectionRole = BluezQt::DevicesModel::LastRole + 10,
        DeviceFullNameRole = BluezQt::DevicesModel::LastRole + 11
    };

    explicit DevicesProxyModel(QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    Q_INVOKABLE QString adapterHciString(const QString &ubi) const;

    QString connectedName(){ return m_connectedName; };
    QString connectedAdress(){ return m_connectedAdress; };

signals:
    void connectedNameChanged(const QString connectedName) const;
    void connectedAdressChanged(const QString connectedAddress) const;

private Q_SLOTS:
    void bluetoothBlockedChanged(bool blocked);

private:
    bool duplicateIndexAddress(const QModelIndex &idx) const;

    mutable QString m_connectedName = "";
    mutable QString m_connectedAdress = "";

    BluezQt::Manager *m_manager;
};

#endif // DEVICESPROXYMODEL_H
