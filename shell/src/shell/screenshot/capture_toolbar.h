// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "base/signal.h"
#include "base/timer.h"
#include "ui/geometry.h"
#include "ui/view.h"

namespace Lingmo::Audio {
class AudioModel;
}

namespace Lingmo::ui {
class Label;
class Menu;
class Window;
} // namespace Lingmo::UI

namespace Lingmo::Shell {

class CaptureOverlay;
class CaptureTooltip;
class ScreenCapture;
class Shell;

// A glyph or a label on the capture toolbar; a
// checked one is the current mode.
class ToolbarButton : public ui::View
{
public:
    // A glyph: the artwork's file, `iconSize` square.
    ToolbarButton(std::string icon, int iconSize, std::string tip);
    // A label, or with `accent` the filled button that does the capture.
    explicit ToolbarButton(std::string text, bool accent = false);
    ~ToolbarButton() override;

    void setChecked(bool checked);
    bool isChecked() const { return m_checked; }
    void setText(const std::string &text);
    const std::string &tip() const { return m_tip; }
    void setTip(std::string tip);
    bool isPressed() const { return m_pressed; }
    bool isPointerOver() const { return m_hovered; }
    // Options: a label and a chevron instead of the button's own label.
    void setChevron(bool chevron);

    ui::SizeF sizeHint() const override;
    bool acceptsPointer() const override { return isEnabled(); }

    // The press began while the Options menu was up.
    std::function<void()> onPressed;
    std::function<void(uint32_t modifiers)> onClicked;
    // Hover or a press changed: the tooltip follows.
    std::function<void()> onStateChanged;

protected:
    void pointerEnter() override;
    void pointerLeave() override;
    void pointerPress(const ui::PointerEvent &event) override;
    void pointerRelease(const ui::PointerEvent &event) override;
    void enabledChanged() override;
    void layout() override;
    void paint(ui::Painter &painter) override;

private:
    ui::Color contentColor() const;

    std::string m_icon;
    int m_iconSize = 24;
    std::string m_tip;
    std::string m_text;
    bool m_accent = false;
    bool m_chevron = false;
    bool m_checked = false;
    bool m_pressed = false;
    bool m_hovered = false;
    ui::Label *m_label = nullptr;
};

// The capture toolbar: Close, the three ways to take
// a picture, the three ways to record, Options and the button that does it,
// on the compositor's glass. Dragged by its background.
class CaptureToolbar : public ui::View
{
public:
    // DragHandler's threshold, the platform's start drag distance.
    static constexpr double kDragThreshold = 10;

    CaptureToolbar(CaptureOverlay *overlay, ScreenCapture *capture, Shell *shell);
    ~CaptureToolbar() override;

    double radius() const;
    ui::SizeF implicitSize() const;
    // The modes, the recording state and the labels from the capture.
    void update();
    // A drag of the toolbar, or one on the overlay crossing it: no tooltip.
    bool isDragging() const { return m_dragActive || m_selecting; }
    void setSelecting(bool selecting);
    bool isOptionsOpen() const;
    void closeOptions();
    // A press anywhere on the overlay, before it is delivered: the Options
    // menu closes on any press outside it.
    void pressedOutsideMenu();
    void hideTooltip();
    // Closes the menu and destroys the tooltip's popup, while the overlay's
    // window they hang from is still there.
    void releasePopups();
    // Hides the tooltip when the toolbar does.
    void visibilityChanged();

    ToolbarButton *closeButton() const { return m_buttons.front(); }
    // Image screen, window, region, then video screen, window, region.
    ToolbarButton *modeButton(int type, int source) const;
    ToolbarButton *optionsButton() const { return m_options; }
    ToolbarButton *actionButton() const { return m_action; }
    CaptureTooltip *tooltip() const { return m_tooltip.get(); }
    void openOptions();

    // The toolbar was dragged; its rect in the overlay's coordinates.
    std::function<void()> onMoved;
    // The Options menu opened or closed.
    std::function<void()> onOptionsChanged;

    bool acceptsPointer() const override { return true; }

protected:
    void pointerPress(const ui::PointerEvent &event) override;
    void pointerMove(const ui::PointerEvent &event) override;
    void pointerRelease(const ui::PointerEvent &event) override;
    void layout() override;
    void paint(ui::Painter &painter) override;

private:
    void buttonStateChanged(ToolbarButton *button);
    void retranslate();
    std::vector<double> separatorPositions() const;
    void buildMenu();

    CaptureOverlay *m_overlay;
    ScreenCapture *m_capture;
    Shell *m_shell;
    // In row order, with the separators' places between groups.
    std::vector<ToolbarButton *> m_buttons;
    ToolbarButton *m_options = nullptr;
    ToolbarButton *m_action = nullptr;
    std::unique_ptr<CaptureTooltip> m_tooltip;
    std::unique_ptr<ui::Menu> m_menu;
    std::unique_ptr<audio::AudioModel> m_sources;
    base::Subscription m_languageChanged;
    bool m_menuOpen = false;
    // A press on Options while its menu was up only closes it.
    bool m_closesOptions = false;
    bool m_selecting = false;
    bool m_pressed = false;
    bool m_dragActive = false;
    ui::PointF m_pressPoint;
    ui::PointF m_pressPosition;
};

// The toolbar's tooltip, the dock's bubble: a popup of the overlay of its
// own size that the next button moves, transparent for input. Up as soon as
// the pointer is on a button; leaving hides it 150 ms later unless another
// button takes it.
class CaptureTooltip
{
public:
    static constexpr std::chrono::milliseconds kHideDelay {150};

    CaptureTooltip(CaptureToolbar *toolbar, CaptureOverlay *overlay, Shell *shell);
    ~CaptureTooltip();

    void showFor(ToolbarButton *button);
    void leave(ToolbarButton *button);
    void hideNow();
    // After a drag: the button under the pointer gets its tooltip.
    void draggingChanged();
    // Shown, hidden or moved with the toolbar, the Options menu.
    void refresh();
    // Gone for good, before the overlay's window.
    void release();

    bool isShown() const;
    const std::string &text() const { return m_text; }
    ui::Window *window() const { return m_window.get(); }
    ui::Label *label() const { return m_label; }
    // The bubble's body in the popup, without the arrow.
    const ui::RectF &bodyRect() const { return m_body; }

private:
    void apply();

    CaptureToolbar *m_toolbar;
    CaptureOverlay *m_overlay;
    Shell *m_shell;
    ToolbarButton *m_target = nullptr;
    ToolbarButton *m_hovered = nullptr;
    std::string m_text;
    std::unique_ptr<ui::Window> m_window;
    ui::Label *m_label = nullptr;
    ui::RectF m_body;
    base::Timer m_hideTimer;
    bool m_released = false;
};

} // namespace Lingmo::Shell

