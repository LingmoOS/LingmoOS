// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <memory>

#include "base/signal.h"
#include "shell/shell.h"

namespace Lingmo::base::dbus {
class Connection;
}

namespace Lingmo::bluez {
class Manager;
}

namespace Lingmo::Networkmanager {
class Client;
}

namespace Lingmo::Shell {

class BluetoothPairingDialog;
class NetworkSecretAgent;
class PairingAgent;
class VpnPasswordDialog;
class WirelessPasswordDialog;

// The session's secret agents and their prompts: NetworkManager's secret agent
// (org.lingmo.network, the keyring behind it, and org.lingmo.SecretAgent
// for passwords asked before a network is added) with the Wi-Fi and VPN
// password dialogs, and BlueZ's default pairing agent with the pairing dialog.
// Each dialog is a window of its own, made while its request is up.
class SecretPrompts final : public Component
{
public:
    // `network` is NetworkManager on the system bus, `session` the session
    // bus (org.lingmo.SecretAgent, the keyring), `bluetooth` BlueZ's
    // manager; any may be null.
    SecretPrompts(Shell *shell, networkmanager::Client *network, base::dbus::Connection *session,
                  std::shared_ptr<bluez::Manager> bluetooth);
    // On the session's NetworkManager, BlueZ and session bus.
    static std::unique_ptr<SecretPrompts> create(Shell *shell);
    ~SecretPrompts() override;

    // Becomes the session's agents: registers with NetworkManager, serves
    // org.lingmo.SecretAgent and takes BlueZ's default agent. Not for a
    // development instance, which would take the session's prompts.
    void start();

    NetworkSecretAgent *networkAgent() const { return m_agent.get(); }
    WirelessPasswordDialog *wirelessDialog() const { return m_wireless.get(); }
    VpnPasswordDialog *vpnDialog() const { return m_vpn.get(); }
    BluetoothPairingDialog *pairingDialog() const { return m_pairing.get(); }

private:
    void networkRequestChanged();
    void pairingChanged();
    void retranslate();

    Shell *m_shell;
    networkmanager::Client *m_network;
    base::dbus::Connection *m_session;
    std::shared_ptr<bluez::Manager> m_bluetooth;
    std::unique_ptr<NetworkSecretAgent> m_agent;
    std::unique_ptr<WirelessPasswordDialog> m_wireless;
    std::unique_ptr<VpnPasswordDialog> m_vpn;
    std::unique_ptr<PairingAgent> m_pairingAgent;
    std::unique_ptr<BluetoothPairingDialog> m_pairing;
    base::Subscription m_requestSubscription;
    base::Subscription m_pairingSubscription;
    base::Subscription m_errorSubscription;
    base::Subscription m_languageChanged;
    // create()'s: the session's NetworkManager and BlueZ, taken by start().
    bool m_systemServices = false;
};

} // namespace Lingmo::Shell

