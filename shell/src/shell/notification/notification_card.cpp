// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/notification/notification_card.h"

#include <algorithm>
#include <cmath>

#include "base/process.h"
#include "shell/notification/notification_controls.h"
#include "shell/notification/notification_motion.h"
#include "ui/icon.h"
#include "ui/image_view.h"
#include "ui/painter.h"
#include "ui/text_markup.h"
#include "ui/theme.h"
#include "toolkit/compositor_colors.h"

namespace Lingmo::Shell {

static constexpr const char *kFallbackIcon = "preferences-desktop-notification";
static constexpr int kBodyLines = 4;
static constexpr double kProgressHeight = 6 + 4;

static double smallFontSize()
{
    return ui::Theme::current().fontPointSize * 0.9 * 4 / 3;
}

class NotificationCard::ActionButton : public ui::View
{
public:
    ActionButton(NotificationCard *card, std::string key, std::string label)
        : m_card(card)
        , m_key(std::move(key))
    {
        m_label = addChild<ui::Label>(std::move(label));
        m_label->setWeight(500);
        m_label->setTextAlignment(ui::Label::TextAlignment::Center);
        m_label->setVerticallyCentered(true);
    }

    bool acceptsPointer() const override { return true; }

protected:
    void pointerEnter() override
    {
        m_hovered = true;
        requestPaint();
    }

    void pointerLeave() override
    {
        m_hovered = false;
        requestPaint();
    }

    void pointerPress(const ui::PointerEvent &event) override
    {
        if (event.button != ui::MouseButton::Left)
            return;
        m_pressed = true;
        requestPaint();
    }

    void pointerRelease(const ui::PointerEvent &event) override
    {
        if (!m_pressed)
            return;
        m_pressed = false;
        requestPaint();
        if (localRect().contains(event.position) && m_card->onActionClicked)
            m_card->onActionClicked(m_key);
    }

    void layout() override
    {
        const double margin = ui::Theme::current().smallSpacing;
        m_label->setGeometry({margin, 0, std::max(0.0, geometry().width - 2 * margin), geometry().height});
    }

    void paint(ui::Painter &painter) override
    {
        const ui::RectF rect = localRect();
        painter.drawRoundedRect(rect, {rect.height / 2, 0, ui::Corners::Circular},
                                m_card->controlTint(m_pressed ? 2 : m_hovered ? 1 : 0));
    }

private:
    NotificationCard *m_card;
    std::string m_key;
    ui::Label *m_label = nullptr;
    bool m_hovered = false;
    bool m_pressed = false;
};

NotificationCard::NotificationCard(ui::Window *window)
    : m_surface(ui::Theme::current().background)
    , m_progress(std::make_unique<Tween>(window, std::chrono::milliseconds(200), outCubic))
{
    setTracksHover(true);

    m_themeIcon = addChild<ui::ThemeIcon>(kFallbackIcon, kIconSize);
    m_fileIcon = addChild<ui::ImageView>();
    m_fileIcon->setFillMode(ui::ImageView::FillMode::PreserveAspectFit);
    m_fileIcon->setVisible(false);

    m_appName = addChild<ui::Label>();
    m_appName->setWeight(500);
    m_when = addChild<ui::Label>();
    m_when->setElide(ui::Label::Elide::None);

    m_title = addChild<ui::Label>();
    m_title->setWeight(700);
    m_body = addChild<ui::Label>();
    m_body->setWrap(true);
    m_body->setMaximumLineCount(kBodyLines);
    m_more = addChild<ui::Label>();

    m_thumbnail = addChild<ui::ImageView>();
    m_thumbnail->setFillMode(ui::ImageView::FillMode::PreserveAspectCrop);
    m_thumbnail->setSourceSize({kThumbnailSize, kThumbnailSize});
    m_thumbnail->setCornerRadius(ui::Theme::current().smallRadius);
    m_thumbnail->setRendering(ui::ImageView::Rendering::MaskedLayer);
    m_thumbnail->setVisible(false);
    m_thumbnail->onStatusChanged = [this](ui::ImageView::Status status) {
        const bool failed = status == ui::ImageView::Status::Error;
        m_thumbnailReady = status == ui::ImageView::Status::Ready;
        if (failed != m_thumbnailFailed) {
            m_thumbnailFailed = failed;
            requestLayout();
            heightMayHaveChanged();
        }
        requestPaint();
    };

    m_close = addChild<ClearButton>(window, ClearButton::Kind::CardClose);
    m_close->setTint([this](int state) { return controlTint(state); });
    m_close->setVisible(false);
    m_close->onClicked = [this] {
        if (onCloseClicked)
            onCloseClicked();
    };
    m_close->onWidthChanged = [this] { requestLayout(); };
    m_close->onArmedChanged = [this] { updateClose(); };

    m_progress->onChanged = [this] { requestPaint(); };
    m_progress->snap(0);
}

NotificationCard::~NotificationCard() = default;

void NotificationCard::setNotification(const Notification &notification)
{
    const Notification previous = m_notification;
    m_notification = notification;

    const std::string &icon = notification.appIcon;
    const bool file = !icon.empty() && icon.front() == '/';
    m_fileIcon->setVisible(file);
    m_themeIcon->setVisible(!file);
    if (file)
        m_fileIcon->setSource(icon);
    else
        m_themeIcon->setIconName(icon.empty() ? kFallbackIcon : icon);

    m_appName->setText(notification.appName);
    m_title->setText(notification.summary);
    m_title->setVisible(!notification.summary.empty());
    updateBody();
    m_body->setVisible(!notification.body.empty());

    if (notification.image != previous.image || !m_thumbnail->isVisible()) {
        m_thumbnailFailed = false;
        m_thumbnailReady = false;
        m_thumbnail->setSource(notification.image);
    }

    // A replacement glides the bar from where it was.
    const double fraction = std::clamp(notification.value, 0, 100) / 100.0;
    m_progress->setTarget(fraction, previous.value >= 0 && notification.value >= 0 && m_progress->window());

    const std::vector<Notification::Action> actions = notification.actionList();
    std::vector<std::pair<std::string, std::string>> wanted;
    for (const Notification::Action &action : actions) {
        if (int(wanted.size()) == kMaxActions)
            break;
        wanted.emplace_back(action.key, action.label);
    }
    for (ActionButton *button : m_actions)
        takeChild(button);
    m_actions.clear();
    for (auto &[key, label] : wanted)
        m_actions.push_back(addChild<ActionButton>(this, key, label));
    // Above the card's own press area, below nothing.
    raiseChild(m_close);

    requestLayout();
    requestPaint();
    heightMayHaveChanged();
}

void NotificationCard::setTimeText(const std::string &text)
{
    if (text == m_timeText)
        return;
    m_timeText = text;
    requestLayout();
}

void NotificationCard::setMoreText(const std::string &text)
{
    if (text == m_moreText)
        return;
    m_moreText = text;
    m_more->setText(text);
    requestLayout();
    heightMayHaveChanged();
}

void NotificationCard::setDetailed(bool detailed)
{
    if (detailed == m_detailed)
        return;
    m_detailed = detailed;
    m_body->setMaximumLineCount(detailed ? 1000 : kBodyLines);
    requestLayout();
    heightMayHaveChanged();
}

void NotificationCard::setConfirmClose(bool confirm)
{
    m_close->setConfirm(confirm);
}

void NotificationCard::setSurfaceColor(ui::Color color)
{
    if (color == m_surface)
        return;
    m_surface = color;
    requestPaint();
    m_close->requestPaint();
    for (ActionButton *button : m_actions)
        button->requestPaint();
}

ui::Color NotificationCard::controlTint(int state) const
{
    return inkIsDark(m_surface) ? inkOn(m_surface, 0.14 + state * 0.07) : inkOn(m_surface, 0.06 + state * 0.04);
}

void NotificationCard::disarmUnless(ui::PointF windowPoint)
{
    m_close->disarmUnless(windowPoint);
}

std::vector<ui::RectF> NotificationCard::actionRects() const
{
    std::vector<ui::RectF> rects;
    for (const ActionButton *button : m_actions)
        rects.push_back(button->geometry());
    return rects;
}

ui::Color NotificationCard::secondaryTextColor() const
{
    return ui::color(toolkit::CompositorColor::NotificationSecondaryText);
}

bool NotificationCard::showsThumbnail() const
{
    return !m_notification.image.empty() && !m_thumbnailFailed;
}

NotificationCard::Geometry NotificationCard::computeGeometry(double width) const
{
    const ui::Theme &theme = ui::Theme::current();
    Geometry g;
    g.closeY = kPadding + (kHeaderHeight - m_close->buttonHeight()) / 2 - 3;
    g.closeX = width - g.closeY - m_close->currentWidth();
    g.headerWidth = g.closeX - theme.smallSpacing - kPadding;

    const bool thumbnail = showsThumbnail();
    g.thumbnailX = width - kPadding - kThumbnailSize;
    g.contentWidth = (thumbnail ? g.thumbnailX - theme.largeSpacing : width - kPadding) - kPadding;

    // The Column: visible children, 2 apart.
    const double top = kPadding + kHeaderHeight + theme.smallSpacing;
    double y = top;
    bool first = true;
    const auto place = [&](double height) {
        if (!first)
            y += 2;
        first = false;
        const double at = y;
        y += height;
        return at;
    };
    if (!m_notification.summary.empty())
        g.titleY = place(m_title->sizeHint().height);
    if (!m_notification.body.empty()) {
        g.bodyHeight = m_body->heightForWidth(g.contentWidth);
        g.bodyY = place(g.bodyHeight);
    }
    if (m_notification.value >= 0)
        g.progressY = place(kProgressHeight);
    if (!m_moreText.empty())
        g.moreY = place(m_more->sizeHint().height);
    g.contentBottom = y;

    g.thumbnailY = top + 2;
    const double bottom = std::max(g.contentBottom, thumbnail ? g.thumbnailY + kThumbnailSize : 0.0);
    if (!m_actions.empty())
        g.actionsY = bottom + theme.largeSpacing;
    g.height = bottom + (m_actions.empty() ? 0 : kActionHeight + theme.largeSpacing) + kPadding;
    return g;
}

double NotificationCard::implicitHeight(double width) const
{
    m_more->setFontSize(smallFontSize());
    return computeGeometry(width).height;
}

void NotificationCard::updateBody()
{
    // The link colour follows the accent.
    const uint64_t generation = ui::Theme::generation();
    if (m_bodyText == m_notification.body && m_bodyTheme == generation)
        return;
    m_bodyText = m_notification.body;
    m_bodyTheme = generation;
    std::vector<ui::Label::Span> spans = ui::styledTextSpans(m_bodyText, ui::Theme::current().highlight);
    // Plain text needs no runs.
    if (spans.size() == 1 && !spans.front().weight && !spans.front().color && !spans.front().italic
        && !spans.front().underline)
        m_body->setText(spans.front().text);
    else
        m_body->setSpans(std::move(spans));
}

void NotificationCard::heightMayHaveChanged()
{
    if (onImplicitHeightChanged)
        onImplicitHeightChanged();
}

void NotificationCard::updateClose()
{
    m_close->setVisible(isHovered() || m_close->isArmed());
    if (!m_close->isVisible())
        m_close->reset();
}

void NotificationCard::hoverChanged()
{
    updateClose();
    if (onHoveredChanged)
        onHoveredChanged();
}

void NotificationCard::pointerPress(const ui::PointerEvent &event)
{
    m_pressed = event.button == ui::MouseButton::Left;
}

void NotificationCard::pointerRelease(const ui::PointerEvent &event)
{
    if (!m_pressed)
        return;
    m_pressed = false;
    if (!localRect().contains(event.position))
        return;
    // A link in the body opens, as Text.onLinkActivated; the card takes the rest.
    if (m_body->isVisible()) {
        const ui::RectF body = m_body->geometry();
        const std::string link = m_body->linkAt({event.position.x - body.x, event.position.y - body.y});
        if (!link.empty()) {
            base::Process::startDetached("xdg-open", {link});
            return;
        }
    }
    if (onClicked)
        onClicked();
}

void NotificationCard::layout()
{
    const ui::Theme &theme = ui::Theme::current();
    const double small = smallFontSize();
    const ui::Color secondary = secondaryTextColor();
    m_appName->setFontSize(small);
    m_appName->setColor(secondary);
    m_when->setFontSize(small);
    m_when->setColor(secondary);
    m_more->setFontSize(small);
    m_more->setColor(secondary);
    updateBody();

    const double width = geometry().width;
    const Geometry g = computeGeometry(width);

    m_close->setGeometry({g.closeX, g.closeY, m_close->currentWidth(), m_close->buttonHeight()});

    // The caption, vertically centred in the header.
    const double iconY = kPadding + (kHeaderHeight - kIconSize) / 2;
    m_themeIcon->setGeometry({kPadding, iconY, kIconSize, kIconSize});
    m_fileIcon->setGeometry({kPadding, iconY, kIconSize, kIconSize});
    // Its width counts even with no time shown.
    m_when->setText((m_notification.appName.empty() ? "" : "  ·  ") + m_timeText);
    const ui::SizeF when = m_when->sizeHint();
    const ui::SizeF name = m_appName->sizeHint();
    const double nameX = kPadding + kIconSize + theme.smallSpacing;
    const double nameWidth = std::max(0.0, std::min(name.width, g.headerWidth - (nameX - kPadding) - when.width));
    const double nameY = kPadding + (kHeaderHeight - name.height) / 2;
    m_appName->setGeometry({nameX, nameY, nameWidth, name.height});
    m_when->setVisible(!m_timeText.empty());
    m_when->setGeometry({nameX + nameWidth, nameY + m_appName->baselineOffset() - m_when->baselineOffset(),
                         when.width, when.height});

    if (g.titleY >= 0)
        m_title->setGeometry({kPadding, g.titleY, g.contentWidth, m_title->sizeHint().height});
    if (g.bodyY >= 0)
        m_body->setGeometry({kPadding, g.bodyY, g.contentWidth, g.bodyHeight});
    m_more->setVisible(g.moreY >= 0);
    if (g.moreY >= 0)
        m_more->setGeometry({kPadding, g.moreY, g.contentWidth, m_more->sizeHint().height});

    m_thumbnail->setVisible(showsThumbnail());
    m_thumbnail->setGeometry({g.thumbnailX, g.thumbnailY, kThumbnailSize, kThumbnailSize});

    if (!m_actions.empty()) {
        const double spacing = theme.smallSpacing;
        const double rowWidth = width - kPadding * 2;
        const double count = double(m_actions.size());
        const double buttonWidth = (rowWidth - spacing * (count - 1)) / count;
        for (size_t i = 0; i < m_actions.size(); ++i) {
            m_actions[i]->setGeometry({kPadding + double(i) * (buttonWidth + spacing), g.actionsY, buttonWidth,
                                       kActionHeight});
        }
    }

    if (g.height != m_lastHeight) {
        m_lastHeight = g.height;
        heightMayHaveChanged();
    }
}

void NotificationCard::paint(ui::Painter &painter)
{
    const Geometry g = computeGeometry(geometry().width);

    // Where the thumbnail goes, while it loads.
    if (showsThumbnail() && !m_thumbnailReady) {
        painter.drawRoundedRect({g.thumbnailX, g.thumbnailY, kThumbnailSize, kThumbnailSize},
                                {ui::Theme::current().smallRadius, 0, ui::Corners::Circular}, controlTint(0));
    }

    if (g.progressY >= 0) {
        const ui::RectF track {kPadding, g.progressY + kProgressHeight - 4, g.contentWidth, 4};
        painter.drawSmoothRectangle(track, 2, controlTint(0));
        const double fill = track.width * m_progress->value();
        if (fill > 0)
            painter.drawSmoothRectangle({track.x, track.y, fill, track.height}, 2, ui::Theme::current().highlight);
    }
}

} // namespace Lingmo::Shell
