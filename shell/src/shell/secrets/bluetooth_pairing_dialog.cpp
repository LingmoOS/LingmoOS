// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/secrets/bluetooth_pairing_dialog.h"

#include "base/i18n.h"
#include "ui/dialog_window.h"
#include "ui/label.h"
#include "ui/text_field.h"

using Lingmo::i18n::format;
using Lingmo::i18n::tr;
using Lingmo::i18n::trn;

namespace Lingmo::Shell {

static constexpr int kDemiBold = 600;
static constexpr int kNormal = 400;

BluetoothPairingDialog::BluetoothPairingDialog(Shell *shell, PairingAgent *agent)
    : SecretDialog(shell)
    , m_agent(agent)
{
}

std::string BluetoothPairingDialog::text(const std::string &kind, const std::string &device)
{
    if (kind == "RequestConfirmation")
        return format(tr("Confirm that “{name}” shows this code."), {{"name", device}});
    if (kind == "RequestPinCode")
        return format(tr("Enter the PIN for “{name}”."), {{"name", device}});
    if (kind == "RequestPasskey")
        return format(tr("Enter the passkey shown on “{name}”."), {{"name", device}});
    if (kind == "DisplayPinCode" || kind == "DisplayPasskey")
        return format(tr("Type this code on “{name}”, then press Enter on it."), {{"name", device}});
    if (kind == "AuthorizeService")
        return format(tr("Allow “{name}” to use this Bluetooth service?"), {{"name", device}});
    return format(tr("Allow “{name}” to pair with this computer?"), {{"name", device}});
}

bool BluetoothPairingDialog::inputRequired() const
{
    const std::string kind = m_agent->pairingKind();
    return kind == "RequestPinCode" || kind == "RequestPasskey";
}

bool BluetoothPairingDialog::displayOnly() const
{
    const std::string kind = m_agent->pairingKind();
    return kind == "DisplayPinCode" || kind == "DisplayPasskey";
}

void BluetoothPairingDialog::build(ui::View *frame)
{
    m_code = frame->addChild<ui::Label>();
    m_code->setWrap(true);
    m_code->setWrapAnywhere(true);
    m_code->setTextAlignment(ui::Label::TextAlignment::Center);
    m_entered = frame->addChild<ui::Label>();
    m_entered->setTextAlignment(ui::Label::TextAlignment::Center);
    m_field = frame->addChild<ui::TextField>();
    m_field->onTextChanged = [this] {
        if (primaryButton())
            primaryButton()->setEnabled(!inputRequired() || !m_field->text().empty());
    };
    m_field->onAccepted = [this] { accept(); };
    // Input method hints that allow completion.
    m_field->setInputHints(ui::CompletionHint);
    m_error = frame->addChild<ui::Label>();
    m_error->setWrap(true);
    setRows({{m_code}, {m_entered}, {m_field}, {m_error}});
}

ui::View *BluetoothPairingDialog::initialFocus() const
{
    if (inputRequired())
        return m_field;
    return primaryButton();
}

void BluetoothPairingDialog::accept()
{
    // A code the manager rejects leaves the request open with errorText set.
    if (!inputRequired() || !m_field->text().empty())
        m_agent->respondPairing(true, m_field->text());
}

void BluetoothPairingDialog::update()
{
    const std::string kind = m_agent->pairingKind();
    setTitle(tr("Bluetooth Pairing Request"));
    setText(text(kind, m_agent->pairingDevice()));

    const bool service = kind == "AuthorizeService";
    m_code->setText(m_agent->pairingCode());
    m_code->setFontSize(service ? 14 : 28);
    m_code->setWeight(service ? kNormal : kDemiBold);
    m_code->setLetterSpacing(service ? 0 : 4);
    m_code->setVisible(!m_agent->pairingCode().empty());

    const int count = m_agent->pairingEntered();
    m_entered->setText(trn("{n} digit entered", "{n} digits entered", count));
    m_entered->setColor(ui::ColorId::DisabledText);
    m_entered->setVisible(kind == "DisplayPasskey" && count > 0);

    m_field->setVisible(inputRequired());
    m_field->setPlaceholder(kind == "RequestPasskey" ? tr("Passkey") : tr("PIN"));
    m_field->setMaximumLength(kind == "RequestPasskey" ? 6 : 16);
    m_field->setInputPurpose(kind == "RequestPasskey" ? ui::TextPurpose::Digits : ui::TextPurpose::Normal);

    m_error->setText(m_agent->errorText());
    m_error->setColor(ui::ColorId::DestructiveAction);
    m_error->setVisible(!m_agent->errorText().empty());

    const bool decline = kind == "RequestConfirmation" || kind == "RequestAuthorization" || service;
    cancelButton()->setText(decline ? tr("Decline") : tr("Cancel"));
    primaryButton()->setVisible(!displayOnly());
    primaryButton()->setText(kind == "RequestConfirmation" ? tr("Confirm") : inputRequired() ? tr("Pair") : tr("Allow"));
    primaryButton()->setEnabled(!inputRequired() || !m_field->text().empty());
}

} // namespace Lingmo::Shell
