// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "widgets/sections.h"

#include "widgets/metrics.h"

using namespace Lingmo::ui;

namespace settings {

SectionLabel::SectionLabel(std::string text, bool wrap)
{
    setMargins({metrics::cardHorizontalPadding, 0, metrics::cardHorizontalPadding, 0});
    setLayoutHints({.fillWidth = true});
    m_label = addChild<Label>(std::move(text));
    m_label->setWrap(wrap);
    m_label->setColor(ColorId::DisabledText);
    m_label->setLayoutHints({.fillWidth = true});
}

void SectionLabel::setColor(std::optional<ColorRecipe> color)
{
    m_label->setColor(std::move(color).value_or(ColorId::DisabledText));
}

Section::Section(std::string title)
{
    setSpacing(metrics::sectionLabelSpacing);
    m_title = addChild<SectionLabel>(std::move(title));
    m_title->setColor(ColorId::Text);
    m_title->label()->setWeight(600);
    m_title->label()->setFontSize(14);
}

} // namespace settings
