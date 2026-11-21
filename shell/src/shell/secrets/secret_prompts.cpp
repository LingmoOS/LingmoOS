// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/secrets/secret_prompts.h"

#include "base/dbus.h"
#include "lingmo/bluez/manager.h"
#include "lingmo/networkmanager/client.h"
#include "shell/secrets/bluetooth_pairing_dialog.h"
#include "shell/secrets/network_dialogs.h"
#include "shell/secrets/network_secret_agent.h"
#include "ui/application.h"

namespace Lingmo::Shell {

namespace {

class BluezPairingAgent final : public PairingAgent
{
public:
    explicit BluezPairingAgent(bluez::Manager *manager)
        : m_manager(manager)
    {
    }

    std::string pairingKind() const override { return m_manager->pairingKind(); }
    std::string pairingDevice() const override { return m_manager->pairingDevice(); }
    std::string pairingCode() const override { return m_manager->pairingCode(); }
    int pairingEntered() const override { return m_manager->pairingEntered(); }
    std::string errorText() const override { return m_manager->errorText(); }
    void respondPairing(bool accept, const std::string &code) override { m_manager->respondPairing(accept, code); }

private:
    bluez::Manager *m_manager;
};

} // namespace

SecretPrompts::SecretPrompts(Shell *shell, networkmanager::Client *network, base::dbus::Connection *session,
                             std::shared_ptr<bluez::Manager> bluetooth)
    : m_shell(shell)
    , m_network(network)
    , m_session(session)
    , m_bluetooth(std::move(bluetooth))
{
}

std::unique_ptr<SecretPrompts> SecretPrompts::create(Shell *shell)
{
    auto prompts = std::make_unique<SecretPrompts>(shell, nullptr, shell->bus(), nullptr);
    prompts->m_systemServices = true;
    return prompts;
}

SecretPrompts::~SecretPrompts()
{
    m_requestSubscription.reset();
    m_pairingSubscription.reset();
    m_errorSubscription.reset();
    m_languageChanged.reset();
    m_wireless.reset();
    m_vpn.reset();
    m_pairing.reset();
    m_pairingAgent.reset();
    // Unanswered requests fail, and BlueZ forgets the agent.
    m_agent.reset();
    if (m_bluetooth)
        m_bluetooth->setAgentEnabled(false);
}

void SecretPrompts::start()
{
    if (m_agent)
        return;
    // The system bus's services only now: a development instance never starts.
    if (m_systemServices) {
        m_network = networkmanager::Client::instance();
        m_bluetooth = bluez::Manager::shared();
    }

    m_agent = std::make_unique<NetworkSecretAgent>(m_network, m_session);
    m_agent->registerService();
    m_wireless = std::make_unique<WirelessPasswordDialog>(m_shell, m_agent.get());
    m_vpn = std::make_unique<VpnPasswordDialog>(m_shell, m_agent.get());
    // Closing the window declines what it asks.
    const auto declined = [this] {
        if (m_agent->pending())
            m_agent->cancel();
    };
    m_wireless->onDismissed = declined;
    m_vpn->onDismissed = declined;
    m_requestSubscription = m_agent->requestChanged.connect([this] { networkRequestChanged(); });

    if (m_bluetooth) {
        m_pairingAgent = std::make_unique<BluezPairingAgent>(m_bluetooth.get());
        m_pairing = std::make_unique<BluetoothPairingDialog>(m_shell, m_pairingAgent.get());
        m_pairing->onDismissed = [this] {
            if (!m_bluetooth->pairingKind().empty())
                m_bluetooth->cancelPairing();
        };
        m_pairingSubscription = m_bluetooth->pairingChanged.connect([this] { pairingChanged(); });
        m_errorSubscription = m_bluetooth->errorChanged.connect([this] {
            if (m_pairing->isOpen())
                m_pairing->refresh();
        });
        m_bluetooth->setAgentEnabled(true);
    }
    m_languageChanged = ui::Application::instance()->languageChanged.connect([this] { retranslate(); });
    networkRequestChanged();
    pairingChanged();
}

// The prompts stay up with what was typed in them, still answering the same
// requests.
void SecretPrompts::retranslate()
{
    m_agent->retranslate();
    m_wireless->retranslate();
    m_vpn->retranslate();
    if (m_pairing)
        m_pairing->retranslate();
}

void SecretPrompts::networkRequestChanged()
{
    // A VPN asks for its own set of secrets in a window of its own.
    const bool pending = m_agent->pending();
    const bool vpn = m_agent->kind() == NetworkSecretAgent::Kind::VpnSecrets;
    SecretDialog *shown = !pending ? nullptr : vpn ? static_cast<SecretDialog *>(m_vpn.get()) : m_wireless.get();
    for (SecretDialog *dialog : {static_cast<SecretDialog *>(m_wireless.get()), static_cast<SecretDialog *>(m_vpn.get())}) {
        if (dialog != shown)
            dialog->close();
    }
    if (!shown)
        return;
    if (shown->isOpen()) {
        if (vpn)
            m_vpn->requestChanged();
        else
            m_wireless->requestChanged();
        return;
    }
    shown->open();
}

void SecretPrompts::pairingChanged()
{
    if (!m_pairing)
        return;
    if (m_bluetooth->pairingKind().empty()) {
        m_pairing->close();
        return;
    }
    if (m_pairing->isOpen()) {
        m_pairing->refresh();
        return;
    }
    // Left over from whatever the manager did before this request.
    m_bluetooth->clearError();
    m_pairing->open();
}

} // namespace Lingmo::Shell
