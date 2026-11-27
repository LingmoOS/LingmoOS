// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/navigation_buttons.h"

namespace Lingmo::ui {

NavigationButtons::NavigationButtons()
    : m_back(addButton("chevron-left"))
    , m_forward(addButton("chevron-right"))
{
}

} // namespace Lingmo::ui
