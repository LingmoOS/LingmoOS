// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/button.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <utility>

#include "ui/animation.h"
#include "ui/color_provider.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/theme.h"

namespace Lingmo::ui {

static constexpr double kMinimumWidth = 64;
static constexpr double kMinimumHeight = 32;
static constexpr double kFocusInset = 3;

Button::Button(std::string text)
    : m_label(addChild<Label>(std::move(text)))
{
    m_label->setTextAlignment(Label::TextAlignment::Center);
    m_label->setVerticallyCentered(true);
    // A button colours its icon and label instead of fading them.
    m_label->setFadesWhenDisabled(false);
}

Button::~Button() = default;

void Button::setText(std::string text)
{
    m_label->setText(std::move(text));
}

SizeF Button::sizeHint() const
{
    const Theme& theme = Theme::current();
    const SizeF text = m_label->sizeHint();
    return {std::max(kMinimumWidth, text.width + 2 * theme.largeSpacing),
            std::max(kMinimumHeight, text.height + 2 * theme.smallSpacing)};
}

RectF Button::visualRect() const
{
    return localRect().adjusted(-kFocusInset, -kFocusInset, kFocusInset, kFocusInset);
}

void Button::layout()
{
    const Theme& theme = Theme::current();
    const double padding = theme.largeSpacing;
    m_label->setGeometry(localRect().adjusted(padding, 0, -padding, 0));
    m_label->setColor(isEnabled() ? std::nullopt : std::optional<Color>(theme.disabledText));
}

void Button::enabledChanged()
{
    if (!isEnabled()) {
        m_pressed = false;
        if (m_rippleAnimation)
            m_rippleAnimation->stop();
        if (m_rippleFadeAnimation)
            m_rippleFadeAnimation->stop();
        m_rippleOpacity = 0;
        animateHover(0);
    }
    requestLayout();
    requestPaint();
}

void Button::paint(Painter& painter)
{
    const Theme& theme = Theme::current();
    const bool enabled = isEnabled();
    const RectF bounds = localRect();
    const double radius = theme.mediumRadius;

    if (theme.controlStyle == 1) {
        // Liquid Glass: semi-transparent glass surface with an edge highlight.
        Color surface = color(ColorId::ButtonBackground);
        if (!enabled)
            surface = surface.withAlpha(surface.a / 255.0 * 0.28);

        // Drop shadow for lift effect.
        if (enabled)
            painter.drawShadow(bounds, radius, 6, Color{0, 0, 0, uint8_t(theme.darkMode ? 60 : 40)}, {0, 1});

        painter.fillRoundedRect(bounds, radius, surface, themeCorners());

        // Top-edge specular highlight (the lit rim of the glass).
        const double rimAlpha = enabled ? (m_pressed ? 0.18 : (m_hoverAmount > 0 ? 0.32 + m_hoverAmount * 0.12 : 0.24)) : 0.10;
        const Color rimColor = Color{255, 255, 255, uint8_t(rimAlpha * 255)};
        // A thin line along the top edge using strokeRoundedRect inset by half a pixel.
        painter.strokeRoundedRect(bounds.adjusted(0.5, 0.5, -0.5, -0.5), radius - 0.5, rimColor,
                                  painter.borderWidth(1), themeCorners());

        // Press: add a brief brightness shift.
        if (enabled && m_pressed)
            painter.fillRoundedRect(bounds, radius, Color{0, 0, 0, 20}, themeCorners());

        // Ripple.
        if (enabled && m_rippleOpacity > 0 && m_rippleProgress > 0) {
            const double farX = std::max(m_rippleOrigin.x, bounds.width - m_rippleOrigin.x);
            const double farY = std::max(m_rippleOrigin.y, bounds.height - m_rippleOrigin.y);
            const double maxR = std::hypot(farX, farY);
            const double rNow = std::max(1.0 / painter.scale(), maxR * m_rippleProgress);
            painter.save();
            painter.clipRoundedRect(bounds, radius, themeCorners());
            painter.fillEllipse({m_rippleOrigin.x - rNow, m_rippleOrigin.y - rNow, rNow * 2, rNow * 2},
                                 Color{255, 255, 255, uint8_t(0.14 * m_rippleOpacity * 255)});
            painter.restore();
        }
    } else {
        // Classic.
        Color surface = color(ColorId::ButtonBackground);
        if (!enabled)
            surface = surface.withAlpha(surface.a / 255.0 * 0.38);
        painter.fillRoundedRect(bounds, radius, surface, themeCorners());

        // 1px border for edge definition in classic mode (transparent = no-op).
        const Color border = color(ColorId::ButtonBorder);
        if (border.a > 0)
            painter.strokeRoundedRect(bounds.adjusted(0.5, 0.5, -0.5, -0.5), radius - 0.5,
                                      border.withAlpha(enabled ? border.a / 255.0 : border.a / 255.0 * 0.38),
                                      1, themeCorners());

        if (enabled) {
            const double stateOpacity = std::max(m_hoverAmount * 0.08, m_pressed ? 0.12 : 0.0);
            if (stateOpacity > 0)
                painter.fillRoundedRect(bounds, radius, theme.highlight.withAlpha(stateOpacity), themeCorners());
            if (m_rippleOpacity > 0 && m_rippleProgress > 0) {
                const double farX = std::max(m_rippleOrigin.x, bounds.width - m_rippleOrigin.x);
                const double farY = std::max(m_rippleOrigin.y, bounds.height - m_rippleOrigin.y);
                const double maxRadius = std::hypot(farX, farY);
                const double radiusNow = std::max(1.0 / painter.scale(), maxRadius * m_rippleProgress);
                painter.save();
                painter.clipRoundedRect(bounds, radius, themeCorners());
                painter.fillEllipse({m_rippleOrigin.x - radiusNow, m_rippleOrigin.y - radiusNow,
                                     radiusNow * 2, radiusNow * 2},
                                    theme.highlight.withAlpha(0.16 * m_rippleOpacity));
                painter.restore();
            }
        }
    }

    if (hasVisibleFocus()) {
        painter.strokeRoundedRect(bounds.adjusted(-kFocusInset, -kFocusInset, kFocusInset, kFocusInset),
                                  radius + kFocusInset, color(ColorId::ButtonFocusRing),
                                  painter.borderWidth(2), themeCorners());
    }
}

void Button::animateHover(double target)
{
    if (!window()) {
        m_hoverAmount = target;
        requestPaint();
        return;
    }
    if (!m_hoverAnimation) {
        m_hoverAnimation = std::make_unique<Animation>(window());
        m_hoverAnimation->setEasing(Animation::Easing::OutCubic);
        m_hoverAnimation->onUpdate = [this](double value) {
            m_hoverAmount = value;
            requestPaint();
        };
    }
    const double from = m_hoverAmount;
    m_hoverAnimation->stop();
    m_hoverAnimation->setDuration(std::chrono::milliseconds(110));
    m_hoverAnimation->onUpdate = [this, from, target](double value) {
        m_hoverAmount = from + (target - from) * value;
        requestPaint();
    };
    m_hoverAnimation->start();
}

void Button::beginRipple(PointF origin)
{
    m_rippleOrigin = origin;
    m_rippleProgress = 0;
    m_rippleOpacity = 1;
    if (m_rippleFadeAnimation)
        m_rippleFadeAnimation->stop();
    if (!window()) {
        m_rippleProgress = 1;
        requestPaint();
        return;
    }
    if (!m_rippleAnimation)
        m_rippleAnimation = std::make_unique<Animation>(window());
    m_rippleAnimation->stop();
    m_rippleAnimation->setDuration(std::chrono::milliseconds(220));
    m_rippleAnimation->setEasing(Animation::Easing::OutCubic);
    m_rippleAnimation->onUpdate = [this](double value) {
        m_rippleProgress = value;
        requestPaint();
    };
    m_rippleAnimation->onFinished = [this] {
        if (!m_pressed && m_rippleOpacity > 0.01)
            fadeRipple();
    };
    m_rippleAnimation->start();
}

void Button::fadeRipple()
{
    if (!window()) {
        m_rippleOpacity = 0;
        requestPaint();
        return;
    }
    const double from = m_rippleOpacity;
    if (!m_rippleFadeAnimation)
        m_rippleFadeAnimation = std::make_unique<Animation>(window());
    m_rippleFadeAnimation->stop();
    m_rippleFadeAnimation->setDuration(std::chrono::milliseconds(150));
    m_rippleFadeAnimation->setEasing(Animation::Easing::OutCubic);
    m_rippleFadeAnimation->onUpdate = [this, from](double value) {
        m_rippleOpacity = from * (1 - value);
        requestPaint();
    };
    m_rippleFadeAnimation->start();
}

void Button::pointerEnter()
{
    animateHover(1);
}

void Button::pointerLeave()
{
    animateHover(0);
}

void Button::pointerPress(const PointerEvent& event)
{
    if (!isEnabled() || event.button != MouseButton::Left)
        return;
    m_pressed = true;
    beginRipple(event.position);
    requestPaint();
}

void Button::pointerRelease(const PointerEvent& event)
{
    if (!m_pressed)
        return;
    m_pressed = false;
    fadeRipple();
    requestPaint();
    if (localRect().contains(event.position) && onClicked)
        onClicked();
}

bool Button::keyPress(const KeyEvent& event)
{
    if (!isEnabled() || event.modifiers != NoModifier)
        return false;
    if (event.key == key::Space) {
        m_pressed = true;
        beginRipple({geometry().width / 2, geometry().height / 2});
        requestPaint();
        return true;
    }
    if (event.key == key::Return || event.key == key::KeypadEnter) {
        beginRipple({geometry().width / 2, geometry().height / 2});
        m_pressed = false;
        fadeRipple();
        if (onClicked)
            onClicked();
        return true;
    }
    return false;
}

bool Button::keyRelease(const KeyEvent& event)
{
    if (!isEnabled() || event.key != key::Space || !m_pressed)
        return false;
    m_pressed = false;
    fadeRipple();
    requestPaint();
    if (onClicked)
        onClicked();
    return true;
}

} // namespace Lingmo::ui
