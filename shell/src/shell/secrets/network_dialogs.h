// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <string>
#include <vector>

#include "shell/secrets/network_secret_agent.h"
#include "shell/secrets/secret_dialog.h"

namespace Lingmo::ui {
class CheckBox;
class TextField;
} // namespace Lingmo::UI

namespace Lingmo::Shell {

// A Wi-Fi, 802.1X or private key
// password for the agent's request, whichever application started the
// connection. Connect stays off until the field holds a valid WEP key or WPA
// passphrase; what can never become one is not typed at all.
class WirelessPasswordDialog final : public SecretDialog
{
public:
    WirelessPasswordDialog(Shell *shell, NetworkSecretAgent *agent);

    // The next request, or another network picked meanwhile, reuses the
    // window: an empty field, the password hidden again.
    void requestChanged();

    static std::string title(NetworkSecretAgent::Kind kind);
    static std::string text(const NetworkSecretAgent &agent);
    // WEP: 5 or 13 characters or 10 or 26 hex digits; WPA: 8 to 63
    // characters or 64 hex digits; anything else: not empty.
    static bool acceptable(NetworkSecretAgent::Kind kind, const std::string &text);
    // Whether more typing could still make `text` acceptable.
    static bool intermediate(NetworkSecretAgent::Kind kind, const std::string &text);

    ui::TextField *field() const { return m_field; }
    ui::CheckBox *showPassword() const { return m_show; }

protected:
    void build(ui::View *frame) override;
    ui::View *initialFocus() const override;
    void accept() override;
    void update() override;
    void retranslateRows() override;

private:
    NetworkSecretAgent *m_agent;
    ui::TextField *m_field = nullptr;
    ui::CheckBox *m_show = nullptr;
};

// One field per secret the VPN's plugin
// needs, such as a password and a one-time code; Connect once all are filled.
class VpnPasswordDialog final : public SecretDialog
{
public:
    VpnPasswordDialog(Shell *shell, NetworkSecretAgent *agent);

    // The next request reuses the window with its own fields.
    void requestChanged();

    static std::string text(const NetworkSecretAgent &agent);

    const std::vector<ui::TextField *> &fields() const { return m_fields; }
    ui::CheckBox *showPassword() const { return m_show; }

protected:
    void build(ui::View *frame) override;
    ui::View *initialFocus() const override;
    void accept() override;
    void update() override;
    void retranslateRows() override;

private:
    void rebuildFields();
    bool complete() const;
    void applyEcho();

    NetworkSecretAgent *m_agent;
    ui::View *m_frame = nullptr;
    std::vector<NetworkSecretAgent::Field> m_fieldInfo;
    std::vector<ui::TextField *> m_fields;
    ui::CheckBox *m_show = nullptr;
};

} // namespace Lingmo::Shell

