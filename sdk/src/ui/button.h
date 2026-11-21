// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>
#include <string>

#include "ui/geometry.h"
#include "ui/view.h"

namespace Lingmo::ui {

class Label;
class Animation;

class Button : public View
{
public:
    explicit Button(std::string text = {});
    ~Button() override;

    void setText(std::string text);

    SizeF sizeHint() const override;
    RectF visualRect() const override;
    bool acceptsPointer() const override { return true; }
    bool acceptsFocus() const override { return true; }

    std::function<void()> onClicked;

protected:
    void layout() override;
    void paint(Painter& painter) override;
    void enabledChanged() override;

    void pointerEnter() override;
    void pointerLeave() override;
    void pointerPress(const PointerEvent& event) override;
    void pointerRelease(const PointerEvent& event) override;
    bool keyPress(const KeyEvent& event) override;
    bool keyRelease(const KeyEvent& event) override;

private:
    void animateHover(double target);
    void beginRipple(PointF origin);
    void fadeRipple();

    Label* m_label;
    std::unique_ptr<Animation> m_hoverAnimation;
    std::unique_ptr<Animation> m_rippleAnimation;
    std::unique_ptr<Animation> m_rippleFadeAnimation;
    PointF m_rippleOrigin;
    double m_hoverAmount = 0;
    double m_rippleProgress = 0;
    double m_rippleOpacity = 0;
    bool m_pressed = false;
};

} // namespace Lingmo::ui
