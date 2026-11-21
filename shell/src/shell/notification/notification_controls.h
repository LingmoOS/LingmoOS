// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <functional>
#include <memory>
#include <string>

#include "base/signal.h"
#include "ui/geometry.h"
#include "ui/view.h"

namespace Lingmo::ui {
class Label;
}

namespace Lingmo::Shell {

class Tween;

// Whether white reads on the
// surface (its relative luminance, alpha ignored, at most 0.3), and the ink
// of that colour at an alpha.
bool inkIsDark(ui::Color surface);
ui::Color inkOn(ui::Color surface, double alpha);
// A colour of rgba components, each rounded to 16 bits.
ui::Color rgba(double red, double green, double blue, double alpha);

// The round "x" that turns into a "Clear" pill when it asks first: the
// card's close button (24 px, its plate only under the pointer) and the
// centre's clear buttons (26 px, always on a plate, always asking first).
class ClearButton : public ui::View
{
public:
    enum class Kind {
        CardClose,
        Clear,
    };

    ClearButton(ui::Window *window, Kind kind);
    ~ClearButton() override;

    // The plate's fill for hover (1) and press (2), for the card's close
    // button; the card's own button tint.
    void setTint(std::function<ui::Color(int state)> tint) { m_tint = std::move(tint); }

    // A first click arms, a second one clicks.
    void setConfirm(bool confirm);
    bool isArmed() const { return m_armed; }
    void setArmed(bool armed);
    // 0 is the round button, 1 the pill.
    double progress() const;
    // Backs an armed button out unless the window point is on it.
    void disarmUnless(ui::PointF windowPoint);
    // Hiding resets it, as a hidden item gets no frames.
    void reset();

    double buttonHeight() const;
    // The width at the current progress.
    double currentWidth() const;

    std::function<void()> onClicked;
    // The width follows the progress; the owner places the button again.
    std::function<void()> onWidthChanged;
    // Armed or disarmed.
    std::function<void()> onArmedChanged;

    bool acceptsPointer() const override { return true; }

protected:
    void pointerEnter() override;
    void pointerLeave() override;
    void pointerPress(const ui::PointerEvent &event) override;
    void pointerRelease(const ui::PointerEvent &event) override;
    void enabledChanged() override;
    void layout() override;
    void paint(ui::Painter &painter) override;

private:
    void updateLabel();

    Kind m_kind;
    std::function<ui::Color(int)> m_tint;
    bool m_confirm = false;
    bool m_armed = false;
    bool m_hovered = false;
    bool m_pressed = false;
    ui::Label *m_label = nullptr;
    std::unique_ptr<Tween> m_progress;
    base::Subscription m_languageChanged;
};

// A 26 px pill around a small medium-weight label.
class PillButton : public ui::View
{
public:
    explicit PillButton(std::string text);

    void setText(std::string text);
    double preferredWidth() const;

    std::function<void()> onClicked;

    bool acceptsPointer() const override { return true; }

protected:
    void pointerEnter() override;
    void pointerLeave() override;
    void pointerPress(const ui::PointerEvent &event) override;
    void pointerRelease(const ui::PointerEvent &event) override;
    void layout() override;
    void paint(ui::Painter &painter) override;

private:
    bool m_hovered = false;
    bool m_pressed = false;
    ui::Label *m_label = nullptr;
};

// The fill of a pill button on its surface (PillButton, ClearButton): light
// 0.07 / 0.12 / 0.16, dark 0.16 / 0.24 / 0.30 for rest, hover and press.
ui::Color pillFill(ui::Color surface, bool hovered, bool pressed);

} // namespace Lingmo::Shell

