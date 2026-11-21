// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/network/common/rows.h"

#include "ui/icon.h"
#include "ui/progress_bar.h"
#include "ui/theme.h"
#include "widgets/iconbutton.h"

using namespace Lingmo::ui;
namespace nm = Lingmo::Networkmanager;

namespace settings::network {

void arrange(View* parent, const std::vector<View*>& order)
{
    const auto& children = parent->children();
    // Already in order at the end: nothing moves.
    if (children.size() >= order.size()) {
        bool inOrder = true;
        const size_t offset = children.size() - order.size();
        for (size_t i = 0; i < order.size() && inOrder; ++i)
            inOrder = children[offset + i].get() == order[i];
        if (inOrder)
            return;
    }
    for (View* view : order)
        parent->raiseChild(view);
}

bool isEnterprise(nm::SecurityType type)
{
    return type == nm::SecurityType::WpaEap || type == nm::SecurityType::Wpa2Eap
           || type == nm::SecurityType::Wpa3SuiteB192;
}

WifiRow::WifiRow()
{
    m_icon = setLeading<Icon>("network-wireless-connected-00", 18);
    m_busy = addContent<BusyIndicator>();
    m_busy->setLayoutHints({.preferredWidth = 18, .preferredHeight = 18});
    m_check = addContent<Icon>("check", 16);
    m_lock = addContent<Icon>("lock", 16);
    m_info = addContent<IconButton>("info", 20);
    m_info->onClicked = [this] {
        if (onInfo)
            onInfo(this);
    };
}

void WifiRow::setItem(const nm::NetworkItem& item, nm::ConnectionStatus displayed)
{
    m_item = item;
    m_displayed = displayed;
    setName("wifi:" + item.name);
    setText(item.uniqueName);
    m_icon->setIconName(item.icon);
    const bool busy = displayed == nm::ConnectionStatus::Activating || displayed == nm::ConnectionStatus::Deactivating;
    m_busy->setVisible(busy);
    m_busy->setRunning(busy);
    m_check->setVisible(item.connectionState == nm::ConnectionStatus::Activated);
    m_lock->setVisible(item.securityType != nm::SecurityType::Unknown && item.securityType != nm::SecurityType::None);
}

void WifiRow::layout()
{
    const Color highlight = Theme::current().highlight;
    m_check->setColor(highlight);
    m_info->setColor(highlight);
    ListRow::layout();
}

} // namespace settings::network
