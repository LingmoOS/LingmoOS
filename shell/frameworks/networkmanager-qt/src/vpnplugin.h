/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_VPNPLUGIN_H
#define NETWORKMANAGERQT_VPNPLUGIN_H

#include "generictypes.h"
#include "vpnconnection.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QObject>

namespace NetworkManager
{
class VpnPluginPrivate;

class NETWORKMANAGERQT_EXPORT VpnPlugin : public QObject
{
    Q_OBJECT

public:
    enum FailureType {
        LoginFailed,
        ConnectFailed,
        BadIpConfig,
    };

    explicit VpnPlugin(const QString &path, QObject *parent = nullptr);
    ~VpnPlugin() override;

Q_SIGNALS:
    void configChanged(const QVariantMap &configuration);
    void failureChanged(uint reason);
    void ip4ConfigChanged(const QVariantMap &ip4config);
    void ip6ConfigChanged(const QVariantMap &ip6config);
    void loginBannerChanged(const QString &banner);
    void stateChanged(VpnConnection::State state);

protected Q_SLOTS:
    void connect(const NMVariantMapMap &connection);
    void disconnect();
    QString needSecrets(const NMVariantMapMap &connection);
    void setConfig(const QVariantMap &config);
    void setFailure(const QString &reason);
    void setIp4Config(const QVariantMap &config);
    void setIp6Config(const QVariantMap &config);
    void onStateChanged(uint state);

protected:
    VpnPluginPrivate *const d_ptr;

private:
    Q_DECLARE_PRIVATE(VpnPlugin)
};

} // namespace NetworkManager

#endif // NETWORKMANAGERQT_VPNPLUGIN_H
