// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <functional>
#include <vector>

#include "shell/launcher/launcher_tween.h"
#include "ui/view.h"

namespace Lingmo::Shell {

// A row of dots that tracks the current page and lets the user tap to jump.
class LauncherPageIndicator final : public ui::View
{
public:
    static constexpr double kPadding = 6;

    LauncherPageIndicator();

    bool acceptsPointer() const override { return m_count > 1; }

    void setDotSize(double size);
    void setDots(int count, int current, bool animate);

    ui::SizeF implicitSize() const;
    ui::RectF dotRect(int index) const;

    std::function<void(int)> onPick;

protected:
    void paint(ui::Painter &painter) override;
    void pointerPress(const ui::PointerEvent &event) override { pick(event.position); }
    void pointerMove(const ui::PointerEvent &event) override { pick(event.position); }

private:
    double spacing() const;
    void pick(ui::PointF position);

    double m_dotSize = 0;
    int m_count = 0;
    int m_current = -1;
    std::vector<double> m_alphas;
    LauncherTween m_opacityTween;
    LauncherTween m_colorTween;
};

} // namespace Lingmo::Shell
