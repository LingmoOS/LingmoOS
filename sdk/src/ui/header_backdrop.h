// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace Lingmo::base { class EventLoop; }
namespace base = Lingmo::base;

#include <optional>

#include "base/signal.h"
#include "ui/color_mixer.h"
#include "ui/view.h"

namespace Lingmo::ui {

class ScrollView;

// The frosted bar over the top or bottom of a scrolling view, the content
// scrolled beneath it blurred by the mip blur of multi_effect_blur.h
// (blurMax 32), under a translucent tint and a hairline. At rest it is
// invisible; it fades in over 16 pixels of content scrolling under it.
// It lies over the view, which it must not be inside.
class HeaderBackdrop : public View
{
public:
    enum class Edge {
        Top,
        Bottom,
    };

    explicit HeaderBackdrop(ScrollView* view, Edge edge = Edge::Top);
    ~HeaderBackdrop() override;

    void setSeparatorVisible(bool visible);
    // What the view's content is drawn on; the theme's background unless set.
    void setBaseColor(std::optional<ColorRecipe> color);
    // The tint, at 0.72; the theme's second background unless set.
    void setTintColor(std::optional<ColorRecipe> color);

protected:
    void paint(Painter& painter) override;

private:
    double fade() const;

    ScrollView* m_view;
    Edge m_edge;
    bool m_separatorVisible = true;
    std::optional<ColorRecipe> m_baseColor;
    std::optional<ColorRecipe> m_tintColor;
    base::Subscription m_scrolled;
};

} // namespace Lingmo::ui
