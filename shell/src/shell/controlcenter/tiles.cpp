// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/controlcenter/tiles.h"

#include <algorithm>
#include <cmath>

#include "base/i18n.h"
#include "base/url.h"
#include "shell/controlcenter/panel_style.h"
#include "ui/icon.h"
#include "ui/geometry.h"
#include "ui/image_view.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/theme.h"

using Lingmo::i18n::tr;

namespace Lingmo::Shell {

// How long a press is held before it counts as press-and-hold.
static constexpr std::chrono::milliseconds kPressAndHold {800};

// The light/ artwork's ink, for glyphs drawn in its place.
static constexpr ui::Color kLightArtwork = ui::Color::rgb(0x383838);
static constexpr ui::Color kWhite = ui::Color::rgb(0xFFFFFF);

void SvgIcon::setFile(std::string file)
{
    if (file == m_file)
        return;
    m_file = std::move(file);
    requestPaint();
}

void SvgIcon::setTint(std::optional<ui::Color> tint)
{
    m_tint = tint;
    requestPaint();
}

void SvgIcon::paint(ui::Painter &painter)
{
    if (m_file.empty())
        return;
    // On whole device pixels, as FishUI's IconItem puts its pixmap.
    const ui::Rect device = painter.snappedRect(localRect());
    const double scale = painter.scale();
    const ui::PointF offset = painter.offset();
    painter.drawSvg(m_file,
                    {device.x / scale - offset.x, device.y / scale - offset.y, device.width / scale,
                     device.height / scale},
                    m_tint);
}

CardItem::CardItem()
{
    m_svg = addChild<SvgIcon>();
    m_glyph = addChild<ui::Icon>(std::string(), kIconSize);
    m_glyph->setVisible(false);
    m_title = addChild<ui::Label>();
    m_title->setElide(ui::Label::Elide::Right);
    m_status = addChild<ui::Label>();
    m_status->setElide(ui::Label::Elide::Right);
    m_holdTimer.setSingleShot(true);
    m_holdTimer.setInterval(kPressAndHold);
    m_holdTimer.onTimeout = [this] {
        if (!m_pressed)
            return;
        m_held = true;
        if (onPressAndHold)
            onPressAndHold();
    };
}

CardItem::~CardItem() = default;

void CardItem::setArtwork(std::string name)
{
    m_artwork = std::move(name);
    m_glyph->setVisible(false);
    m_svg->setVisible(true);
    updateIcon();
}

void CardItem::setGlyph(std::string name)
{
    m_artwork.clear();
    m_glyph->setIconName(std::move(name));
    m_glyph->setVisible(true);
    m_svg->setVisible(false);
    updateIcon();
}

void CardItem::setChecked(bool checked)
{
    if (checked == m_checked)
        return;
    m_checked = checked;
    updateIcon();
    requestPaint();
}

void CardItem::setTitle(std::string title)
{
    if (title != m_title->text()) {
        m_title->setText(std::move(title));
        requestLayout();
    }
}

void CardItem::setStatus(std::string status)
{
    if (status != m_status->text()) {
        m_status->setText(std::move(status));
        requestLayout();
    }
}

void CardItem::updateIcon()
{
    const bool white = ::Lingmo::Shell::Panel::dark() || m_checked;
    if (!m_artwork.empty())
        m_svg->setFile(::Lingmo::Shell::Panel::artwork(m_artwork, white));
    m_glyph->setColor(white ? kWhite : kLightArtwork);
}

void CardItem::layout()
{
    const ui::Theme &theme = ui::Theme::current();
    // A theme change relays the card out; the artwork follows it here.
    updateIcon();
    const double padding = theme.largeSpacing;
    const double height = geometry().height;
    const ui::RectF slot {padding, (height - kPlateSize) / 2, kPlateSize, kPlateSize};
    const ui::RectF icon {slot.x + (kPlateSize - kIconSize) / 2, slot.y + (kPlateSize - kIconSize) / 2, kIconSize,
                          kIconSize};
    m_svg->setGeometry(icon);
    m_glyph->setGeometry(icon);

    m_status->setFontSize(theme.smallFontSize());
    m_status->setColor(::Lingmo::Shell::Panel::secondaryTextColor());
    m_status->setVisible(!m_status->text().empty());
    const double x = slot.x + kPlateSize + theme.smallSpacing * 1.5;
    const double width = std::max(0.0, geometry().width - x - padding);
    const double titleHeight = m_title->sizeHint().height;
    const double statusHeight = m_status->isVisible() ? m_status->sizeHint().height : 0;
    const double top = (height - titleHeight - statusHeight) / 2;
    m_title->setGeometry({x, top, width, titleHeight});
    m_status->setGeometry({x, top + titleHeight, width, statusHeight});
}

void CardItem::paint(ui::Painter &painter)
{
    ::Lingmo::Shell::Panel::paintTile(painter, localRect());
    const ui::Color surface = ::Lingmo::Shell::Panel::tileInkColor();
    const bool darkSurface = ::Lingmo::Shell::Panel::isDark(surface);
    const ui::Color highlight = ui::Theme::current().highlight;
    ui::Color plate;
    if (m_checked)
        plate = m_pressed ? ::Lingmo::Shell::Panel::shade(highlight, 0.2) : highlight;
    else
        plate = ::Lingmo::Shell::Panel::on(surface, m_pressed ? (darkSurface ? 0.25 : 0.2) : (darkSurface ? 0.15 : 0.1));
    const double padding = ui::Theme::current().largeSpacing;
    const ui::RectF slot {padding, (geometry().height - kPlateSize) / 2, kPlateSize, kPlateSize};
    const ui::RectF rect = ::Lingmo::Shell::Panel::parityPlate(slot, kIconSize, painter.scale());
    painter.fillRoundedRect(rect, rect.width / 2, plate);
}

void CardItem::pointerPress(const ui::PointerEvent &event)
{
    if (event.button != ui::MouseButton::Left)
        return;
    m_pressed = true;
    m_held = false;
    m_holdTimer.start();
    requestPaint();
}

void CardItem::pointerRelease(const ui::PointerEvent &event)
{
    if (event.button != ui::MouseButton::Left || !m_pressed)
        return;
    m_pressed = false;
    m_holdTimer.stop();
    requestPaint();
    const ui::RectF rect = localRect();
    const bool inside = event.position.x >= 0 && event.position.y >= 0 && event.position.x < rect.width
            && event.position.y < rect.height;
    // A press that was held has had its action.
    if (inside && !m_held && onClicked)
        onClicked();
}

IconButton::IconButton()
    : m_surface(ui::Theme::current().background)
{
    m_icon = addChild<SvgIcon>();
    setTracksHover(true);
}

void IconButton::setFile(std::string file)
{
    m_icon->setFile(std::move(file));
}

void IconButton::setSurfaceColor(ui::Color color)
{
    m_surface = color;
    requestPaint();
}

void IconButton::layout()
{
    const double size = geometry().height * 0.75;
    m_icon->setGeometry({(geometry().width - size) / 2, (geometry().height - size) / 2, size, size});
}

void IconButton::paint(ui::Painter &painter)
{
    if (!isEnabled() || !isHovered())
        return;
    const bool dark = ::Lingmo::Shell::Panel::isDark(m_surface);
    const bool pressed = m_pressed && m_inside;
    const ui::Color color = ::Lingmo::Shell::Panel::on(m_surface, pressed ? (dark ? 0.3 : 0.2) : (dark ? 0.2 : 0.1));
    const ui::RectF rect = localRect();
    painter.fillRoundedRect(rect, rect.height / 2, color);
}

void IconButton::hoverChanged()
{
    requestPaint();
}

void IconButton::enabledChanged()
{
    if (!isEnabled())
        m_pressed = false;
    requestPaint();
}

void IconButton::pointerPress(const ui::PointerEvent &event)
{
    if (event.button != ui::MouseButton::Left || !isEnabled())
        return;
    m_pressed = true;
    m_inside = true;
    requestPaint();
}

void IconButton::pointerMove(const ui::PointerEvent &event)
{
    if (!m_pressed)
        return;
    const ui::RectF rect = localRect();
    const bool inside = event.position.x >= 0 && event.position.y >= 0 && event.position.x < rect.width
            && event.position.y < rect.height;
    if (inside != m_inside) {
        m_inside = inside;
        requestPaint();
    }
}

void IconButton::pointerRelease(const ui::PointerEvent &event)
{
    if (event.button != ui::MouseButton::Left || !m_pressed)
        return;
    pointerMove(event);
    m_pressed = false;
    requestPaint();
    if (m_inside && isEnabled() && onClicked)
        onClicked();
}

// ── ExpandButton ──────────────────────────────────────────────────────────────

ExpandButton::ExpandButton()
{
    m_label = addChild<ui::Label>();
    m_label->setFontSize(12);
    m_label->setElide(ui::Label::Elide::Right);
    setExpanded(false);
}

void ExpandButton::setExpanded(bool expanded)
{
    m_expanded = expanded;
    requestPaint();
}

void ExpandButton::setLabel(std::string more, std::string less)
{
    m_label->setText(m_expanded ? std::move(less) : std::move(more));
}

void ExpandButton::layout()
{
    const ui::Theme &theme = ui::Theme::current();
    const double h      = geometry().height;
    const double chevW  = 12;
    const double gap    = theme.smallSpacing;
    const double labelH = m_label->sizeHint().height;
    m_label->setGeometry({gap * 2, (h - labelH) / 2.0,
                          std::max(0.0, geometry().width - chevW - gap * 4), labelH});
}

void ExpandButton::paint(ui::Painter &painter)
{
    const ui::Color surface = ::Lingmo::Shell::Panel::tileInkColor();
    const bool dark = ::Lingmo::Shell::Panel::isDark(surface);

    if (m_hovered)
        painter.fillRoundedRect(localRect(), ui::Theme::current().smallRadius,
                                ::Lingmo::Shell::Panel::on(surface, dark ? 0.12 : 0.07));

    // Hand-drawn chevron: ∨ when collapsed, ∧ when expanded.
    const double h     = geometry().height;
    const double w     = geometry().width;
    const double gap   = ui::Theme::current().smallSpacing;
    const double chevW = 12;
    const double cx    = w - chevW / 2.0 - gap;
    const double cy    = h / 2.0;
    const double arm   = chevW * 0.36;
    const ui::Color ink = ::Lingmo::Shell::Panel::on(surface, dark ? 0.7 : 0.45);
    if (m_expanded) {
        // ∧
        painter.drawLine({cx - arm, cy + arm * 0.5}, {cx,         cy - arm * 0.5}, ink, 1.5);
        painter.drawLine({cx,       cy - arm * 0.5}, {cx + arm,   cy + arm * 0.5}, ink, 1.5);
    } else {
        // ∨
        painter.drawLine({cx - arm, cy - arm * 0.5}, {cx,         cy + arm * 0.5}, ink, 1.5);
        painter.drawLine({cx,       cy + arm * 0.5}, {cx + arm,   cy - arm * 0.5}, ink, 1.5);
    }
}

void ExpandButton::pointerEnter()
{
    m_hovered = true;
    requestPaint();
}

void ExpandButton::pointerLeave()
{
    m_hovered = false;
    requestPaint();
}

void ExpandButton::pointerRelease(const ui::PointerEvent &event)
{
    if (localRect().contains(event.position) && onClicked)
        onClicked();
}

// MprisItem's buttons: 28 px each, 6 apart.
static constexpr double kButtonSize = 28;

MediaTile::MediaTile()
{
    m_cover = addChild<SvgIcon>();
    m_art = addChild<ui::ImageView>();
    m_art->setFillMode(ui::ImageView::FillMode::PreserveAspectFit);
    m_art->setCornerRadius(ui::Theme::current().smallRadius, ui::Corners::Continuous);
    m_art->setVisible(false);
    m_art->onStatusChanged = [this](ui::ImageView::Status status) {
        m_artReady = status == ui::ImageView::Status::Ready;
        m_art->setVisible(m_artReady);
        m_cover->setVisible(!m_artReady);
        requestPaint();
    };
    m_title = addChild<ui::Label>();
    m_title->setElide(ui::Label::Elide::Right);
    m_artist = addChild<ui::Label>();
    m_artist->setElide(ui::Label::Elide::Right);
    m_artist->setOpacity(0.6);
    m_previous = addChild<IconButton>();
    m_play = addChild<IconButton>();
    m_next = addChild<IconButton>();
    m_previous->onClicked = [this] {
        if (onPrevious)
            onPrevious();
    };
    m_play->onClicked = [this] {
        if (m_state.playing ? bool(onPause) : bool(onPlay))
            (m_state.playing ? onPause : onPlay)();
    };
    m_next->onClicked = [this] {
        if (onNext)
            onNext();
    };
    setState({});
}

void MediaTile::setState(const MediaState &state)
{
    m_state = state;
    const bool control = state.hasPlayer && state.canControl;
    m_previous->setEnabled(control && state.canGoPrevious);
    m_play->setEnabled(control && (state.playing ? state.canPause : state.canPlay));
    m_next->setEnabled(control && state.canGoNext);
    for (IconButton *button : {m_previous, m_play, m_next})
        button->setOpacity(button->isEnabled() ? 1 : 0.35);
    std::string title = tr("Not Playing");
    if (state.hasPlayer)
        title = !state.title.empty() ? state.title : state.identity;
    m_title->setText(std::move(title));
    m_artist->setText(state.hasPlayer ? state.artist : std::string());
    updateArtwork();
    requestLayout();
}

void MediaTile::updateArtwork()
{
    const bool white = ::Lingmo::Shell::Panel::dark();
    m_cover->setFile(::Lingmo::Shell::Panel::artwork("media-cover", white));
    m_previous->setFile(::Lingmo::Shell::Panel::artwork("media-skip-backward-symbolic", white));
    m_play->setFile(::Lingmo::Shell::Panel::artwork(m_state.playing ? "media-playback-pause-symbolic"
                                                   : "media-playback-start-symbolic",
                                   white));
    m_next->setFile(::Lingmo::Shell::Panel::artwork("media-skip-forward-symbolic", white));
    for (IconButton *button : {m_previous, m_play, m_next})
        button->setSurfaceColor(::Lingmo::Shell::Panel::tileInkColor());

    // Album art from a file; anything else keeps the default cover.
    std::string path;
    if (m_state.hasPlayer && base::url::isLocalFile(m_state.artUrl))
        path = base::url::toLocalFile(m_state.artUrl).value_or(std::string());
    else if (m_state.hasPlayer && m_state.artUrl.starts_with("/"))
        path = m_state.artUrl;
    if (path != m_art->source()) {
        m_artReady = false;
        m_art->setVisible(false);
        m_cover->setVisible(true);
        m_art->setSource(path);
    }
}

void MediaTile::layout()
{
    const ui::Theme &theme = ui::Theme::current();
    updateArtwork();
    const double padding = theme.largeSpacing;
    const double content = geometry().height - 2 * padding;
    const ui::RectF art {padding, padding, content, content};
    m_cover->setGeometry(art);
    m_art->setGeometry(art);
    m_art->setSourceSize({content, content});

    // The last glyph lines up with the sliders' tracks: the artwork keeps 3.125
    // of its 16 units clear, and 2px more make up for the tracks' rounded ends.
    const double iconInset = (kButtonSize - kButtonSize * 0.75) / 2;
    const double rightMargin = -iconInset - (kButtonSize - iconInset * 2) * 3.125 / 16 + 2;
    const double spacing = theme.smallSpacing;
    const double buttonsWidth = 3 * kButtonSize + 2 * spacing;
    const double buttonsX = geometry().width - padding - rightMargin - buttonsWidth;
    const double buttonY = padding + (content - kButtonSize) / 2;
    m_previous->setGeometry({buttonsX, buttonY, kButtonSize, kButtonSize});
    m_play->setGeometry({buttonsX + kButtonSize + spacing, buttonY, kButtonSize, kButtonSize});
    m_next->setGeometry({buttonsX + 2 * (kButtonSize + spacing), buttonY, kButtonSize, kButtonSize});

    m_artist->setFontSize(theme.smallFontSize());
    m_artist->setVisible(!m_artist->text().empty());
    const double x = art.x + art.width + padding;
    const double width = std::max(0.0, buttonsX - padding - x);
    const double titleHeight = m_title->sizeHint().height;
    const double artistHeight = m_artist->isVisible() ? m_artist->sizeHint().height : 0;
    const double top = padding + (content - titleHeight - artistHeight) / 2;
    m_title->setGeometry({x, top, width, titleHeight});
    m_artist->setGeometry({x, top + titleHeight, width, artistHeight});
}

void MediaTile::paint(ui::Painter &painter)
{
    ::Lingmo::Shell::Panel::paintTile(painter, localRect());
    if (m_artReady)
        return;
    // The cards' plate colour under the default cover, inside its mask.
    const ui::Color surface = ::Lingmo::Shell::Panel::tileInkColor();
    const ui::RectF art = m_cover->geometry();
    painter.fillRoundedRect(art, ui::Theme::current().smallRadius,
                            ::Lingmo::Shell::Panel::on(surface, ::Lingmo::Shell::Panel::isDark(surface) ? 0.15 : 0.1), ui::Corners::Continuous);
}

std::string deviceUsageText(const DeviceUser &user)
{
    // Whole phrases rather than joined parts, so each translates as one.
    if (user.camera && user.microphone && user.systemAudio)
        // TRANSLATORS: follows an application's name shown before it, as do the other "is using" phrases.
        return tr("is using the camera and microphone and recording system audio");
    if (user.camera && user.microphone)
        return tr("is using the camera and microphone");
    if (user.camera && user.systemAudio)
        return tr("is using the camera and recording system audio");
    if (user.microphone && user.systemAudio)
        return tr("is using the microphone and recording system audio");
    if (user.camera)
        return tr("is using the camera");
    if (user.microphone)
        return tr("is using the microphone");
    return tr("is recording system audio");
}

DeviceUsersTile::DeviceUsersTile() = default;

void DeviceUsersTile::setUsers(const std::vector<DeviceUser> &users)
{
    if (users == m_users)
        return;
    m_users = users;
    for (Row &row : m_rows) {
        takeChild(row.icon);
        takeChild(row.name);
        takeChild(row.usage);
    }
    m_rows.clear();
    for (const DeviceUser &user : m_users) {
        Row row;
        row.icon = addChild<ui::ThemeIcon>(user.icon.empty() ? "application-x-executable" : user.icon,
                                           kIconSize);
        row.name = addChild<ui::Label>(user.name);
        row.name->setElide(ui::Label::Elide::Right);
        row.usage = addChild<ui::Label>(" " + deviceUsageText(user));
        row.dot = user.camera ? ui::accent::green : user.microphone ? ui::accent::orange : ui::accent::purple;
        m_rows.push_back(row);
    }
    requestLayout();
}

void DeviceUsersTile::retranslate()
{
    for (size_t i = 0; i < m_rows.size(); ++i)
        m_rows[i].usage->setText(" " + deviceUsageText(m_users[i]));
    requestLayout();
}

double DeviceUsersTile::inset() const
{
    // The icon centred where the cards' plates are, its gap to the tile's
    // edges the same on every side.
    return ui::Theme::current().largeSpacing + (CardItem::kPlateSize - kIconSize) / 2;
}

double DeviceUsersTile::rowHeight() const
{
    double height = kIconSize;
    for (const Row &row : m_rows)
        height = std::max(height, row.name->sizeHint().height);
    return height;
}

double DeviceUsersTile::heightForWidth(double) const
{
    if (m_rows.empty())
        return 0;
    const double spacing = ui::Theme::current().smallSpacing * 1.5;
    return 2 * inset() + m_rows.size() * rowHeight() + (m_rows.size() - 1) * spacing;
}

void DeviceUsersTile::layout()
{
    const ui::Theme &theme = ui::Theme::current();
    const double spacing = theme.smallSpacing * 1.5;
    const double left = theme.largeSpacing;
    const double right = geometry().width - inset();
    const double height = rowHeight();
    double y = inset();
    for (const Row &row : m_rows) {
        row.icon->setGeometry({left + (CardItem::kPlateSize - kIconSize) / 2, y + (height - kIconSize) / 2,
                               kIconSize, kIconSize});
        const double sentenceX = left + CardItem::kPlateSize + spacing;
        // The dot keeps the row's end; only the name gives way to a narrow panel.
        const double sentenceWidth = std::max(0.0, right - 6 - spacing - sentenceX);
        const double usageWidth = row.usage->sizeHint().width;
        const double nameWidth = std::min(row.name->sizeHint().width, std::max(0.0, sentenceWidth - usageWidth));
        const double nameHeight = row.name->sizeHint().height;
        row.name->setGeometry({sentenceX, y + (height - nameHeight) / 2, nameWidth, nameHeight});
        row.usage->setGeometry({sentenceX + nameWidth, y + (height - nameHeight) / 2, usageWidth, nameHeight});
        y += height + spacing;
    }
}

void DeviceUsersTile::paint(ui::Painter &painter)
{
    ::Lingmo::Shell::Panel::paintTile(painter, localRect());
    const double spacing = ui::Theme::current().smallSpacing * 1.5;
    const double height = rowHeight();
    double y = inset();
    for (const Row &row : m_rows) {
        painter.fillRoundedRect({geometry().width - inset() - 6, y + (height - 6) / 2, 6, 6}, 3, row.dot);
        y += height + spacing;
    }
}

} // namespace Lingmo::Shell
