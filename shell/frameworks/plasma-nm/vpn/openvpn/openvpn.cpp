/*
    SPDX-FileCopyrightText: 2008 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2011-2012 Rajeesh K Nambiar <rajeeshknambiar@gmail.com>
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>
    SPDX-FileCopyrightText: 2012-2016 Lamarque V. Souza <lamarque@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "openvpn.h"

#include <QLatin1Char>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QStringBuilder>

#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>
#include <QRegularExpression>
#include <kwidgetsaddons_version.h>

#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/Ipv4Setting>
#include <NetworkManagerQt/VpnSetting>

#include "openvpnauth.h"
#include "openvpnwidget.h"

#include <arpa/inet.h>

#include "nm-openvpn-service.h"

K_PLUGIN_CLASS_WITH_JSON(OpenVpnUiPlugin, "plasmanetworkmanagement_openvpnui.json")

#define AUTH_TAG "auth"
#define AUTH_USER_PASS_TAG "auth-user-pass"
#define CA_TAG "ca"
#define CERT_TAG "cert"
#define CIPHER_TAG "cipher"
#define CLIENT_TAG "client"
#define COMPRESS_TAG "compress"
#define COMP_TAG "comp-lzo"
#define DEV_TAG "dev"
#define FRAGMENT_TAG "fragment"
#define IFCONFIG_TAG "ifconfig"
#define KEY_TAG "key"
#define MSSFIX_TAG "mssfix"
#define PKCS12_TAG "pkcs12"
#define PORT_TAG "port"
#define PROTO_TAG "proto"
#define HTTP_PROXY_TAG "http-proxy"
#define HTTP_PROXY_RETRY_TAG "http-proxy-retry"
#define SOCKS_PROXY_TAG "socks-proxy"
#define SOCKS_PROXY_RETRY_TAG "socks-proxy-retry"
#define REMOTE_TAG "remote"
#define RENEG_SEC_TAG "reneg-sec"
#define RPORT_TAG "rport"
#define SECRET_TAG "secret"
#define TLS_AUTH_TAG "tls-auth"
#define TLS_CRYPT_TAG "tls-crypt"
#define TLS_CLIENT_TAG "tls-client"
#define TLS_REMOTE_TAG "tls-remote"
#define TUNMTU_TAG "tun-mtu"
#define KEY_DIRECTION_TAG "key-direction"

#define BEGIN_KEY_CA_TAG "<ca>"
#define END_KEY_CA_TAG "</ca>"
#define BEGIN_KEY_CERT_TAG "<cert>"
#define END_KEY_CERT_TAG "</cert>"
#define BEGIN_KEY_KEY_TAG "<key>"
#define END_KEY_KEY_TAG "</key>"
#define BEGIN_KEY_SECRET_TAG "<secret>"
#define END_KEY_SECRET_TAG "</secret>"
#define BEGIN_TLS_AUTH_TAG "<tls-auth>"
#define END_TLS_AUTH_TAG "</tls-auth>"
#define BEGIN_TLS_CRYPT_TAG "<tls-crypt>"
#define END_TLS_CRYPT_TAG "</tls-crypt>"

#define PROC_TYPE_TAG "Proc-Type: 4,ENCRYPTED"
#define PKCS8_TAG "-----BEGIN ENCRYPTED PRIVATE KEY-----"

QString localCertPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/networkmanagement/certificates/");
}

QString unQuote(QString &certVal, const QString &fileName)
{
    /* Unquote according to openvpn rules
     * Unquoted filename is returned, and @certVal is modified
     * to the leftover string
     */
    int nextSep;
    QString certFile = certVal.trimmed();
    if (certFile.startsWith('"') || certFile.startsWith('\'')) { // Quoted
        certFile.remove(0, 1); // Remove the starting quote
        nextSep = 0;
        while ((nextSep = certFile.indexOf(QRegularExpression(QStringLiteral("\"|'")), nextSep)) != -1) {
            if (nextSep > 0 && certFile.at(nextSep - 1) != '\\') { // Quote not escaped
                certVal = certFile.right(certFile.length() - nextSep - 1); // Leftover string
                certFile.truncate(nextSep); // Quoted string
                break;
            }
        }
    } else {
        nextSep = certFile.indexOf(QRegularExpression(QStringLiteral("\\s"))); // First whitespace
        if (nextSep != -1) {
            certVal = certFile.right(certFile.length() - nextSep - 1); // Leftover
            certFile = certFile.left(nextSep); // value
        } else {
            certVal.clear();
        }
    }
    certFile.replace("\\\\", "\\"); // Replace '\\' with '\'
    certFile.replace("\\ ", " "); // Replace escaped space with space
    if (QFileInfo(certFile).isRelative()) {
        certFile = QFileInfo(fileName).dir().absolutePath() + QLatin1Char('/') + certFile;
    }
    return certFile;
}

bool isEncrypted(const QString &fileName)
{
    bool encrypted = false;
    // TODO: if is_pkcs12(fileName) return true;
    // NOTE: will have to use SEC_PKCS12DecoderStart and friends from <p12.h>, which will
    //       build a new dependency on nss-devel. See NetworkManager/libnm-util/crypto_nss.c+453

    QFile inFile(fileName);
    if (!inFile.open(QFile::ReadOnly)) {
        return false;
    }
    QTextStream in(&inFile);
    while (!in.atEnd()) {
        const QString line = in.readLine();
        if (!line.isEmpty() && (line.startsWith(PROC_TYPE_TAG) || line.startsWith(PKCS8_TAG))) {
            encrypted = true;
            break;
        }
    }
    inFile.close();
    return encrypted;
}

OpenVpnUiPlugin::OpenVpnUiPlugin(QObject *parent, const QVariantList &)
    : VpnUiPlugin(parent)
{
}

OpenVpnUiPlugin::~OpenVpnUiPlugin() = default;

SettingWidget *OpenVpnUiPlugin::widget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent)
{
    auto wid = new OpenVpnSettingWidget(setting, parent);
    return wid;
}

SettingWidget *OpenVpnUiPlugin::askUser(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent)
{
    return new OpenVpnAuthWidget(setting, hints, parent);
}

QString OpenVpnUiPlugin::suggestedFileName(const NetworkManager::ConnectionSettings::Ptr &connection) const
{
    return connection->id() + QStringLiteral("_openvpn.conf");
}

QStringList OpenVpnUiPlugin::supportedFileExtensions() const
{
    return {QStringLiteral("*.ovpn"), QStringLiteral("*.conf")};
}

VpnUiPlugin::ImportResult OpenVpnUiPlugin::importConnectionSettings(const QString &fileName)
{
    GError *error = nullptr;

    GSList *plugins = nm_vpn_plugin_info_list_load();

    NMVpnPluginInfo *plugin_info = nm_vpn_plugin_info_list_find_by_service(plugins, "org.freedesktop.NetworkManager.openvpn");

    if (!plugin_info) {
        return VpnUiPlugin::ImportResult::fail(i18n("NetworkManager is missing support for OpenVPN"));
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

QString OpenVpnUiPlugin::saveFile(QTextStream &in, const QString &endTag, const QString &connectionName, const QString &fileName)
{
    const QString certificatesDirectory = localCertPath() + connectionName;
    const QString absoluteFilePath = certificatesDirectory + '/' + fileName;
    QFile outFile(absoluteFilePath);

    QDir().mkpath(certificatesDirectory);
    if (!outFile.open(QFile::WriteOnly | QFile::Text)) {
        KMessageBox::information(nullptr, i18n("Error saving file %1: %2", absoluteFilePath, outFile.errorString()));
        return {};
    }

    QTextStream out(&outFile);
    while (!in.atEnd()) {
        const QString line = in.readLine();

        if (line.indexOf(endTag) >= 0) {
            break;
        }

        out << line << "\n";
    }

    outFile.close();
    return absoluteFilePath;
}

QString OpenVpnUiPlugin::tryToCopyToCertificatesDirectory(const QString &connectionName, const QString &sourceFilePath)
{
    const QString certificatesDirectory = localCertPath();
    const QString absoluteFilePath = certificatesDirectory + connectionName + '_' + QFileInfo(sourceFilePath).fileName();

    QFile sourceFile(sourceFilePath);

    QDir().mkpath(certificatesDirectory);
    if (!sourceFile.copy(absoluteFilePath)) {
        KMessageBox::information(nullptr, i18n("Error copying certificate to %1: %2", absoluteFilePath, sourceFile.errorString()));
        return sourceFilePath;
    }

    return absoluteFilePath;
}

VpnUiPlugin::ExportResult OpenVpnUiPlugin::exportConnectionSettings(const NetworkManager::ConnectionSettings::Ptr &connection, const QString &fileName)
{
    QFile expFile(fileName);
    if (!expFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return VpnUiPlugin::ExportResult::fail("Could not open file for writing");
    }

    NMStringMap dataMap;
    NMStringMap secretData;

    NetworkManager::VpnSetting::Ptr vpnSetting = connection->setting(NetworkManager::Setting::Vpn).dynamicCast<NetworkManager::VpnSetting>();
    dataMap = vpnSetting->data();
    secretData = vpnSetting->secrets();

    QString line;
    QString cacert, user_cert, private_key;

    line = QString(CLIENT_TAG) + '\n';
    expFile.write(line.toLatin1());
    line = QString(REMOTE_TAG) + ' ' + dataMap[NM_OPENVPN_KEY_REMOTE]
        + (dataMap[NM_OPENVPN_KEY_PORT].isEmpty() ? "\n" : (' ' + dataMap[NM_OPENVPN_KEY_PORT]) + '\n');
    expFile.write(line.toLatin1());
    if (dataMap[NM_OPENVPN_KEY_CONNECTION_TYPE] == NM_OPENVPN_CONTYPE_TLS //
        || dataMap[NM_OPENVPN_KEY_CONNECTION_TYPE] == NM_OPENVPN_CONTYPE_PASSWORD //
        || dataMap[NM_OPENVPN_KEY_CONNECTION_TYPE] == NM_OPENVPN_CONTYPE_PASSWORD_TLS) {
        if (!dataMap[NM_OPENVPN_KEY_CA].isEmpty()) {
            cacert = dataMap[NM_OPENVPN_KEY_CA];
        }
    }
    if (dataMap[NM_OPENVPN_KEY_CONNECTION_TYPE] == NM_OPENVPN_CONTYPE_TLS //
        || dataMap[NM_OPENVPN_KEY_CONNECTION_TYPE] == NM_OPENVPN_CONTYPE_PASSWORD_TLS) {
        if (!dataMap[NM_OPENVPN_KEY_CERT].isEmpty()) {
            user_cert = dataMap[NM_OPENVPN_KEY_CERT];
        }
        if (!dataMap[NM_OPENVPN_KEY_KEY].isEmpty()) {
            private_key = dataMap[NM_OPENVPN_KEY_KEY];
        }
    }
    // Handle PKCS#12 (all certs are the same file)
    if (!cacert.isEmpty() && !user_cert.isEmpty() && !private_key.isEmpty() && cacert == user_cert && cacert == private_key) {
        line = QString("%1 \"%2\"\n").arg(PKCS12_TAG, cacert);
        expFile.write(line.toLatin1());
    } else {
        if (!cacert.isEmpty()) {
            line = QString("%1 \"%2\"\n").arg(CA_TAG, cacert);
            expFile.write(line.toLatin1());
        }
        if (!user_cert.isEmpty()) {
            line = QString("%1 \"%2\"\n").arg(CERT_TAG, user_cert);
            expFile.write(line.toLatin1());
        }
        if (!private_key.isEmpty()) {
            line = QString("%1 \"%2\"\n").arg(KEY_TAG, private_key);
            expFile.write(line.toLatin1());
        }
    }
    if (dataMap[NM_OPENVPN_KEY_CONNECTION_TYPE] == NM_OPENVPN_CONTYPE_TLS //
        || dataMap[NM_OPENVPN_KEY_CONNECTION_TYPE] == NM_OPENVPN_CONTYPE_STATIC_KEY //
        || dataMap[NM_OPENVPN_KEY_CONNECTION_TYPE] == NM_OPENVPN_CONTYPE_PASSWORD //
        || dataMap[NM_OPENVPN_KEY_CONNECTION_TYPE] == NM_OPENVPN_CONTYPE_PASSWORD_TLS) {
        line = QString(AUTH_USER_PASS_TAG) + '\n';
        expFile.write(line.toLatin1());
        if (!dataMap[NM_OPENVPN_KEY_TLS_REMOTE].isEmpty()) {
            line = QString(TLS_REMOTE_TAG) + " \"" + dataMap[NM_OPENVPN_KEY_TLS_REMOTE] + "\"\n";
            expFile.write(line.toLatin1());
        }
        if (!dataMap[NM_OPENVPN_KEY_TA].isEmpty()) {
            line = QString(TLS_AUTH_TAG) + " \"" + dataMap[NM_OPENVPN_KEY_TA] + '\"'
                + (dataMap[NM_OPENVPN_KEY_TA_DIR].isEmpty() ? "\n" : (' ' + dataMap[NM_OPENVPN_KEY_TA_DIR]) + '\n');
            expFile.write(line.toLatin1());
        }
    }
    if (dataMap[NM_OPENVPN_KEY_CONNECTION_TYPE] == NM_OPENVPN_CONTYPE_STATIC_KEY) {
        line = QString(SECRET_TAG) + " \"" + dataMap[NM_OPENVPN_KEY_STATIC_KEY] + '\"'
            + (dataMap[NM_OPENVPN_KEY_STATIC_KEY_DIRECTION].isEmpty() ? "\n" : (' ' + dataMap[NM_OPENVPN_KEY_STATIC_KEY_DIRECTION]) + '\n');
        expFile.write(line.toLatin1());
    }
    if (dataMap.contains(NM_OPENVPN_KEY_RENEG_SECONDS) && !dataMap[NM_OPENVPN_KEY_RENEG_SECONDS].isEmpty()) {
        line = QString(RENEG_SEC_TAG) + ' ' + dataMap[NM_OPENVPN_KEY_RENEG_SECONDS] + '\n';
        expFile.write(line.toLatin1());
    }
    if (!dataMap[NM_OPENVPN_KEY_CIPHER].isEmpty()) {
        line = QString(CIPHER_TAG) + ' ' + dataMap[NM_OPENVPN_KEY_CIPHER] + '\n';
        expFile.write(line.toLatin1());
    }
    if (dataMap[NM_OPENVPN_KEY_COMP_LZO] == "adaptive") {
        line = QString(COMP_TAG) + " adaptive\n";
        expFile.write(line.toLatin1());
    }
    if (dataMap[NM_OPENVPN_KEY_COMPRESS] == "yes") {
        line = QString(COMPRESS_TAG) + " yes\n";
        expFile.write(line.toLatin1());
    }
    if (dataMap[NM_OPENVPN_KEY_COMPRESS] == "lzo") {
        line = QString(COMPRESS_TAG) + " lzo\n";
        expFile.write(line.toLatin1());
    }
    if (dataMap[NM_OPENVPN_KEY_COMPRESS] == "lz4") {
        line = QString(COMPRESS_TAG) + " lz4\n";
        expFile.write(line.toLatin1());
    }
    if (dataMap[NM_OPENVPN_KEY_COMPRESS] == "lz4-v2") {
        line = QString(COMPRESS_TAG) + " lz4-v2\n";
        expFile.write(line.toLatin1());
    }
    if (dataMap[NM_OPENVPN_KEY_MSSFIX] == "yes") {
        line = QString(MSSFIX_TAG) + '\n';
        expFile.write(line.toLatin1());
    }
    if (!dataMap[NM_OPENVPN_KEY_TUNNEL_MTU].isEmpty()) {
        line = QString(TUNMTU_TAG) + ' ' + dataMap[NM_OPENVPN_KEY_TUNNEL_MTU] + '\n';
        expFile.write(line.toLatin1());
    }
    if (!dataMap[NM_OPENVPN_KEY_FRAGMENT_SIZE].isEmpty()) {
        line = QString(FRAGMENT_TAG) + ' ' + dataMap[NM_OPENVPN_KEY_FRAGMENT_SIZE] + '\n';
        expFile.write(line.toLatin1());
    }
    line = QString(DEV_TAG) + (dataMap[NM_OPENVPN_KEY_TAP_DEV] == "yes" ? " tap\n" : " tun\n");
    expFile.write(line.toLatin1());
    line = QString(PROTO_TAG) + (dataMap[NM_OPENVPN_KEY_PROTO_TCP] == "yes" ? " tcp\n" : " udp\n");
    expFile.write(line.toLatin1());
    // Proxy stuff
    if (!dataMap[NM_OPENVPN_KEY_PROXY_TYPE].isEmpty()) {
        QString proxy_port = dataMap[NM_OPENVPN_KEY_PROXY_PORT];
        if (dataMap[NM_OPENVPN_KEY_PROXY_TYPE] == "http" && !dataMap[NM_OPENVPN_KEY_PROXY_SERVER].isEmpty() && dataMap.contains(NM_OPENVPN_KEY_PROXY_PORT)) {
            if (proxy_port.toInt() == 0) {
                proxy_port = "8080";
            }
            line = QString(HTTP_PROXY_TAG) + ' ' + dataMap[NM_OPENVPN_KEY_PROXY_SERVER] + ' ' + proxy_port
                + (dataMap[NM_OPENVPN_KEY_HTTP_PROXY_USERNAME].isEmpty() ? "\n" : (' ' + fileName + "-httpauthfile") + '\n');
            expFile.write(line.toLatin1());
            if (dataMap[NM_OPENVPN_KEY_PROXY_RETRY] == "yes") {
                line = QString(HTTP_PROXY_RETRY_TAG) + '\n';
                expFile.write(line.toLatin1());
            }
            // If there is a username, need to write an authfile
            if (!dataMap[NM_OPENVPN_KEY_HTTP_PROXY_USERNAME].isEmpty()) {
                QFile authFile(fileName + "-httpauthfile");
                if (authFile.open(QFile::WriteOnly | QFile::Text)) {
                    line = dataMap[NM_OPENVPN_KEY_HTTP_PROXY_USERNAME]
                        + (dataMap[NM_OPENVPN_KEY_HTTP_PROXY_PASSWORD].isEmpty() ? "\n" : (dataMap[NM_OPENVPN_KEY_HTTP_PROXY_PASSWORD] + '\n'));
                    authFile.write(line.toLatin1());
                    authFile.close();
                }
            }
        } else if (dataMap[NM_OPENVPN_KEY_PROXY_TYPE] == "socks" && !dataMap[NM_OPENVPN_KEY_PROXY_SERVER].isEmpty()
                   && dataMap.contains(NM_OPENVPN_KEY_PROXY_PORT)) {
            if (proxy_port.toInt() == 0) {
                proxy_port = "1080";
            }
            line = QString(SOCKS_PROXY_TAG) + dataMap[NM_OPENVPN_KEY_PROXY_SERVER] + ' ' + proxy_port + '\n';
            expFile.write(line.toLatin1());
            if (dataMap[NM_OPENVPN_KEY_PROXY_RETRY] == "yes") {
                line = QString(SOCKS_PROXY_RETRY_TAG) + '\n';
                expFile.write(line.toLatin1());
            }
        }
    }

    NetworkManager::Ipv4Setting::Ptr ipv4Setting = connection->setting(NetworkManager::Setting::Ipv4).dynamicCast<NetworkManager::Ipv4Setting>();
    // Export X-NM-Routes
    if (!ipv4Setting->routes().isEmpty()) {
        QString routes;
        for (const NetworkManager::IpRoute &route : ipv4Setting->routes()) {
            routes += route.ip().toString() % QLatin1Char('/') % QString::number(route.prefixLength()) % QLatin1Char(' ');
        }
        if (!routes.isEmpty()) {
            routes = "X-NM-Routes " + routes.trimmed();
            expFile.write(routes.toLatin1() + '\n');
        }
    }
    // Add hard-coded stuff
    expFile.write(
        "nobind\n"
        "auth-nocache\n"
        "script-security 2\n"
        "persist-key\n"
        "persist-tun\n"
        "user nobody\n"
        "group nobody\n");
    expFile.close();
    return VpnUiPlugin::ExportResult::pass();
}

#include "openvpn.moc"

#include "moc_openvpn.cpp"
