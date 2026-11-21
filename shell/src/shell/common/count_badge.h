// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <string>

#include "ui/view.h"

namespace Lingmo::ui {
class Label;
} // namespace Lingmo::UI

namespace Lingmo::Shell {

// An application's count on the top-right corner of its icon: a red circle
// with the number in it, as wide as a pill for two digits, and "99+" past 99.
class CountBadge final : public ui::View
{
public:
    static constexpr ui::Color kColor {255, 59, 48, 255};
    // Its height as a share of the icon's: the dock's, and the launcher's
    // larger icons'.
    static constexpr double kDockRatio = 0.34;
    static constexpr double kLauncherRatio = 0.24;

    CountBadge();
    ~CountBadge() override;

    // Hidden at 0.
    void setCount(int64_t count);
    int64_t count() const { return m_count; }
    const std::string &text() const;
    static std::string textFor(int64_t count);

    void setHeightRatio(double ratio);
    // Sizes it for the icon and puts it on the icon's top-right corner, both
    // in the parent's coordinates.
    void setIconRect(const ui::RectF &icon);
    // The badge's height for an icon of `iconSize`.
    static double heightFor(double iconSize, double ratio);

protected:
    void layout() override;
    void paint(ui::Painter &painter) override;

private:
    void place();

    ui::Label *m_label = nullptr;
    int64_t m_count = 0;
    double m_heightRatio = kDockRatio;
    ui::RectF m_iconRect;
};

} // namespace Lingmo::Shell

