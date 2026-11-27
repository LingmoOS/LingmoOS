// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/header_button_group.h"

namespace Lingmo::ui {

// Back and forward navigation buttons on a single shared plate in the
// window-controls style.
class NavigationButtons : public HeaderButtonGroup
{
public:
    NavigationButtons();

    HeaderButton* back() const { return m_back; }
    HeaderButton* forward() const { return m_forward; }

private:
    HeaderButton* m_back;
    HeaderButton* m_forward;
};

} // namespace Lingmo::ui
