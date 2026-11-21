// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/network/common/formscroll.h"

#include <algorithm>

#include "ui/layout.h"

using namespace Lingmo::ui;

namespace settings::network {

FormScroll::FormScroll(std::unique_ptr<View> form, double bleed)
    : m_bleed(bleed)
{
    setLayoutHints({.fillWidth = true});
    m_scroll = addChild<ScrollView>();
    Column* inset = m_scroll->setContent<Column>();
    inset->setMargins({bleed, 0, bleed, 0});
    m_form = form.get();
    inset->addChild(std::move(form));
    m_form->setLayoutHints({.fillWidth = true});
}

void FormScroll::scrollToTop()
{
    m_scroll->setScrollPosition({0, 0});
}

SizeF FormScroll::sizeHint() const
{
    return {m_form->sizeHint().width, heightForWidth(m_form->sizeHint().width)};
}

double FormScroll::heightForWidth(double width) const
{
    const double height = m_form->heightForWidth(width);
    return maximumHeight ? std::min(height, std::max(0.0, maximumHeight())) : height;
}

void FormScroll::layout()
{
    m_scroll->setGeometry({-m_bleed, 0, geometry().width + 2 * m_bleed, geometry().height});
}

} // namespace settings::network
