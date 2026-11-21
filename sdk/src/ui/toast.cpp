// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/color_provider.h"
#include "ui/toast.h"

#include <algorithm>
#include <cmath>
#include <memory>

#include "base/event_loop.h"
#include "ui/animation.h"
#include "ui/application.h"
#include "ui/button.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/view.h"
#include "ui/window.h"

namespace Lingmo::ui {

static constexpr std::chrono::milliseconds kAnimation{400};
// RowLayout's and GridLayout's default spacing.
static constexpr double kLayoutSpacing = 5;
// DropShadow radius 32: a deviation of (32 + 1) / 3.3333.
static constexpr double kShadowSigma = 9.9;
// At most this many earlier messages stay when another comes.
static constexpr size_t kKept = 3;

// One message: a control with large padding, a label and an optional button.
class Bubble : public View
{
public:
    Bubble(std::string message, std::string actionText, std::function<void()> action,
           std::chrono::milliseconds timeout, std::function<void()> changed)
        : m_label(addChild<Label>(std::move(message)))
        , m_timeout(timeout)
        , m_changed(std::move(changed))
    {
        m_label->setWrap(true);
        m_label->setMaximumLineCount(4);
        m_label->setElide(Label::Elide::Right);
        m_label->setVerticallyCentered(true);
        if (!actionText.empty()) {
            m_button = addChild<Button>(std::move(actionText));
            m_button->onClicked = [this, action = std::move(action)] {
                close();
                if (action)
                    action();
            };
        }
        setTracksHover(true);
        setOpacity(0);
    }

    ~Bubble() override { stopTimer(); }

    bool isClosing() const { return m_closing; }
    double bottomMargin() const { return m_bottomMargin; }

    // Label.maximumWidth: within the window, less four large spacings.
    SizeF bubbleSize(double windowWidth) const
    {
        const double padding = Theme::current().largeSpacing;
        const double labelWidth = std::min(windowWidth - 4 * padding, m_label->sizeHint().width);
        const double labelHeight = m_label->heightForWidth(labelWidth);
        double width = labelWidth;
        double height = labelHeight;
        if (m_button) {
            const SizeF button = m_button->sizeHint();
            width += kLayoutSpacing + button.width;
            height = std::max(height, button.height);
        }
        return {width + 2 * padding, height + 2 * padding};
    }

    void open()
    {
        animate(true);
        restartTimer();
    }

    void close()
    {
        if (m_closing)
            return;
        m_closing = true;
        stopTimer();
        animate(false);
    }

    bool acceptsPointer() const override { return true; }

protected:
    void layout() override
    {
        const double padding = Theme::current().largeSpacing;
        const RectF content = localRect().adjusted(padding, padding, -padding, -padding);
        double labelWidth = content.width;
        if (m_button) {
            const SizeF button = m_button->sizeHint();
            labelWidth -= kLayoutSpacing + button.width;
            m_button->setGeometry({content.right() - button.width, content.y + (content.height - button.height) / 2,
                                   button.width, button.height});
        }
        m_label->setGeometry({content.x, content.y, std::max(0.0, labelWidth), content.height});
    }

    void paint(Painter& painter) override
    {
        const Theme& theme = Theme::current();
        painter.save();
        painter.multiplyOpacity(0.9);
        painter.drawShadow(localRect(), theme.mediumRadius, kShadowSigma, color(ColorId::ToastShadow));
        painter.drawRoundedRect(localRect(), {theme.mediumRadius, 0, themeCorners()}, theme.background);
        painter.restore();
    }

    RectF visualRect() const override
    {
        const double reach = Painter::shadowReach(kShadowSigma);
        return localRect().adjusted(-reach, -reach, reach, reach);
    }

    // The timer runs while the pointer is away, and starts over when it leaves.
    void hoverChanged() override
    {
        if (m_closing)
            return;
        if (isHovered())
            stopTimer();
        else
            restartTimer();
    }

    void pointerPress(const PointerEvent&) override { m_pressed = true; }

    void pointerRelease(const PointerEvent& event) override
    {
        const double padding = Theme::current().largeSpacing;
        const RectF content = localRect().adjusted(padding, padding, -padding, -padding);
        if (std::exchange(m_pressed, false) && content.contains(event.position))
            close();
    }

private:
    void animate(bool in)
    {
        Window* window = this->window();
        if (!window)
            return;
        m_animation = std::make_unique<Animation>(window);
        m_animation->setDuration(kAnimation);
        m_animation->setEasing(Animation::Easing::InOutQuad);
        const double fromOpacity = in ? 0 : opacity();
        const double toOpacity = in ? 1 : 0;
        const double height = geometry().height;
        const double fromMargin = in ? -height : m_bottomMargin;
        const double toMargin = in ? 0 : -height;
        m_animation->onUpdate = [this, fromOpacity, toOpacity, fromMargin, toMargin](double value) {
            setOpacity(fromOpacity + (toOpacity - fromOpacity) * value);
            m_bottomMargin = fromMargin + (toMargin - fromMargin) * value;
            m_changed();
        };
        if (!in) {
            // delegate.destroy(), after the frame that shows it gone.
            m_animation->onFinished = [this] {
                Application::instance()->loop().post([self = std::weak_ptr<Bubble*>(m_self)] {
                    if (auto bubble = self.lock())
                        (*bubble)->removeFromParent();
                });
            };
        }
        m_bottomMargin = fromMargin;
        m_animation->start();
    }

    void restartTimer()
    {
        stopTimer();
        m_timer = Application::instance()->loop().startTimer(m_timeout, false, [this] {
            m_timer = 0;
            close();
        });
    }

    void stopTimer()
    {
        if (m_timer)
            Application::instance()->loop().stopTimer(m_timer);
        m_timer = 0;
    }

    void removeFromParent()
    {
        View* parent = this->parent();
        std::function<void()> changed = m_changed;
        parent->takeChild(this).reset();
        changed();
    }

    Label* m_label;
    Button* m_button = nullptr;
    std::chrono::milliseconds m_timeout;
    std::function<void()> m_changed;
    std::unique_ptr<Animation> m_animation;
    std::shared_ptr<Bubble*> m_self = std::make_shared<Bubble*>(this);
    double m_bottomMargin = 0;
    int m_timer = 0;
    bool m_closing = false;
    bool m_pressed = false;
};

// The popup at the bottom centre: smallSpacing around its column of
// messages and a largeSpacing below, largeSpacing above the window's bottom.
class Toast::Stack : public View
{
public:
    void add(std::unique_ptr<Bubble> bubble)
    {
        std::vector<Bubble*> bubbles = this->bubbles();
        for (size_t i = 0; i + kKept < bubbles.size(); ++i)
            bubbles[i]->close();
        Bubble* added = bubble.get();
        addChild(std::move(bubble));
        // Placed now, so the slide starts from the message's own height.
        if (Window* window = this->window())
            setGeometry({0, 0, window->size().width, window->size().height});
        layout();
        added->open();
    }

protected:
    void layout() override
    {
        const Theme& theme = Theme::current();
        const double windowWidth = geometry().width;
        // The newest on top.
        std::vector<Bubble*> bubbles = this->bubbles();
        std::reverse(bubbles.begin(), bubbles.end());
        std::vector<SizeF> sizes;
        double contentWidth = 0;
        double contentHeight = 0;
        for (size_t i = 0; i < bubbles.size(); ++i) {
            sizes.push_back(bubbles[i]->bubbleSize(windowWidth));
            contentWidth = std::max(contentWidth, sizes.back().width);
            contentHeight += (i > 0 ? kLayoutSpacing : 0) + std::max(0.0, sizes.back().height + bubbles[i]->bottomMargin());
        }
        const double popupWidth = contentWidth + 2 * theme.smallSpacing;
        const double popupHeight = contentHeight + theme.smallSpacing + theme.largeSpacing;
        const double popupX = std::round(windowWidth / 2 - popupWidth / 2);
        const double popupY = geometry().height - popupHeight - theme.largeSpacing;
        double y = popupY + theme.smallSpacing;
        for (size_t i = 0; i < bubbles.size(); ++i) {
            const double x = popupX + theme.smallSpacing + (contentWidth - sizes[i].width) / 2;
            bubbles[i]->setGeometry({x, y, sizes[i].width, sizes[i].height});
            y += std::max(0.0, sizes[i].height + bubbles[i]->bottomMargin()) + kLayoutSpacing;
        }
    }

private:
    std::vector<Bubble*> bubbles() const
    {
        std::vector<Bubble*> result;
        for (const auto& child : children())
            result.push_back(static_cast<Bubble*>(child.get()));
        return result;
    }
};

Toast::Toast(Window* window)
    : m_window(window)
{
}

Toast::~Toast()
{
    if (m_stack)
        m_window->overlay()->takeChild(m_stack).reset();
}

void Toast::show(std::string message, std::chrono::milliseconds timeout, std::string actionText,
                 std::function<void()> action)
{
    if (message.empty())
        return;
    if (!m_stack) {
        auto stack = std::make_unique<Stack>();
        m_stack = stack.get();
        m_window->overlay()->addChild(std::move(stack));
    }
    Stack* stack = m_stack;
    m_stack->add(std::make_unique<Bubble>(std::move(message), std::move(actionText), std::move(action), timeout,
                                          [stack] { stack->requestLayout(); }));
}

} // namespace Lingmo::ui
