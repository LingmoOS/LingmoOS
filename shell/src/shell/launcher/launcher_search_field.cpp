// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/launcher/launcher_search_field.h"

#include <cmath>

#include "base/i18n.h"
#include "ui/icon.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/painter.h"

using Lingmo::i18n::tr;

namespace Lingmo::Shell {

static const ui::Color kWhite {255, 255, 255, 255};

// The text field itself, drawn without its frame over the pill.
class LauncherSearchField::Field final : public ui::TextField
{
public:
    explicit Field(LauncherSearchField *owner)
        : m_owner(owner)
    {
    }

protected:
    void focusIn(ui::FocusReason reason) override
    {
        ui::TextField::focusIn(reason);
        m_owner->focusChanged(true);
    }

    void focusOut() override
    {
        ui::TextField::focusOut();
        m_owner->focusChanged(false);
    }

    bool keyPress(const ui::KeyEvent &event) override
    {
        switch (event.key) {
        case ui::key::Escape:
        case ui::key::Up:
        case ui::key::Down:
        case ui::key::Return:
        case ui::key::KeypadEnter:
            // Left and right stay with the caret.
            if (m_owner->onKey && m_owner->onKey(event.key))
                return true;
            break;
        default:
            break;
        }
        return ui::TextField::keyPress(event);
    }

    void textInput(const ui::TextInputEvent &event) override
    {
        ui::TextField::textInput(event);
        // The placeholder's label goes while a preedit shows.
        m_owner->updatePlaceholder();
    }

private:
    LauncherSearchField *m_owner;
};

LauncherSearchField::LauncherSearchField()
{
    setName("launcher-search");
    m_field = addChild<Field>(this);
    m_field->setFrameVisible(false);
    m_field->setTextColor(kWhite);
    m_field->setSelectByMouse(true);
    m_field->onTextChanged = [this] {
        updatePlaceholder();
        if (onTextChanged)
            onTextChanged();
    };

    m_placeholder = addChild<ui::View>();
    m_placeholder->setOpacity(0.6);
    m_glyph = m_placeholder->addChild<ui::Icon>("search");
    m_glyph->setColor(kWhite);
    m_label = m_placeholder->addChild<ui::Label>(tr("Search"));
    m_label->setColor(kWhite);
}

LauncherSearchField::~LauncherSearchField() = default;

const std::string &LauncherSearchField::text() const
{
    return m_field->text();
}

void LauncherSearchField::setText(std::string text)
{
    m_field->setText(std::move(text));
}

bool LauncherSearchField::hasFieldFocus() const
{
    return m_field->hasFocus();
}

void LauncherSearchField::focusField()
{
    m_field->setFocus();
}

void LauncherSearchField::retranslate()
{
    m_label->setText(tr("Search"));
    layoutPlaceholder();
}

void LauncherSearchField::setMetrics(const LauncherMetrics &metrics)
{
    if (metrics == m_metrics)
        return;
    m_metrics = metrics;
    m_field->setFontSize(metrics.labelPixelSize);
    m_label->setFontSize(metrics.labelPixelSize);
    m_glyph->setSize(metrics.searchIconSize);
    requestLayout();
}

void LauncherSearchField::focusChanged(bool focused)
{
    if (focused == m_focused)
        return;
    m_focused = focused;
    // Keeps the typed text on the same x as the placeholder label: the
    // margin, the glyph and the row's spacing.
    requestLayout();

    ui::Window *animating = m_animated ? window() : nullptr;
    const double progress = focused ? 1 : 0;
    m_focusTween.start(animating, m_focusProgress, progress, std::chrono::milliseconds(220), easing::outCubic,
                       [this](double value) {
                           m_focusProgress = value;
                           layoutPlaceholder();
                       });
    m_opacityTween.start(animating, m_backgroundOpacity, focused ? 0.28 : 0.18, std::chrono::milliseconds(200),
                         easing::outCubic, [this](double value) {
                             m_backgroundOpacity = value;
                             requestPaint();
                         });
    m_scaleTween.start(animating, m_backgroundScale, focused ? 1.0 : 0.985, std::chrono::milliseconds(260),
                       easing::outBack, [this](double value) {
                           m_backgroundScale = value;
                           requestPaint();
                       });
    m_glyphTween.start(animating, m_glyphScale, focused ? 1.1 : 1.0, std::chrono::milliseconds(260),
                       easing::outBack, [this](double value) {
                           m_glyphScale = value;
                           m_glyph->setTransform({{}, value});
                       });
    requestPaint();
}

void LauncherSearchField::updatePlaceholder()
{
    const bool visible = m_field->text().empty() && m_field->preedit().empty();
    if (visible == m_label->isVisible())
        return;
    m_label->setVisible(visible);
    layoutPlaceholder();
}

void LauncherSearchField::layoutPlaceholder()
{
    const double height = geometry().height;
    const double iconSize = m_metrics.searchIconSize;
    ui::SizeF label = m_label->sizeHint();
    // The row takes its items' widths in whole pixels.
    label.width = std::ceil(label.width);
    const double width = iconSize + (m_label->isVisible() ? m_metrics.searchIconGap + label.width : 0);
    const double centred = (geometry().width - width) / 2;
    const double margin = 12;
    const double x = std::round(centred + (margin - centred) * m_focusProgress);
    m_placeholder->setGeometry({x, 0, width, height});
    m_glyph->setGeometry({0, (height - iconSize) / 2, iconSize, iconSize});
    m_label->setGeometry({iconSize + m_metrics.searchIconGap, (height - label.height) / 2, label.width,
                          label.height});
}

void LauncherSearchField::layout()
{
    const ui::RectF bounds = localRect();
    const double margin = 12;
    m_field->setTextMargins(m_focused ? margin + m_metrics.searchIconSize + m_metrics.searchIconGap : margin,
                            margin);
    m_field->setGeometry(bounds);
    layoutPlaceholder();
}

void LauncherSearchField::paint(ui::Painter &painter)
{
    // A pill scaled about its centre, its border on the device grid.
    const ui::RectF bounds = localRect();
    const double width = bounds.width * m_backgroundScale;
    const double height = bounds.height * m_backgroundScale;
    const ui::RectF pill {(bounds.width - width) / 2, (bounds.height - height) / 2, width, height};
    painter.save();
    painter.multiplyOpacity(m_backgroundOpacity);
    painter.drawRoundedRect(pill, {height / 2, 0, ui::Corners::Circular}, kWhite, m_focused ? 1 : 0,
                            ui::Color {255, 255, 255, 89});
    painter.restore();
}

} // namespace Lingmo::Shell
