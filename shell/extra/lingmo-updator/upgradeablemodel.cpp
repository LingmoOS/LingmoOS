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

#include "upgradeablemodel.h"

static QString formatByteSize(double size, int precision)
{
    int unit = 0;
    double multiplier = 1024.0;

    while (qAbs(size) >= multiplier && unit < int(8)) {
        size /= multiplier;
        ++unit;
    }

    if (unit == 0) {
        precision = 0;
    }

    QString numString = QString::number(size, 'f', precision);

    switch (unit) {
    case 0:
        return QString("%1 B").arg(numString);
    case 1:
        return QString("%1 KB").arg(numString);
    case 2:
        return QString("%1 MB").arg(numString);
    case 3:
        return QString("%1 GB").arg(numString);
    case 4:
        return QString("%1 TB").arg(numString);
    case 5:
        return QString("%1 PB").arg(numString);
    case 6:
        return QString("%1 EB").arg(numString);
    case 7:
        return QString("%1 ZB").arg(numString);
    case 8:
        return QString("%1 YB").arg(numString);
    default:
        return QString();
    }

    return QString();
}

UpgradeableModel *UpgradeableModel::self()
{
    static UpgradeableModel s;
    return &s;
}

UpgradeableModel::UpgradeableModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QHash<int, QByteArray> UpgradeableModel::roleNames() const
{
    static QHash<int, QByteArray> s_roles;

    if (s_roles.isEmpty()) {
        s_roles.insert(UpgradeableModel::NameRole, QByteArrayLiteral("name"));
        s_roles.insert(UpgradeableModel::VersionRole, QByteArrayLiteral("version"));
        s_roles.insert(UpgradeableModel::DownloadSize, QByteArrayLiteral("downloadSize"));
    }

    return s_roles;
}

int UpgradeableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_packages.count();
}

QVariant UpgradeableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    PackageInfo info = m_packages.at(index.row());

    switch (role) {
    case UpgradeableModel::NameRole:
        return info.name;
    case UpgradeableModel::VersionRole:
        return info.version;
    case UpgradeableModel::DownloadSize:
        return formatByteSize(info.downloadSize, 1);
    default:
        break;
    }

    // FIXME: Implement me!
    return QVariant();
}

void UpgradeableModel::addPackage(const QString &name, const QString &version, quint64 downloadSize)
{
    beginInsertRows(QModelIndex(), 0, 0);
    PackageInfo info;
    info.name = name;
    info.version = version;
    info.downloadSize = downloadSize;
    m_packages.prepend(info);
    endInsertRows();
}
