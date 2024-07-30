/*
 *   SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KCM_ENERGYINFO_BATTERYMODEL_H
#define KCM_ENERGYINFO_BATTERYMODEL_H

#include <QAbstractListModel>
#include <QList>

#include <Solid/Battery>
#include <Solid/Device>

class BatteryModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    explicit BatteryModel(QObject *parent);

    enum Roles {
        BatteryRole = Qt::UserRole,
        UdiRole,
        VendorRole,
        ProductRole,
    };
    Q_ENUM(Roles)

    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void countChanged();

private:
    QList<Solid::Device> m_batteries;
};

#endif // KCM_ENERGYINFO_BATTERYMODEL_H
