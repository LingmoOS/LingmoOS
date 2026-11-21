// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

#include "widgets/listrow.h"

namespace Lingmo::ui {
class ThemeIcon;
}

namespace settings {

// One choice of several: its name, optionally a theme icon before it, and a
// check when it is the current one.
class CheckRow : public ListRow
{
public:
    explicit CheckRow(std::string text = {});

    void setChecked(bool checked);
    bool isChecked() const { return m_checked; }

    // An 18 px theme icon centred in a 24 px slot before the label.
    void setIcon(const std::string& name);

protected:
    void layout() override;

private:
    TrailingIcon* m_check;
    Lingmo::ui::ThemeIcon* m_icon = nullptr;
    bool m_checked = false;
};

} // namespace settings
