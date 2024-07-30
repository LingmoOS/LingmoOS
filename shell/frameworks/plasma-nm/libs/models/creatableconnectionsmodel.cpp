/*
    SPDX-FileCopyrightText: 2016-2018 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "creatableconnectionsmodel.h"
#include <KLocalizedString>
#include <KPluginMetaData>
#include <NetworkManagerQt/Manager>

CreatableConnectionItem::CreatableConnectionItem(const QString &typeName,
                                                 const QString &typeSection,
                                                 const QString &description,
                                                 const QString &icon,
                                                 NetworkManager::ConnectionSettings::ConnectionType type,
                                                 const QString &vpnType,
                                                 const QString &specificType,
                                                 bool shared,
                                                 QObject *parent)
    : QObject(parent)
    , m_shared(shared)
    , m_connectionType(type)
    , m_description(description)
    , m_icon(icon)
    , m_specificType(specificType)
    , m_typeName(typeName)
    , m_typeSection(typeSection)
    , m_vpnType(vpnType)
{
}

CreatableConnectionItem::CreatableConnectionItem(QObject *parent)
    : QObject(parent)
{
}

CreatableConnectionItem::~CreatableConnectionItem() = default;
NetworkManager::ConnectionSettings::ConnectionType CreatableConnectionItem::connectionType() const
{
    return m_connectionType;
}

void CreatableConnectionItem::setConnectionType(NetworkManager::ConnectionSettings::ConnectionType type)
{
    m_connectionType = type;
}

QString CreatableConnectionItem::description() const
{
    return m_description;
}

void CreatableConnectionItem::setDescription(const QString &description)
{
    m_description = description;
}

QString CreatableConnectionItem::icon() const
{
    return m_icon;
}

void CreatableConnectionItem::setIcon(const QString &icon)
{
    m_icon = icon;
}

bool CreatableConnectionItem::shared() const
{
    return m_shared;
}

void CreatableConnectionItem::setShared(bool shared)
{
    m_shared = shared;
}

QString CreatableConnectionItem::specificType() const
{
    return m_specificType;
}

void CreatableConnectionItem::setSpecificType(const QString &specificType)
{
    m_specificType = specificType;
}

QString CreatableConnectionItem::typeName() const
{
    return m_typeName;
}

void CreatableConnectionItem::setTypeName(const QString &typeName)
{
    m_typeName = typeName;
}

QString CreatableConnectionItem::typeSection() const
{
    return m_typeSection;
}

void CreatableConnectionItem::setTypeSection(const QString &typeSection)
{
    m_typeSection = typeSection;
}

QString CreatableConnectionItem::vpnType() const
{
    return m_vpnType;
}

void CreatableConnectionItem::setVpnType(const QString &vpnType)
{
    m_vpnType = vpnType;
}

CreatableConnectionsModel::CreatableConnectionsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    populateModel();
}

void CreatableConnectionsModel::populateModel()
{
    beginResetModel();

    if (!m_list.isEmpty()) {
        qDeleteAll(m_list);
        m_list.clear();
    }

    CreatableConnectionItem *connectionItem{nullptr};
    connectionItem = new CreatableConnectionItem(i18n("DSL"),
                                                 i18n("Hardware based connections"),
                                                 i18n("Some DSL description"),
                                                 QStringLiteral("network-modem"),
                                                 NetworkManager::ConnectionSettings::Pppoe);
    m_list << connectionItem;

    connectionItem = new CreatableConnectionItem(i18n("Infiniband"),
                                                 i18n("Hardware based connections"),
                                                 i18n("Some infiniband description"),
                                                 QStringLiteral("network-wired"),
                                                 NetworkManager::ConnectionSettings::Infiniband);
    m_list << connectionItem;

    connectionItem = new CreatableConnectionItem(i18n("Mobile Broadband"),
                                                 i18n("Hardware based connections"),
                                                 i18n("Some mobile broadband description"),
                                                 QStringLiteral("smartphone"),
                                                 NetworkManager::ConnectionSettings::Gsm);
    m_list << connectionItem;

    connectionItem = new CreatableConnectionItem(i18n("Wired Ethernet"),
                                                 i18n("Hardware based connections"),
                                                 i18n("Some wired ethernet description"),
                                                 QStringLiteral("network-wired"),
                                                 NetworkManager::ConnectionSettings::Wired);
    m_list << connectionItem;

    connectionItem = new CreatableConnectionItem(i18n("Wired Ethernet (shared)"),
                                                 i18n("Hardware based connections"),
                                                 i18n("Some wired ethernet description"),
                                                 QStringLiteral("network-wired"),
                                                 NetworkManager::ConnectionSettings::Wired,
                                                 QString(),
                                                 QString(),
                                                 true); // VpnType and SpecificType are empty
    m_list << connectionItem;

    connectionItem = new CreatableConnectionItem(i18n("Wi-Fi"),
                                                 i18n("Hardware based connections"),
                                                 i18n("Some wi-fi description"),
                                                 QStringLiteral("network-wireless"),
                                                 NetworkManager::ConnectionSettings::Wireless);
    m_list << connectionItem;

    connectionItem = new CreatableConnectionItem(i18n("Wi-Fi (shared)"),
                                                 i18n("Hardware based connections"),
                                                 i18n("Some wi-fi description"),
                                                 QStringLiteral("network-wireless"),
                                                 NetworkManager::ConnectionSettings::Wireless,
                                                 QString(),
                                                 QString(),
                                                 true); // VpnType and SpecificType are empty
    m_list << connectionItem;

    connectionItem = new CreatableConnectionItem(i18n("Bond"),
                                                 i18n("Virtual connections"),
                                                 i18n("Some bond description"),
                                                 QStringLiteral("network-wired"),
                                                 NetworkManager::ConnectionSettings::Bond,
                                                 QString(),
                                                 QString(),
                                                 true); // VpnType and SpecificType are empty
    m_list << connectionItem;

    connectionItem = new CreatableConnectionItem(i18n("Bridge"),
                                                 i18n("Virtual connections"),
                                                 i18n("Some bond description"),
                                                 QStringLiteral("network-wired"),
                                                 NetworkManager::ConnectionSettings::Bridge,
                                                 QString(),
                                                 QString(),
                                                 true); // VpnType and SpecificType are empty
    m_list << connectionItem;

    connectionItem = new CreatableConnectionItem(i18n("Team"),
                                                 i18n("Virtual connections"),
                                                 i18n("Some team description"),
                                                 QStringLiteral("network-wired"),
                                                 NetworkManager::ConnectionSettings::Team,
                                                 QString(),
                                                 QString(),
                                                 true); // VpnType and SpecificType are empty
    m_list << connectionItem;

    connectionItem = new CreatableConnectionItem(i18n("Vlan"),
                                                 i18n("Virtual connections"),
                                                 i18n("Some vlan description"),
                                                 QStringLiteral("network-wired"),
                                                 NetworkManager::ConnectionSettings::Vlan,
                                                 QString(),
                                                 QString(),
                                                 true); // VpnType and SpecificType are empty
    m_list << connectionItem;

    QList<KPluginMetaData> plugins = KPluginMetaData::findPlugins(QStringLiteral("plasma/network/vpn"));

    std::sort(plugins.begin(), plugins.end(), [](const auto &left, const auto &right) {
        return QString::localeAwareCompare(left.name(), right.name()) <= 0;
    });

    for (const auto &service : std::as_const(plugins)) {
        const QString vpnType = service.value(QStringLiteral("X-NetworkManager-Services"));
        const QString vpnSubType = service.value(QStringLiteral("X-NetworkManager-Services-Subtype"));
        const QString vpnDescription = service.description();

        connectionItem = new CreatableConnectionItem(service.name(),
                                                     i18n("VPN connections"),
                                                     vpnDescription,
                                                     QStringLiteral("network-vpn"),
                                                     NetworkManager::ConnectionSettings::Vpn,
                                                     vpnType,
                                                     vpnSubType,
                                                     false);
        m_list << connectionItem;
    }

    // WireGuard changed from VPN plugin to primary device in version 1.16 of NetworkManager
    if (NetworkManager::checkVersion(1, 16, 0)) {
        connectionItem = new CreatableConnectionItem(i18n("WireGuard"),
                                                     i18n("VPN connections"),
                                                     i18n("WireGuard"),
                                                     QStringLiteral("network-vpn"),
                                                     NetworkManager::ConnectionSettings::WireGuard,
                                                     QStringLiteral("WireGuard"),
                                                     QString(),
                                                     true); // VpnType and SpecificType are empty
        m_list << connectionItem;
    }

    // Placeholder for VPN import
    connectionItem = new CreatableConnectionItem(i18n("Import VPN connection…"),
                                                 i18n("Other"),
                                                 i18n("Import a saved configuration file"),
                                                 QStringLiteral("document-import"),
                                                 NetworkManager::ConnectionSettings::Vpn,
                                                 QStringLiteral("imported"),
                                                 QStringLiteral("imported"),
                                                 false);
    m_list << connectionItem;
    endResetModel();
}

CreatableConnectionsModel::~CreatableConnectionsModel() = default;

QVariant CreatableConnectionsModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();

    if (row >= 0 && row < m_list.count()) {
        CreatableConnectionItem *item = m_list.at(row);

        switch (role) {
        case ConnectionDescription:
            return item->description();
        case ConnectionIcon:
            return item->icon();
        case ConnectionSpeficType:
            return item->specificType();
        case ConnectionShared:
            return item->shared();
        case ConnectionType:
            return item->connectionType();
        case ConnectionTypeName:
            return item->typeName();
        case ConnectionTypeSection:
            return item->typeSection();
        case ConnectionVpnType:
            return item->vpnType();
        default:
            break;
        }
    }

    return {};
}

QHash<int, QByteArray> CreatableConnectionsModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles[ConnectionDescription] = "ConnectionDescription";
    roles[ConnectionIcon] = "ConnectionIcon";
    roles[ConnectionSpeficType] = "ConnectionSpecificType";
    roles[ConnectionShared] = "ConnectionShared";
    roles[ConnectionType] = "ConnectionType";
    roles[ConnectionTypeName] = "ConnectionTypeName";
    roles[ConnectionTypeSection] = "ConnectionTypeSection";
    roles[ConnectionVpnType] = "ConnectionVpnType";

    return roles;
}

int CreatableConnectionsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_list.count();
}

#include "moc_creatableconnectionsmodel.cpp"
