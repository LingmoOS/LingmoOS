/*
    SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <QAbstractListModel>
#include <QList>
#include <Solid/Device>

/**
 * Model of cifs mount entries. Implementing properties
 * as columns rather than roles.
 */
class SmbMountModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum class Role {
        Share = Qt::UserRole + 1,
        Path,
        Accessible,
    };
    Q_ENUM(Role)

    explicit SmbMountModel(QObject *parent = nullptr);
    ~SmbMountModel() override;

    int rowCount(const QModelIndex &parent) const final;
    QVariant data(const QModelIndex &index, int intRole) const final;
    bool hasChildren(const QModelIndex &parent) const final;

    QHash<int, QByteArray> roleNames() const final;

private slots:
    void addDevice(const QString &udi);
    void removeDevice(const QString &udi);
    void reloadData();

private:
    inline QList<Solid::Device>::const_iterator deviceForUdi(const QString &udi) const
    {
        return std::find_if(m_devices.constBegin(), m_devices.constEnd(), [&udi](const Solid::Device &device) {
            return device.udi() == udi;
        });
    }

    QList<Solid::Device> m_devices;
};
