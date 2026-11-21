// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/window_header.h"

#include <algorithm>
#include <cmath>

#include "ui/application.h"
#include "ui/header_button.h"
#include "ui/window.h"

namespace Lingmo::ui {

static constexpr double kButton = 32;
static constexpr double kButtonSpacing = 1;

namespace {

// Restore while the window is maximized, maximize otherwise.
class MaximizeButton final : public HeaderButton
{
public:
    MaximizeButton()
        : HeaderButton("maximize", kButton)
    {
    }

protected:
    std::string currentArtwork() const override
    {
        const Window* window = this->window();
        return window && window->states().maximized ? "restore" : "maximize";
    }
};

} // namespace

WindowHeader::WindowHeader(double height)
    : m_height(height)
    , m_minimize(addChild<HeaderButton>("minimize", kButton))
    , m_maximize(addChild<MaximizeButton>())
    , m_close(addChild<HeaderButton>("close", kButton))
{
    m_minimize->onClicked = [this] {
        if (Window* window = this->window())
            window->minimize();
    };
    m_maximize->onClicked = [this] {
        if (Window* window = this->window())
            window->setMaximized(!window->states().maximized);
    };
    m_close->onClicked = [this] {
        Window* window = this->window();
        if (window && window->onCloseRequested)
            window->onCloseRequested();
        else if (window && !window->isPopup())
            Application::instance()->quit();
    };
}

void WindowHeader::setContentView(std::unique_ptr<View> view)
{
    if (m_content)
        takeChild(m_content);
    m_content = view.get();
    addChild(std::move(view));
    // Under the buttons.
    raiseChild(m_minimize);
    raiseChild(m_maximize);
    raiseChild(m_close);
}

void WindowHeader::setMinimizeVisible(bool visible)
{
    m_minimize->setVisible(visible);
}

void WindowHeader::setMaximizeVisible(bool visible)
{
    m_maximize->setVisible(visible);
}

void WindowHeader::setCloseVisible(bool visible)
{
    m_close->setVisible(visible);
}

void WindowHeader::setButtonMargin(double margin)
{
    if (margin == m_buttonMargin)
        return;
    m_buttonMargin = margin;
    requestLayout();
}

void WindowHeader::setButtonsCentered(bool centered)
{
    if (centered == m_buttonsCentered)
        return;
    m_buttonsCentered = centered;
    requestLayout();
}

void WindowHeader::layout()
{
    // A RowLayout centres on whole pixels.
    const double top = m_buttonsCentered ? std::floor((geometry().height - kButton) / 2) : m_buttonMargin;
    double row = 0;
    for (HeaderButton* button : {m_minimize, m_maximize, m_close}) {
        if (button->isVisible())
            row += (row > 0 ? kButtonSpacing : 0) + kButton;
    }
    // FishUI's RowLayout of the content, the buttons' row and an end margin
    // item: each cell starts on a whole logical pixel, halves rounding up,
    // so a fractional margin moves the row by up to half a pixel.
    const double rowX = std::floor(geometry().width - m_buttonMargin - row + 0.5);
    double x = rowX;
    for (HeaderButton* button : {m_minimize, m_maximize, m_close}) {
        if (!button->isVisible())
            continue;
        button->setGeometry({x, top, kButton, kButton});
        x += kButton + kButtonSpacing;
    }
    // The content keeps the spacing before the row, as the ports expect.
    if (m_content)
        m_content->setGeometry({0, 0, std::max(0.0, rowX - kButtonSpacing), geometry().height});
}

void WindowHeader::pointerPress(const PointerEvent& event)
{
    Window* window = this->window();
    if (!window || event.button != MouseButton::Left || window->states().fullScreen)
        return;
    if (event.clickCount == 2) {
        window->setMaximized(!window->states().maximized);
        return;
    }
    window->startMove();
}

} // namespace Lingmo::ui
