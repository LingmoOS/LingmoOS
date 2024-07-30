/*
    SPDX-FileCopyrightText: 2008 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2011-2012 Rajeesh K Nambiar <rajeeshknambiar@gmail.com>
    SPDX-FileCopyrightText: 2011-2012 Lamarque V. Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "vpnc.h"
#include "plasma_nm_vpnc.h"

#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

#include "nm-vpnc-service.h"
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>
#include <KSharedConfig>

#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/Ipv4Setting>
#include <NetworkManagerQt/VpnSetting>

#include <arpa/inet.h>

#include "vpncauth.h"
#include "vpncwidget.h"

VpncUiPluginPrivate::VpncUiPluginPrivate()
{
    decryptedPasswd.clear();
    ciscoDecrypt = nullptr;
}

VpncUiPluginPrivate::~VpncUiPluginPrivate() = default;

QString VpncUiPluginPrivate::readStringKeyValue(const KConfigGroup &configGroup, const QString &key)
{
    const QString retValue = configGroup.readEntry(key);
    if (retValue.isEmpty()) {
        // String key can also start with "!" in CISCO pcf file.
        return configGroup.readEntry('!' + key);
    } else {
        return retValue;
    }
}

void VpncUiPluginPrivate::gotCiscoDecryptOutput()
{
    QByteArray output = ciscoDecrypt->readAll();
    if (!output.isEmpty()) {
        const QList<QByteArray> lines = output.split('\n');
        if (!lines.isEmpty()) {
            decryptedPasswd = QString::fromUtf8(lines.first());
        }
    }
}

void VpncUiPluginPrivate::ciscoDecryptFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode || exitStatus != QProcess::NormalExit) {
        decryptedPasswd.clear();
    }
}

void VpncUiPluginPrivate::ciscoDecryptError(QProcess::ProcessError pError)
{
    if (!pError) {
        qCWarning(PLASMA_NM_VPNC_LOG) << "Error in executing cisco-decrypt";
        KMessageBox::error(nullptr, i18n("Error decrypting the obfuscated password"), i18n("Error"), KMessageBox::Notify);
    }
    decryptedPasswd.clear();
}

#define NM_VPNC_LOCAL_PORT_DEFAULT 500

K_PLUGIN_CLASS_WITH_JSON(VpncUiPlugin, "plasmanetworkmanagement_vpncui.json")

VpncUiPlugin::VpncUiPlugin(QObject *parent, const QVariantList &)
    : VpnUiPlugin(parent)
{
}

VpncUiPlugin::~VpncUiPlugin() = default;

SettingWidget *VpncUiPlugin::widget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent)
{
    return new VpncWidget(setting, parent);
}

SettingWidget *VpncUiPlugin::askUser(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent)
{
    return new VpncAuthDialog(setting, hints, parent);
}

QString VpncUiPlugin::suggestedFileName(const NetworkManager::ConnectionSettings::Ptr &connection) const
{
    return connection->id() + QStringLiteral(".pcf");
}

QStringList VpncUiPlugin::supportedFileExtensions() const
{
    return {QStringLiteral("*.pcf")};
}

VpnUiPlugin::ImportResult VpncUiPlugin::importConnectionSettings(const QString &fileName)
{
    GError *error = nullptr;

    GSList *plugins = nm_vpn_plugin_info_list_load();

    NMVpnPluginInfo *plugin_info = nm_vpn_plugin_info_list_find_by_service(plugins, "org.freedesktop.NetworkManager.vpnc");

    if (!plugin_info) {
        return VpnUiPlugin::ImportResult::fail(i18n("NetworkManager is missing support for Cisco Compatible VPN"));
    }

    NMVpnEditorPlugin *plugin = nm_vpn_plugin_info_load_editor_plugin(plugin_info, &error);

    NMConnection *connection = nm_vpn_editor_plugin_import(plugin, fileName.toUtf8().constData(), &error);

    if (!connection) {
        const QString errorMessage = QString::fromUtf8(error->message);
        g_error_free(error);

        return VpnUiPlugin::ImportResult::fail(errorMessage);
    }

    return VpnUiPlugin::ImportResult::pass(connection);
}

VpncUiPlugin::ExportResult VpncUiPlugin::exportConnectionSettings(const NetworkManager::ConnectionSettings::Ptr &connection, const QString &fileName)
{
    NMStringMap data;
    NMStringMap secretData;

    NetworkManager::VpnSetting::Ptr vpnSetting = connection->setting(NetworkManager::Setting::Vpn).dynamicCast<NetworkManager::VpnSetting>();
    data = vpnSetting->data();
    secretData = vpnSetting->secrets();

    KSharedConfig::Ptr config = KSharedConfig::openConfig(fileName);
    if (!config) {
        return VpnUiPlugin::ExportResult::fail(i18n("%1: file could not be created", fileName));
    }
    KConfigGroup cg(config, "main");

    cg.writeEntry("Description", connection->id());
    cg.writeEntry("Host", data.value(NM_VPNC_KEY_GATEWAY));
    if (data.value(NM_VPNC_KEY_AUTHMODE) == QLatin1String("hybrid")) {
        cg.writeEntry("AuthType", "5");
    } else {
        cg.writeEntry("AuthType", "1");
    }
    cg.writeEntry("GroupName", data.value(NM_VPNC_KEY_ID));
    cg.writeEntry("GroupPwd", secretData.value(NM_VPNC_KEY_SECRET));
    cg.writeEntry("UserPassword", secretData.value(NM_VPNC_KEY_XAUTH_PASSWORD));
    cg.writeEntry("enc_GroupPwd", "");
    cg.writeEntry("enc_UserPassword", "");
    if ((NetworkManager::Setting::SecretFlags)data.value(NM_VPNC_KEY_XAUTH_PASSWORD "-flags").toInt() & NetworkManager::Setting::NotSaved) {
        cg.writeEntry("SaveUserPassword", "0");
    }
    if ((NetworkManager::Setting::SecretFlags)data.value(NM_VPNC_KEY_XAUTH_PASSWORD "-flags").toInt() & NetworkManager::Setting::AgentOwned) {
        cg.writeEntry("SaveUserPassword", "1");
    }
    if ((NetworkManager::Setting::SecretFlags)data.value(NM_VPNC_KEY_XAUTH_PASSWORD "-flags").toInt() & NetworkManager::Setting::NotRequired) {
        cg.writeEntry("SaveUserPassword", "2");
    }
    cg.writeEntry("Username", data.value(NM_VPNC_KEY_XAUTH_USER));
    cg.writeEntry("EnableISPConnect", "0");
    cg.writeEntry("ISPConnectType", "0");
    cg.writeEntry("ISPConnect", "");
    cg.writeEntry("ISPCommand", "");
    cg.writeEntry("EnableBackup", "0");
    cg.writeEntry("BackupServer", "");
    cg.writeEntry("CertStore", "0");
    cg.writeEntry("CertName", "");
    cg.writeEntry("CertPath", "");
    cg.writeEntry("CertSubjectName", "");
    cg.writeEntry("CertSerialHash", "");
    cg.writeEntry("DHGroup", data.value(NM_VPNC_KEY_DHGROUP));
    cg.writeEntry("ForceKeepAlives", "0");
    cg.writeEntry("NTDomain", data.value(NM_VPNC_KEY_DOMAIN));
    cg.writeEntry("EnableMSLogon", "0");
    cg.writeEntry("MSLogonType", "0");
    cg.writeEntry("TunnelingMode", "0");
    cg.writeEntry("TcpTunnelingPort", "10000");
    cg.writeEntry("PeerTimeout", data.value(NM_VPNC_KEY_DPD_IDLE_TIMEOUT));
    cg.writeEntry("EnableLocalLAN", "1");
    cg.writeEntry("SendCertChain", "0");
    cg.writeEntry("VerifyCertDN", "");
    cg.writeEntry("EnableSplitDNS", "1");
    cg.writeEntry("SPPhonebook", "");
    if (data.value(NM_VPNC_KEY_SINGLE_DES) == "yes") {
        cg.writeEntry("SingleDES", "1");
    }
    if (data.value(NM_VPNC_KEY_NAT_TRAVERSAL_MODE) == NM_VPNC_NATT_MODE_CISCO) {
        cg.writeEntry("EnableNat", "1");
    }
    if (data.value(NM_VPNC_KEY_NAT_TRAVERSAL_MODE) == NM_VPNC_NATT_MODE_NATT) {
        cg.writeEntry("EnableNat", "1");
        cg.writeEntry("X-NM-Use-NAT-T", "1");
    }
    if (data.value(NM_VPNC_KEY_NAT_TRAVERSAL_MODE) == NM_VPNC_NATT_MODE_NATT_ALWAYS) {
        cg.writeEntry("EnableNat", "1");
        cg.writeEntry("X-NM-Force-NAT-T", "1");
    }
    // Export X-NM-Routes
    NetworkManager::Ipv4Setting::Ptr ipv4Setting = connection->setting(NetworkManager::Setting::Ipv4).dynamicCast<NetworkManager::Ipv4Setting>();
    if (!ipv4Setting->routes().isEmpty()) {
        QString routes;
        for (const NetworkManager::IpRoute &route : ipv4Setting->routes()) {
            routes += route.ip().toString() + QLatin1Char('/') + QString::number(route.prefixLength()) + QLatin1Char(' ');
        }
        cg.writeEntry("X-NM-Routes", routes.trimmed());
    }

    cg.sync();

    return VpnUiPlugin::ExportResult::pass();
}

#include "vpnc.moc"

#include "moc_vpnc.cpp"
