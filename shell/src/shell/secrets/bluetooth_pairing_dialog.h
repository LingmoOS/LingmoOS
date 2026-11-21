// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <memory>
#include <string>

#include "shell/secrets/secret_dialog.h"

namespace Lingmo::ui {
class Label;
class TextField;
} // namespace Lingmo::UI

namespace Lingmo::Shell {

// What the pairing dialog shows and answers: bluez::Manager's pairing prompt.
class PairingAgent
{
public:
    virtual ~PairingAgent() = default;

    // Agent1's method that asked, e.g. "RequestConfirmation"; empty for none.
    virtual std::string pairingKind() const = 0;
    virtual std::string pairingDevice() const = 0;
    virtual std::string pairingCode() const = 0;
    virtual int pairingEntered() const = 0;
    virtual std::string errorText() const = 0;
    virtual void respondPairing(bool accept, const std::string &code) = 0;
};

// The Bluetooth pairing dialog: answers the session's BlueZ agent,
// for pairings any application starts and for requests a device makes on
// its own. A code to compare or type on the device, a PIN or passkey field,
// the manager's error, and Decline/Cancel beside Confirm, Pair or Allow;
// display-only requests have no primary button.
class BluetoothPairingDialog final : public SecretDialog
{
public:
    BluetoothPairingDialog(Shell *shell, PairingAgent *agent);

    static std::string text(const std::string &kind, const std::string &device);

    ui::TextField *codeField() const { return m_field; }
    ui::Label *code() const { return m_code; }
    ui::Label *error() const { return m_error; }

protected:
    void build(ui::View *frame) override;
    ui::View *initialFocus() const override;
    void accept() override;
    void update() override;

private:
    bool inputRequired() const;
    bool displayOnly() const;

    PairingAgent *m_agent;
    ui::Label *m_code = nullptr;
    ui::Label *m_entered = nullptr;
    ui::TextField *m_field = nullptr;
    ui::Label *m_error = nullptr;
};

} // namespace Lingmo::Shell

