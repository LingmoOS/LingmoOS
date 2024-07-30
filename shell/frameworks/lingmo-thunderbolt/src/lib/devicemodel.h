/*
 * SPDX-FileCopyrightText: 2018-2019 Daniel Vrátil <dvratil@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef DEVICEMODEL_H_
#define DEVICEMODEL_H_

#include <QAbstractListModel>

#include "kbolt_export.h"
#include "manager.h"
namespace Bolt
{
class Device;
class KBOLT_EXPORT DeviceModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(Bolt::Manager *manager READ manager WRITE setManager NOTIFY managerChanged)

    /** Whether to show only peripherals or display hosts as well */
    Q_PROPERTY(bool showHosts READ showHosts WRITE setShowHosts NOTIFY showHostsChanged)
public:
    enum Role {
        DeviceRole = Qt::UserRole,
    };

    using QAbstractListModel::QAbstractListModel;
    ~DeviceModel() override = default;

    Manager *manager() const;
    void setManager(Manager *manager);

    bool showHosts() const;
    void setShowHosts(bool showHosts);

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

Q_SIGNALS:
    void managerChanged(Bolt::Manager *manager);
    void showHostsChanged(bool showHosts);

private:
    void populateWithoutReset();

    Manager *mManager = nullptr;
    QList<QSharedPointer<Device>> mDevices;
    bool mShowHosts = true;
};

} // namespace Bolt

#endif
