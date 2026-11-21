// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "lingmo/networkmanager/handler.h"
#include "ui/cell_layout.h"

namespace Lingmo::ui {
class Label;
class Switch;
class TextField;
}

namespace settings::network {

class PluginForm;
class WireGuardEditor;

// What a VPN profile is made of, for the Add VPN and details dialogs: its
// name, the fields of its kind, and who may use it. load() takes
// Handler::vpnForm(), collect() gives Handler::saveVpn() its form.
class VpnEditor : public Lingmo::ui::ColumnLayout
{
public:
    VpnEditor();
    ~VpnEditor() override;

    void load(const Lingmo::Networkmanager::VpnForm& form);
    void setSecrets(const Lingmo::Networkmanager::VpnSecrets& secrets);
    Lingmo::Networkmanager::VpnValues collect() const;
    void focusFirst();
    // Also measured: a label beside the editor sharing its column.
    void measure(double width);

    const Lingmo::Networkmanager::VpnForm& form() const { return m_form; }
    bool installed() const { return m_form.installed; }
    bool complete() const;
    // The label column's width, which a label beside the editor takes too.
    double labelWidth() const;

    // Return in a field.
    std::function<void()> onAccepted;
    // Anything that changes complete() or the size.
    std::function<void()> onChanged;

protected:
    void layout() override;

private:
    void changed();

    Lingmo::Networkmanager::VpnForm m_form;
    double m_extraLabelWidth = 0;
    Lingmo::ui::Label* m_notInstalled;
    Lingmo::ui::RowLayout* m_nameRow;
    Lingmo::ui::Label* m_nameLabel;
    Lingmo::ui::TextField* m_name;
    PluginForm* m_plugin = nullptr;
    WireGuardEditor* m_wireGuard = nullptr;
    Lingmo::ui::ColumnLayout* m_switches;
    Lingmo::ui::Switch* m_shared;
    Lingmo::ui::Switch* m_ownNetworkOnly;
};

} // namespace settings::network
