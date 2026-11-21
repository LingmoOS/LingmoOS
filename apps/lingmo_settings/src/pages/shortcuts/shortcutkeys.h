// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>

#include "ui/view.h"

namespace Lingmo::ui {
class Label;
}

namespace settings {

// One shortcut drawn as key caps: [Ctrl] + [Shift] + [Q].
class ShortcutKeys : public Lingmo::ui::View
{
public:
    explicit ShortcutKeys(std::vector<std::string> parts = {});

    void setParts(std::vector<std::string> parts);
    // Half-transparent caps, for a shortcut being replaced or a disabled row.
    void setDimmed(bool dimmed);

    Lingmo::ui::SizeF sizeHint() const override;

protected:
    void layout() override;
    void paint(Lingmo::ui::Painter& painter) override;

private:
    struct Part
    {
        Lingmo::ui::Label* plus = nullptr;
        Lingmo::ui::Label* cap = nullptr;
        // The cap's box, which its label is centred in.
        double capX = 0;
    };

    double capWidth(const Part& part) const;

    std::vector<Part> m_parts;
    bool m_dimmed = false;
};

} // namespace settings
