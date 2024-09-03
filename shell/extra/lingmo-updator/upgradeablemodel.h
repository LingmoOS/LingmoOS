/*
 * Copyright (C) 2024 LingmoOS Team.
 *
 * Author:     Kate Leet <kate@lingmoos.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UPGRADEABLEMODEL_H
#define UPGRADEABLEMODEL_H

#include <QAbstractListModel>

class UpgradeableModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        VersionRole,
        DownloadSize
    };
    Q_ENUM(Roles)

    struct PackageInfo {
        QString name;
        QString version;
        quint64 downloadSize;
    };

    static UpgradeableModel *self();
    explicit UpgradeableModel(QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void addPackage(const QString &name, const QString &version, quint64 downloadSize);

private:
    QList<PackageInfo> m_packages;
};

#endif // UPGRADEABLEMODEL_H
