/*
    SPDX-FileCopyrightText: 2008 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2013 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "vpnuiplugin.h"

#include <KLocalizedString>
#include <KPluginMetaData>

VpnUiPlugin::VpnUiPlugin(QObject *parent, const QVariantList & /*args*/)
    : QObject(parent)
{
}

VpnUiPlugin::~VpnUiPlugin() = default;

QMessageBox::StandardButtons VpnUiPlugin::suggestedAuthDialogButtons() const
{
    return QMessageBox::Ok | QMessageBox::Cancel;
}

QStringList VpnUiPlugin::supportedFileExtensions() const
{
    return {};
}

KPluginFactory::Result<VpnUiPlugin> VpnUiPlugin::loadPluginForType(QObject *parent, const QString &serviceType)
{
    auto filter = [serviceType](const KPluginMetaData &md) {
        return md.value(QStringLiteral("X-NetworkManager-Services")) == serviceType;
    };

    const QList<KPluginMetaData> offers = KPluginMetaData::findPlugins(QStringLiteral("plasma/network/vpn"), filter);

    if (offers.isEmpty()) {
        KPluginFactory::Result<VpnUiPlugin> result;
        result.errorReason = KPluginFactory::INVALID_PLUGIN;
        result.errorText = QStringLiteral("No VPN plugin found for type %1").arg(serviceType);
        result.errorString = i18n("No VPN plugin found for type %1", serviceType);

        return result;
    }

    return KPluginFactory::instantiatePlugin<VpnUiPlugin>(offers.first(), parent);
}

VpnUiPlugin::ImportResult VpnUiPlugin::importConnectionSettings(const QString &fileName)
{
    Q_UNUSED(fileName);
    return VpnUiPlugin::ImportResult::notImplemented();
}

VpnUiPlugin::ExportResult VpnUiPlugin::exportConnectionSettings(const NetworkManager::ConnectionSettings::Ptr &connection, const QString &fileName)
{
    Q_UNUSED(connection);
    Q_UNUSED(fileName);
    return VpnUiPlugin::ExportResult::notImplemented();
}

VpnUiPlugin::ImportResult::operator bool() const
{
    return m_error == NoError;
}

QString VpnUiPlugin::ImportResult::errorMessage() const
{
    return m_errorMessage;
}

NMConnection *VpnUiPlugin::ImportResult::connection() const
{
    return m_connection;
}

VpnUiPlugin::ImportResult VpnUiPlugin::ImportResult::fail(const QString &errorMessage)
{
    ImportResult result;
    result.m_error = Error;
    result.m_errorMessage = errorMessage;

    return result;
}

VpnUiPlugin::ImportResult VpnUiPlugin::ImportResult::pass(NMConnection *connection)
{
    ImportResult result;
    result.m_connection = connection;

    return result;
}

VpnUiPlugin::ImportResult VpnUiPlugin::ImportResult::notImplemented()
{
    VpnUiPlugin::ImportResult result;
    result.m_error = NotImplemented;
    result.m_errorMessage = i18n("Importing this type of VPN is not implemented");

    return result;
}

VpnUiPlugin::ExportResult::operator bool() const
{
    return m_error == NoError;
}

QString VpnUiPlugin::ExportResult::errorMessage() const
{
    return m_errorMessage;
}

VpnUiPlugin::ExportResult VpnUiPlugin::ExportResult::pass()
{
    return ExportResult();
}

VpnUiPlugin::ExportResult VpnUiPlugin::ExportResult::notImplemented()
{
    ExportResult result;
    result.m_error = NotImplemented;
    result.m_errorMessage = i18n("Not implemented");

    return result;
}

VpnUiPlugin::ExportResult VpnUiPlugin::ExportResult::fail(const QString &errorMessage)
{
    ExportResult result;
    result.m_error = Error;
    result.m_errorMessage = errorMessage;

    return result;
}

#include "moc_vpnuiplugin.cpp"
