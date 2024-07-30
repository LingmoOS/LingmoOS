/*
    SPDX-FileCopyrightText: 2013-2018 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kcmidentitymodel.h"
#include "networkmodel.h"
#include "uiutils.h"

#include <NetworkManagerQt/Settings>

#include <QFont>
#include <QIcon>

#include <KLocalizedString>

KcmIdentityModel::KcmIdentityModel(QObject *parent)
    : QIdentityProxyModel(parent)
{
    auto baseModel = new NetworkModel(this);
    setSourceModel(baseModel);
}

KcmIdentityModel::~KcmIdentityModel() = default;

Qt::ItemFlags KcmIdentityModel::flags(const QModelIndex &index) const
{
    const QModelIndex mappedProxyIndex = index.sibling(index.row(), 0);
    return QIdentityProxyModel::flags(mappedProxyIndex) | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int KcmIdentityModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 3;
}

QHash<int, QByteArray> KcmIdentityModel::roleNames() const
{
    QHash<int, QByteArray> roles = QIdentityProxyModel::roleNames();
    roles[KcmConnectionIconRole] = "KcmConnectionIcon";
    roles[KcmConnectionTypeRole] = "KcmConnectionType";
    roles[KcmVpnConnectionExportable] = "KcmVpnConnectionExportable";

    return roles;
}

QVariant KcmIdentityModel::data(const QModelIndex &index, int role) const
{
    const QModelIndex sourceIndex = sourceModel()->index(index.row(), 0);
    NetworkManager::ConnectionSettings::ConnectionType type =
        static_cast<NetworkManager::ConnectionSettings::ConnectionType>(sourceModel()->data(sourceIndex, NetworkModel::TypeRole).toInt());

    NetworkManager::ConnectionSettings::Ptr settings;
    NetworkManager::VpnSetting::Ptr vpnSetting;
    if (type == NetworkManager::ConnectionSettings::Vpn) {
        settings = NetworkManager::findConnection(sourceModel()->data(sourceIndex, NetworkModel::ConnectionPathRole).toString())->settings();
        if (settings) {
            vpnSetting = settings->setting(NetworkManager::Setting::Vpn).staticCast<NetworkManager::VpnSetting>();
        }
    }

    QString tooltip;
    const QString iconName = UiUtils::iconAndTitleForConnectionSettingsType(type, tooltip);

    if (role == KcmConnectionIconRole) {
        return iconName;
    } else if (role == KcmConnectionTypeRole) {
        if (type == NetworkManager::ConnectionSettings::Vpn && vpnSetting) {
            return QStringLiteral("%1 (%2)").arg(tooltip, vpnSetting->serviceType().section(QLatin1Char('.'), -1));
        }
        return tooltip;
    } else if (role == KcmVpnConnectionExportable) {
        if (type == NetworkManager::ConnectionSettings::Vpn && vpnSetting) {
            return (vpnSetting->serviceType().endsWith(QLatin1String("vpnc")) //
                    || vpnSetting->serviceType().endsWith(QLatin1String("openvpn")) //
                    || vpnSetting->serviceType().endsWith(QLatin1String("wireguard")));
        }
        return false;
    } else {
        return sourceModel()->data(index, role);
    }

    return {};
}

QModelIndex KcmIdentityModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return createIndex(row, column);
}

QModelIndex KcmIdentityModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (proxyIndex.column() > 0) {
        return {};
    }

    return QIdentityProxyModel::mapToSource(proxyIndex);
}

#include "moc_kcmidentitymodel.cpp"
