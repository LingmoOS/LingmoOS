// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>

#include "ui/scroll_view.h"

namespace settings::network {

// A long form in a dialog: as tall as the form up to a limit, then scrolling.
// It reaches `bleed` past its own sides, so the scroll bar runs along the
// container's edge rather than the fields', which keep their inset.
class FormScroll : public Lingmo::ui::View
{
public:
    FormScroll(std::unique_ptr<Lingmo::ui::View> form, double bleed);

    Lingmo::ui::View* form() const { return m_form; }
    void scrollToTop();

    // The tallest it may be; unbounded while unset.
    std::function<double()> maximumHeight;

    Lingmo::ui::SizeF sizeHint() const override;
    double heightForWidth(double width) const override;

protected:
    void layout() override;

private:
    double m_bleed;
    Lingmo::ui::ScrollView* m_scroll;
    Lingmo::ui::View* m_form;
};

} // namespace settings::network
