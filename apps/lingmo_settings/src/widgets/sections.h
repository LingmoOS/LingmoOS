// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <optional>
#include <string>

#include "ui/label.h"
#include "ui/layout.h"
#include "ui/cell_layout.h"

namespace settings {

// Secondary text lined up with a card's text: a section's title above its
// card, or a footnote below it.
class SectionLabel : public Lingmo::ui::Column
{
public:
    explicit SectionLabel(std::string text = {}, bool wrap = false);

    Lingmo::ui::Label* label() const { return m_label; }
    void setText(std::string text) { m_label->setText(std::move(text)); }
    // Unset: the theme's disabled text colour.
    void setColor(std::optional<Lingmo::ui::ColorRecipe> color);

private:
    Lingmo::ui::Label* m_label;
};

// A titled group on a page: a section label over its content, set tightly so
// the text reads as belonging to that content.
class Section : public Lingmo::ui::ColumnLayout
{
public:
    explicit Section(std::string title = {});

    SectionLabel* title() const { return m_title; }

private:
    SectionLabel* m_title;
};

// Secondary text that wraps, inset as a card's text is.
class Footnote : public SectionLabel
{
public:
    explicit Footnote(std::string text = {})
        : SectionLabel(std::move(text), true)
    {
    }
};

} // namespace settings
