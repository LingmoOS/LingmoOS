// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/controlcenter/status_panel.h"

#include <algorithm>
#include <cmath>

#include "base/i18n.h"
#include "shell/controlcenter/level_slider.h"
#include "shell/controlcenter/panel_style.h"
#include "shell/controlcenter/tiles.h"
#include "ui/animation.h"
#include "ui/icon.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/progress_bar.h"
#include "ui/scroll_view.h"
#include "ui/switch.h"
#include "ui/theme.h"
#include "ui/window.h"

using Lingmo::i18n::tr;
using Lingmo::i18n::trNoop;

namespace Lingmo::Shell {

using namespace status_panel;

// The panels' titles: 13 pt.
static constexpr double kTitleFontSize = 13.0 * 4 / 3;
static constexpr double kBadgeSize = 28;
static constexpr double kGlyphSize = 16;
static constexpr double kRowSpacing = 12;
static constexpr double kListSpacing = 2;
static constexpr double kMinimumRowHeight = 36;
// FishUI's Font.DemiBold.
static constexpr int kDemiBold = 600;
static constexpr ui::Color kLightArtwork = ui::Color::rgb(0x383838);
static constexpr ui::Color kWhite = ui::Color::rgb(0xFFFFFF);
// NetworkManager scans only now and then while connected; an open list asks
// for more.
static constexpr std::chrono::milliseconds kScanInterval {15000};

static double windowScale(const ui::View *view)
{
    const ui::Window *window = view->window();
    return window && window->scale() > 0 ? window->scale() : 1;
}

static bool insideRect(const ui::RectF &rect, const ui::PointF &point)
{
    return point.x >= 0 && point.y >= 0 && point.x < rect.width && point.y < rect.height;
}

// CSS's cubic-bezier(x1, y1, x2, y2) at time t, FrameTween's easing.
static double cubicBezier(double x1, double y1, double x2, double y2, double t)
{
    const auto sample = [](double a, double b, double s) {
        return 3 * a * s * (1 - s) * (1 - s) + 3 * b * s * s * (1 - s) + s * s * s;
    };
    double low = 0;
    double high = 1;
    double s = t;
    for (int i = 0; i < 40; ++i) {
        const double x = sample(x1, x2, s);
        if (std::abs(x - t) < 1e-7)
            break;
        if (x < t)
            low = s;
        else
            high = s;
        s = (low + high) / 2;
    }
    return sample(y1, y2, s);
}

// A row's background ink over the panel's surface.
static ui::Color rowColor(bool pressed, bool hovered)
{
    const ui::Color surface = ::Lingmo::Shell::Panel::statusSurfaceColor();
    if (pressed)
        return ::Lingmo::Shell::Panel::on(surface, 0.14);
    return hovered ? ::Lingmo::Shell::Panel::on(surface, 0.08) : ui::Color {0, 0, 0, 0};
}

PanelRow::PanelRow()
{
    m_glyph = addChild<ui::Icon>(std::string(), kGlyphSize);
    m_artwork = addChild<SvgIcon>();
    m_artwork->setVisible(false);
    m_label = addChild<ui::Label>();
    m_label->setElide(ui::Label::Elide::Right);
    m_detail = addChild<ui::Label>();
    m_detail->setVisible(false);
    m_busy = addChild<ui::BusyIndicator>();
    m_busy->setRunning(false);
    m_busy->setVisible(false);
    m_lock = addChild<ui::Icon>("lock", kGlyphSize);
    m_lock->setVisible(false);
    setTracksHover(true);
}

PanelRow::~PanelRow() = default;

void PanelRow::setGlyph(std::string name)
{
    m_glyph->setIconName(std::move(name));
    m_glyph->setVisible(true);
    m_artwork->setVisible(false);
}

void PanelRow::setArtwork(std::string file)
{
    m_artwork->setFile(std::move(file));
    m_artwork->setVisible(true);
    m_glyph->setVisible(false);
}

void PanelRow::setText(std::string text)
{
    if (text != m_label->text()) {
        m_label->setText(std::move(text));
        requestLayout();
    }
}

void PanelRow::setDetail(std::string detail)
{
    m_detail->setVisible(!detail.empty());
    m_detail->setText(std::move(detail));
    requestLayout();
}

void PanelRow::setActive(bool active)
{
    if (active == m_active)
        return;
    m_active = active;
    requestLayout();
    requestPaint();
}

void PanelRow::setBold(bool bold)
{
    m_label->setWeight(bold ? kDemiBold : 400);
}

void PanelRow::setBusy(bool busy)
{
    if (busy == m_busy->isVisible())
        return;
    m_busy->setVisible(busy);
    m_busy->setRunning(busy);
    requestLayout();
}

void PanelRow::setLocked(bool locked)
{
    if (locked == m_lock->isVisible())
        return;
    m_lock->setVisible(locked);
    requestLayout();
}

void PanelRow::setRowEnabled(bool enabled)
{
    if (enabled == m_rowEnabled)
        return;
    m_rowEnabled = enabled;
    if (!enabled)
        m_pressed = false;
    m_label->setOpacity(enabled ? 1 : 0.6);
    requestPaint();
}

double PanelRow::contentHeight() const
{
    double text = m_label->sizeHint().height;
    if (m_detail->isVisible())
        text += 3 + m_detail->sizeHint().height;
    return std::max(kBadgeSize, text);
}

double PanelRow::heightForWidth(double) const
{
    return std::max(kMinimumRowHeight, contentHeight() + 8);
}

void PanelRow::layout()
{
    const ui::Theme &theme = ui::Theme::current();
    const bool white = theme.darkMode || m_active;
    m_glyph->setColor(white ? kWhite : kLightArtwork);
    m_artwork->setTint(white ? kWhite : kLightArtwork);
    m_lock->setColor(::Lingmo::Shell::Panel::secondaryTextColor());
    m_detail->setColor(::Lingmo::Shell::Panel::secondaryTextColor());

    // The background reaches past the row's padding to kBackgroundMargin.
    const double inset = kRowMargin - kBackgroundMargin + kRowInset;
    const double height = geometry().height;
    const ui::RectF badge {inset, (height - kBadgeSize) / 2, kBadgeSize, kBadgeSize};
    const ui::RectF glyph {badge.x + (kBadgeSize - kGlyphSize) / 2, badge.y + (kBadgeSize - kGlyphSize) / 2,
                           kGlyphSize, kGlyphSize};
    m_glyph->setGeometry(glyph);
    m_artwork->setGeometry(glyph);

    double right = geometry().width - inset;
    if (m_lock->isVisible()) {
        m_lock->setGeometry({right - kGlyphSize, (height - kGlyphSize) / 2, kGlyphSize, kGlyphSize});
        right -= kGlyphSize + kRowSpacing;
    }
    if (m_busy->isVisible()) {
        m_busy->setGeometry({right - 18, (height - 18) / 2, 18, 18});
        right -= 18 + kRowSpacing;
    }
    const double x = badge.x + kBadgeSize + kRowSpacing;
    const double width = std::max(0.0, right - x);
    const double labelHeight = m_label->sizeHint().height;
    if (m_detail->isVisible()) {
        const double detailHeight = m_detail->sizeHint().height;
        const double top = (height - labelHeight - 3 - detailHeight) / 2;
        m_label->setGeometry({x, top, width, labelHeight});
        m_detail->setGeometry({x, top + labelHeight + 3, width, detailHeight});
    } else {
        m_label->setGeometry({x, (height - labelHeight) / 2, width, labelHeight});
    }
}

void PanelRow::paint(ui::Painter &painter)
{
    const bool enabled = m_rowEnabled && isEnabled();
    const ui::Color background = rowColor(enabled && m_pressed, enabled && isHovered());
    if (background.a > 0)
        painter.fillRoundedRect(localRect(), kBackgroundRadius, background);
    const ui::Color surface = ::Lingmo::Shell::Panel::statusSurfaceColor();
    // A step past the row's hover ink, so the badge still shows on a hovered row.
    const ui::Color badge = m_active ? ui::Theme::current().highlight
                                     : ::Lingmo::Shell::Panel::on(surface, ::Lingmo::Shell::Panel::isDark(surface) ? 0.16 : 0.12);
    const double inset = kRowMargin - kBackgroundMargin + kRowInset;
    const ui::RectF slot {inset, (geometry().height - kBadgeSize) / 2, kBadgeSize, kBadgeSize};
    const ui::RectF plate = ::Lingmo::Shell::Panel::parityPlate(slot, kGlyphSize, painter.scale());
    // Liquid Glass: soft shadow under the badge plate so it floats.
    painter.drawShadow(plate, plate.width / 2, 3,
                       ui::Color{0, 0, 0, uint8_t(::Lingmo::Shell::Panel::isDark(surface) ? 60 : 35)},
                       {0, 1});
    painter.fillRoundedRect(plate, plate.width / 2, badge);
    if (!m_active) {
        // Rim highlight on the inactive badge (accent badge already stands out).
        const double rimA = ::Lingmo::Shell::Panel::isDark(surface) ? 0.26 : 0.40;
        painter.strokeRoundedRect(plate.adjusted(0.5, 0.5, -0.5, -0.5), plate.width / 2 - 0.5,
                                  ui::Color{255, 255, 255, uint8_t(rimA * 255)}, 1);
    }
}

void PanelRow::hoverChanged()
{
    requestPaint();
}

void PanelRow::pointerPress(const ui::PointerEvent &event)
{
    if (event.button != ui::MouseButton::Left || !m_rowEnabled)
        return;
    m_pressed = true;
    requestPaint();
}

void PanelRow::pointerRelease(const ui::PointerEvent &event)
{
    if (event.button != ui::MouseButton::Left || !m_pressed)
        return;
    m_pressed = false;
    requestPaint();
    if (insideRect(localRect(), event.position) && m_rowEnabled && onClicked)
        onClicked();
}

// The settings link: its text where the rows' text is, its background 6
// above and below the text and out to kBackgroundMargin from the sides.
class StatusPanelView::SettingsLink final : public ui::View
{
public:
    explicit SettingsLink(std::string text)
    {
        m_label = addChild<ui::Label>(std::move(text));
        m_label->setVerticallyCentered(true);
        setTracksHover(true);
    }

    void setText(std::string text)
    {
        m_label->setText(std::move(text));
        requestLayout();
    }

    // The text's own height, at least 20.
    double textHeight() const { return std::max(20.0, m_label->sizeHint().height); }

    std::function<void()> onClicked;

    bool acceptsPointer() const override { return true; }

protected:
    void layout() override
    {
        const double inset = kHorizontalPadding - kBackgroundMargin;
        m_label->setGeometry({inset, kRowInset, geometry().width - 2 * inset, textHeight()});
    }

    void paint(ui::Painter &painter) override
    {
        const ui::Color background = rowColor(m_pressed, isHovered());
        if (background.a > 0)
            painter.fillRoundedRect(localRect(), kBackgroundRadius, background);
    }

    void hoverChanged() override { requestPaint(); }

    void pointerPress(const ui::PointerEvent &event) override
    {
        if (event.button != ui::MouseButton::Left)
            return;
        m_pressed = true;
        requestPaint();
    }

    void pointerRelease(const ui::PointerEvent &event) override
    {
        if (event.button != ui::MouseButton::Left || !m_pressed)
            return;
        m_pressed = false;
        requestPaint();
        if (insideRect(localRect(), event.position) && onClicked)
            onClicked();
    }

private:
    ui::Label *m_label;
    bool m_pressed = false;
};

StatusPanelView::StatusPanelView(std::string title, std::string settingsText, std::string settingsPage,
                                 ControlCenterServices &services)
    : m_services(services)
    , m_settingsPage(std::move(settingsPage))
{
    m_clip = addChild<ui::View>();
    m_clip->setClipsChildren(true);
    m_body = m_clip->addChild<ui::View>();
    m_title = m_clip->addChild<ui::Label>(std::move(title));
    m_title->setFontSize(kTitleFontSize);
    m_title->setElide(ui::Label::Elide::Right);
    m_detail = m_clip->addChild<ui::Label>();
    m_detail->setVisible(false);
    m_link = addChild<SettingsLink>(std::move(settingsText));
    m_link->onClicked = [this] {
        if (onCloseRequested)
            onCloseRequested();
        m_services.openSettings(m_settingsPage.empty() ? std::vector<std::string>()
                                                       : std::vector<std::string> {"-m", m_settingsPage});
    };
}

StatusPanelView::~StatusPanelView() = default;

ui::View *StatusPanelView::settingsLink() const
{
    return m_link;
}

void StatusPanelView::setTitles(std::string title, std::string settingsText)
{
    m_title->setText(std::move(title));
    m_link->setText(std::move(settingsText));
    requestLayout();
}

void StatusPanelView::setTitleDetail(std::string detail)
{
    m_detail->setVisible(!detail.empty());
    m_detail->setText(std::move(detail));
    requestLayout();
}

void StatusPanelView::setAccessory(ui::View *accessory)
{
    m_accessory = accessory;
    requestLayout();
}

void StatusPanelView::setBodyVisible(bool visible)
{
    if (visible == m_bodyVisible)
        return;
    m_bodyVisible = visible;
    contentChanged();
}

void StatusPanelView::setMaximumHeight(double height)
{
    if (height == m_maximumHeight)
        return;
    m_maximumHeight = height;
    contentChanged();
}

double StatusPanelView::headerHeight() const
{
    // The capitals' top is as far from the edge as the text is from the side,
    // and the body as far below the baseline.
    const double textBaseline = kHorizontalPadding - kContentPadding + m_title->capHeight();
    return textBaseline + kHorizontalPadding - kSectionSpacing;
}

double StatusPanelView::footerHeight() const
{
    // The separator 12 below the body and 12 above the link; the link
    // kRowInset clear of the bottom for its background.
    const double separator = kSectionSpacing + std::max(0.0, kSeparatorSpacing - kSectionSpacing) + 1
            + std::max(0.0, kSeparatorSpacing - kSectionSpacing);
    return separator + kSectionSpacing + m_link->textHeight() + kRowInset;
}

double StatusPanelView::layoutHeight() const
{
    return m_bodyVisible ? headerHeight() + kSectionSpacing + bodyHeight() : headerHeight();
}

double StatusPanelView::contentHeight() const
{
    const double height = std::ceil(kContentPadding + layoutHeight() + footerHeight() + kBottomPadding);
    return m_maximumHeight > 0 ? std::min(height, m_maximumHeight) : height;
}

double StatusPanelView::bodyMaximumHeight() const
{
    return std::floor(m_maximumHeight - kContentPadding - kBottomPadding - headerHeight() - footerHeight()
                      - kSectionSpacing);
}

double StatusPanelView::footerTop() const
{
    // Where the content ends, so a fraction left over by the whole-pixel
    // height goes below the footer, not above it.
    const double rest = std::min(kContentPadding + layoutHeight(),
                                 contentHeight() - kBottomPadding - footerHeight());
    return rest + m_shownBottom - contentHeight();
}

void StatusPanelView::setShown(bool shown)
{
    m_shown = shown;
    if (!shown) {
        if (m_tween)
            m_tween->stop();
        setShownBottom(contentHeight());
        m_reach = m_shownBottom;
    }
}

void StatusPanelView::contentChanged()
{
    animateTo(contentHeight());
    requestLayout();
}

void StatusPanelView::animateTo(double target)
{
    if (target == m_tweenTarget && (m_tween && m_tween->isRunning()))
        return;
    // A hidden window jumps to its height.
    if (!m_shown || !window()) {
        if (m_tween)
            m_tween->stop();
        m_tweenTarget = target;
        m_reach = target;
        setShownBottom(target);
        return;
    }
    if (target == m_shownBottom) {
        m_tweenTarget = target;
        return;
    }
    if (!m_tween) {
        m_tween = std::make_unique<ui::Animation>(window());
        m_tween->setDuration(kHeightDuration);
        m_tween->setEasing(ui::Animation::Easing::Linear);
        m_tween->onUpdate = [this](double t) {
            const double eased = cubicBezier(0.25, 1, 0.5, 1, t);
            setShownBottom(m_tweenFrom + (m_tweenTarget - m_tweenFrom) * eased);
        };
        m_tween->onFinished = [this] {
            m_reach = m_tweenTarget;
            requestLayout();
        };
    }
    m_tween->stop();
    m_tweenFrom = m_shownBottom;
    m_tweenTarget = target;
    m_reach = std::max(m_tweenFrom, target);
    m_tween->start();
}

void StatusPanelView::setShownBottom(double bottom)
{
    if (bottom == m_shownBottom)
        return;
    m_shownBottom = bottom;
    requestLayout();
    requestPaint();
    if (onShownBottomChanged)
        onShownBottomChanged();
}

void StatusPanelView::layout()
{
    const double scale = windowScale(this);
    const double textBaseline = kHorizontalPadding - kContentPadding + m_title->capHeight();
    const double footer = footerTop();
    // Reveals and hides the body while the height animates.
    m_clip->setGeometry({0, kContentPadding, kWidth, std::max(0.0, footer - kContentPadding)});

    const double headerX = kHorizontalPadding;
    double headerRight = kWidth - kHorizontalPadding;
    if (m_accessory) {
        const ui::SizeF size = m_accessory->sizeHint();
        // Centred on the capitals.
        const double capCenter = snapToDevice(snapToDevice(textBaseline, scale) - m_title->capHeight() / 2, scale);
        m_accessory->setGeometry({headerRight - size.width, capCenter - size.height / 2, size.width, size.height});
        headerRight -= size.width + kSectionSpacing;
    }
    if (m_detail->isVisible()) {
        m_detail->setColor(::Lingmo::Shell::Panel::secondaryTextColor());
        const double width = m_detail->sizeHint().width;
        m_detail->setGeometry({headerRight - width, alignedLabelTop(m_detail, textBaseline, scale), width,
                               m_detail->sizeHint().height});
        headerRight -= width + kSectionSpacing;
    }
    m_title->setGeometry({headerX, alignedLabelTop(m_title, textBaseline, scale),
                          std::max(0.0, headerRight - headerX), m_title->sizeHint().height});

    // The body stays drawn while the panel closes over it.
    m_body->setVisible(m_bodyVisible || m_reach > contentHeight());
    m_body->setGeometry({0, 0, kWidth, std::max(0.0, m_maximumHeight)});
    layoutBody(headerHeight() + kSectionSpacing);

    const double linkTop = footer + kSeparatorSpacing + 1 + std::max(0.0, kSeparatorSpacing - kSectionSpacing)
            + kSectionSpacing;
    m_link->setGeometry({kBackgroundMargin, linkTop - kRowInset, kWidth - 2 * kBackgroundMargin,
                         m_link->textHeight() + 2 * kRowInset});
}

void StatusPanelView::paint(ui::Painter &painter)
{
    // The separator above the settings link.
    const double y = footerTop() + kSeparatorSpacing;
    painter.fillRect({kHorizontalPadding, y, kWidth - 2 * kHorizontalPadding, 1}, ::Lingmo::Shell::Panel::textColor(0.08));
}

// A 1 px line of the text colour at 0.08, as the panels' separators.
class Separator final : public ui::View
{
protected:
    void paint(ui::Painter &painter) override { painter.fillRect(localRect(), ::Lingmo::Shell::Panel::textColor(0.08)); }
};

// The networks, laid out in the scroll view edge to edge so its bar sits on
// the panel's edge.
class NetworkPanel::List final : public ui::View
{
public:
    double heightForWidth(double) const override
    {
        double height = 0;
        int count = 0;
        for (const auto &child : children()) {
            if (!child->isVisible())
                continue;
            height += child->heightForWidth(kWidth);
            ++count;
        }
        return count > 0 ? height + (count - 1) * kListSpacing : 0;
    }

protected:
    void layout() override
    {
        double y = 0;
        for (const auto &child : children()) {
            if (!child->isVisible())
                continue;
            const double height = child->heightForWidth(kWidth);
            child->setGeometry({kBackgroundMargin, y, geometry().width - 2 * kBackgroundMargin, height});
            y += height + kListSpacing;
        }
    }
};

NetworkPanel::NetworkPanel(ControlCenterServices &services)
    : StatusPanelView(tr("Wi-Fi"), tr("Wi-Fi Settings"), "wlan", services)
{
    m_switch = header()->addChild<ui::Switch>();
    m_switch->setPadding({0, 0, 0, 0});
    m_switch->setLayoutHints({.preferredHeight = kHeaderHeight});
    m_switch->onToggled = [this](bool checked) {
        this->services().enableWireless(checked);
        if (checked)
            this->services().requestScan();
    };
    setAccessory(m_switch);

    m_scroll = body()->addChild<ui::ScrollView>();
    m_scroll->setScrollDirections(false, true);
    m_list = m_scroll->setContent<List>();
    m_hotspot = m_list->addChild<PanelRow>();
    m_hotspot->setGlyph("network-wireless-hotspot");
    m_hotspot->setActive(true);
    m_hotspot->setBold(true);
    m_hotspot->setDetail(tr("Hotspot"));
    m_hotspot->onClicked = [this] { this->services().stopHotspot(); };

    m_scanTimer.setInterval(kScanInterval);
    m_scanTimer.onTimeout = [this] { this->services().requestScan(); };
    refresh();
}

NetworkPanel::~NetworkPanel() = default;

void NetworkPanel::opened()
{
    setShownForScans(true);
    if (services().wirelessEnabled())
        services().requestScan();
}

void NetworkPanel::closed()
{
    setShownForScans(false);
}

void NetworkPanel::retranslate()
{
    setTitles(tr("Wi-Fi"), tr("Wi-Fi Settings"));
    m_hotspot->setDetail(tr("Hotspot"));
    refresh();
}

void NetworkPanel::setShownForScans(bool shown)
{
    if (shown && services().wirelessEnabled())
        m_scanTimer.start();
    else
        m_scanTimer.stop();
}

void NetworkPanel::refresh()
{
    ControlCenterServices &services = this->services();
    m_switch->setEnabled(services.wirelessHardwareEnabled());
    m_switch->setChecked(services.wirelessEnabled());
    setShownForScans(isShown());

    m_hotspot->setVisible(services.hotspotActive());
    m_hotspot->setText(services.hotspotName());

    std::vector<WirelessNetwork> networks = services.wirelessNetworks();
    if (networks != m_networks) {
        while (m_rows.size() > networks.size()) {
            m_list->takeChild(m_rows.back());
            m_rows.pop_back();
        }
        while (m_rows.size() < networks.size()) {
            PanelRow *row = m_list->addChild<PanelRow>();
            const size_t index = m_rows.size();
            row->onClicked = [this, index] {
                if (index >= m_networks.size())
                    return;
                if (this->services().activateNetwork(m_networks[index]) && onCloseRequested)
                    onCloseRequested();
            };
            m_rows.push_back(row);
        }
        for (size_t i = 0; i < networks.size(); ++i) {
            const WirelessNetwork &network = networks[i];
            PanelRow *row = m_rows[i];
            const bool connected = network.state == WirelessNetwork::State::Activated;
            row->setGlyph(network.icon);
            row->setText(network.name);
            // The connected network is marked by its accent badge alone.
            row->setActive(connected);
            row->setBold(connected);
            row->setBusy(network.state == WirelessNetwork::State::Activating);
            row->setLocked(network.security != WirelessNetwork::Security::None
                           && network.security != WirelessNetwork::Security::Unknown);
            row->setRowEnabled(connected || network.state == WirelessNetwork::State::Deactivated);
        }
        m_networks = std::move(networks);
        m_list->requestLayout();
    }
    // The title's switch already shows that Wi-Fi is off; an empty list shows nothing either.
    setBodyVisible(services.wirelessEnabled() && (!m_networks.empty() || services.hotspotActive()));
    contentChanged();
}

double NetworkPanel::listHeight() const
{
    return m_list->heightForWidth(kWidth);
}

double NetworkPanel::bodyHeight() const
{
    const bool listed = !m_networks.empty() || services().hotspotActive();
    return listed ? std::min(listHeight(), std::max(0.0, bodyMaximumHeight())) : 0;
}

void NetworkPanel::layoutBody(double top)
{
    const bool listed = !m_networks.empty() || services().hotspotActive();
    m_scroll->setVisible(listed);
    // The list keeps its final height while the panel moves, so no scroll bar
    // shows on the way.
    const double height = std::min(listHeight(), std::max(0.0, bodyMaximumHeight()));
    m_scroll->setGeometry({0, top, kWidth, height});
}

class BluetoothPanel::List final : public ui::View
{
public:
    double heightForWidth(double) const override
    {
        double height = 0;
        int count = 0;
        for (const auto &child : children()) {
            if (!child->isVisible())
                continue;
            height += child->heightForWidth(kWidth);
            ++count;
        }
        return count > 0 ? height + (count - 1) * kListSpacing : 0;
    }

protected:
    void layout() override
    {
        double y = 0;
        for (const auto &child : children()) {
            if (!child->isVisible())
                continue;
            const double height = child->heightForWidth(kWidth);
            child->setGeometry({kBackgroundMargin, y, geometry().width - 2 * kBackgroundMargin, height});
            y += height + kListSpacing;
        }
    }
};

BluetoothPanel::BluetoothPanel(ControlCenterServices &services)
    : StatusPanelView(tr("Bluetooth"), tr("Bluetooth Settings"), "bluetooth", services)
{
    m_switch = header()->addChild<ui::Switch>();
    m_switch->setPadding({0, 0, 0, 0});
    m_switch->setLayoutHints({.preferredHeight = kHeaderHeight});
    m_switch->onToggled = [this](bool checked) { this->services().setBluetoothEnabled(checked); };
    setAccessory(m_switch);
    m_scroll = body()->addChild<ui::ScrollView>();
    m_scroll->setScrollDirections(false, true);
    m_list = m_scroll->setContent<List>();
    refresh();
}

BluetoothPanel::~BluetoothPanel() = default;

void BluetoothPanel::opened()
{
    if (services().bluetoothEnabled())
        services().requestBluetoothScan();
}

void BluetoothPanel::closed()
{
    services().stopBluetoothScan();
}

void BluetoothPanel::retranslate()
{
    setTitles(tr("Bluetooth"), tr("Bluetooth Settings"));
    refresh();
}

void BluetoothPanel::refresh()
{
    ControlCenterServices &services = this->services();
    m_switch->setEnabled(services.bluetoothAvailable());
    m_switch->setChecked(services.bluetoothEnabled());
    const std::vector<BluetoothDevice> devices = services.bluetoothDevices();
    if (devices != m_devices) {
        while (m_rows.size() > devices.size()) {
            m_list->takeChild(m_rows.back());
            m_rows.pop_back();
        }
        while (m_rows.size() < devices.size()) {
            PanelRow *row = m_list->addChild<PanelRow>();
            const size_t index = m_rows.size();
            row->onClicked = [this, index] {
                if (index < m_devices.size())
                    this->services().toggleBluetoothDevice(m_devices[index]);
            };
            m_rows.push_back(row);
        }
        for (size_t i = 0; i < devices.size(); ++i) {
            const BluetoothDevice &device = devices[i];
            PanelRow *row = m_rows[i];
            row->setGlyph(device.icon.empty() ? "bluetooth" : device.icon);
            row->setText(device.name);
            row->setDetail(device.connected ? tr("Connected") : device.paired ? tr("Paired") : tr("Not paired"));
            row->setActive(device.connected);
            row->setBold(device.connected);
            row->setBusy(device.busy);
            row->setRowEnabled(!device.busy);
        }
        m_devices = devices;
        m_list->requestLayout();
    }
    setBodyVisible(services.bluetoothEnabled() && !m_devices.empty());
    contentChanged();
}

double BluetoothPanel::listHeight() const
{
    return m_list->heightForWidth(kWidth);
}

double BluetoothPanel::bodyHeight() const
{
    return m_devices.empty() ? 0 : std::min(listHeight(), std::max(0.0, bodyMaximumHeight()));
}

void BluetoothPanel::layoutBody(double top)
{
    const double height = std::min(listHeight(), std::max(0.0, bodyMaximumHeight()));
    m_scroll->setVisible(!m_devices.empty());
    m_scroll->setGeometry({0, top, kWidth, height});
}

VolumePanel::VolumePanel(ControlCenterServices &services)
    : StatusPanelView(tr("Sound"), tr("Sound Settings"), "sound", services)
{
    m_noOutput = body()->addChild<ui::Label>(tr("No output device"));
    m_slider = body()->addChild<LevelSlider>();
    m_slider->setRange(0, 1, 0.01);
    m_slider->onMoved = [this](double value) {
        if (!this->services().hasSink())
            return;
        this->services().setVolume(value);
        this->services().setMuted(value == 0);
    };
    // TRANSLATORS: the heading above the list of sound output devices.
    m_heading = body()->addChild<ui::Label>(tr("Output"));
    m_heading->setWeight(kDemiBold);
    refresh();
}

VolumePanel::~VolumePanel() = default;

void VolumePanel::retranslate()
{
    setTitles(tr("Sound"), tr("Sound Settings"));
    m_noOutput->setText(tr("No output device"));
    // TRANSLATORS: the heading above the list of sound output devices.
    m_heading->setText(tr("Output"));
    refresh();
}

void VolumePanel::refresh()
{
    ControlCenterServices &services = this->services();
    const bool sink = services.hasSink();
    m_noOutput->setVisible(!sink);
    m_slider->setVisible(sink);
    m_slider->setValue(sink ? services.volume() : 0);
    m_slider->setIcon(::Lingmo::Shell::Panel::artwork(volumeIconName(sink, services.volume()), false));

    std::vector<AudioOutput> outputs = services.outputs();
    if (outputs != m_outputs) {
        while (m_rows.size() > outputs.size()) {
            body()->takeChild(m_rows.back());
            m_rows.pop_back();
        }
        while (m_rows.size() < outputs.size()) {
            PanelRow *row = body()->addChild<PanelRow>();
            row->setGlyph("audio-volume-high");
            const size_t index = m_rows.size();
            row->onClicked = [this, index] {
                if (index < m_outputs.size() && !m_outputs[index].isDefault)
                    this->services().setDefaultOutput(m_outputs[index].id);
            };
            m_rows.push_back(row);
        }
        for (size_t i = 0; i < outputs.size(); ++i) {
            m_rows[i]->setText(outputs[i].name);
            // The current output is marked by its accent badge alone.
            m_rows[i]->setActive(outputs[i].isDefault);
        }
        m_outputs = std::move(outputs);
    }
    m_heading->setVisible(!m_outputs.empty());
    contentChanged();
}

double VolumePanel::bodyHeight() const
{
    std::vector<double> heights;
    if (m_noOutput->isVisible())
        heights.push_back(m_noOutput->sizeHint().height);
    if (m_slider->isVisible())
        heights.push_back(LevelSlider::kHeight);
    if (!m_outputs.empty()) {
        heights.push_back(2 + m_heading->sizeHint().height);
        double rows = 0;
        for (const PanelRow *row : m_rows)
            rows += row->heightForWidth(kWidth);
        heights.push_back(rows + (m_rows.size() - 1) * kListSpacing);
    }
    double height = 0;
    for (double value : heights)
        height += value;
    return heights.empty() ? 0 : height + (heights.size() - 1) * kSectionSpacing;
}

void VolumePanel::layoutBody(double top)
{
    const double x = kHorizontalPadding;
    const double width = kWidth - 2 * kHorizontalPadding;
    double y = top;
    m_noOutput->setColor(::Lingmo::Shell::Panel::secondaryTextColor());
    m_heading->setColor(::Lingmo::Shell::Panel::secondaryTextColor());
    if (m_noOutput->isVisible()) {
        m_noOutput->setGeometry({x, y, width, m_noOutput->sizeHint().height});
        y += m_noOutput->sizeHint().height + kSectionSpacing;
    }
    if (m_slider->isVisible()) {
        m_slider->setGeometry({x, y, width, LevelSlider::kHeight});
        y += LevelSlider::kHeight + kSectionSpacing;
    }
    if (m_outputs.empty())
        return;
    y += 2;
    m_heading->setGeometry({x, y, width, m_heading->sizeHint().height});
    y += m_heading->sizeHint().height + kSectionSpacing;
    for (PanelRow *row : m_rows) {
        const double height = row->heightForWidth(kWidth);
        row->setGeometry({kBackgroundMargin, y, kWidth - 2 * kBackgroundMargin, height});
        y += height + kListSpacing;
    }
}

static const char *const kProfiles[] = {"power-saver", "balanced", "performance"};
static const char *const kProfileNames[] = {trNoop("Power save"), trNoop("Balanced"), trNoop("Performance")};

BatteryPanel::BatteryPanel(ControlCenterServices &services)
    : StatusPanelView(tr("Battery"), tr("Battery Settings"), "battery", services)
{
    m_source = body()->addChild<ui::Label>();
    m_source->setWrap(true);
    m_state = body()->addChild<ui::Label>();
    m_state->setWrap(true);
    m_separator = body()->addChild<Separator>();
    m_heading = body()->addChild<ui::Label>(tr("Power Mode"));
    m_heading->setWeight(kDemiBold);
    for (size_t i = 0; i < std::size(kProfiles); ++i) {
        PanelRow *row = body()->addChild<PanelRow>();
        row->setArtwork(::Lingmo::Shell::Panel::artwork(std::string("power-profile-") + kProfiles[i]));
        row->setText(tr(kProfileNames[i]));
        row->onClicked = [this, i] { this->services().setPowerProfile(kProfiles[i]); };
        m_rows.push_back(row);
    }
    refresh();
}

BatteryPanel::~BatteryPanel() = default;

void BatteryPanel::retranslate()
{
    setTitles(tr("Battery"), tr("Battery Settings"));
    m_heading->setText(tr("Power Mode"));
    for (size_t i = 0; i < m_rows.size(); ++i)
        m_rows[i]->setText(tr(kProfileNames[i]));
    refresh();
}

void BatteryPanel::refresh()
{
    ControlCenterServices &services = this->services();
    setTitleDetail(std::to_string(services.chargePercent()) + "%");
    m_source->setText(services.onBattery() ? tr("Power Source: Battery") : tr("Power Source: Power Adapter"));
    const std::string state = services.batteryStatus();
    m_state->setText(state);
    m_state->setVisible(!state.empty());
    m_profiles = services.powerProfiles();
    const std::string current = services.powerProfile();
    for (size_t i = 0; i < m_rows.size(); ++i) {
        m_rows[i]->setVisible(std::find(m_profiles.begin(), m_profiles.end(), kProfiles[i]) != m_profiles.end());
        // The current mode is marked by its accent badge alone.
        m_rows[i]->setActive(current == kProfiles[i]);
    }
    m_heading->setVisible(!m_profiles.empty());
    m_separator->setVisible(!m_profiles.empty());
    contentChanged();
}

double BatteryPanel::bodyHeight() const
{
    const double width = kWidth - 2 * kHorizontalPadding;
    double height = m_source->heightForWidth(width);
    if (m_state->isVisible())
        height += 3 + m_state->heightForWidth(width);
    if (m_profiles.empty())
        return height;
    // The separator, spaced like the one above the settings link.
    const double margin = std::max(0.0, kSeparatorSpacing - kSectionSpacing);
    height += kSectionSpacing + margin + 1 + margin;
    height += kSectionSpacing + m_heading->sizeHint().height;
    double rows = 0;
    int count = 0;
    for (const PanelRow *row : m_rows) {
        if (!row->isVisible())
            continue;
        rows += row->heightForWidth(kWidth);
        ++count;
    }
    if (count > 0)
        height += kSectionSpacing + rows + (count - 1) * kListSpacing;
    return height;
}

void BatteryPanel::layoutBody(double top)
{
    const double x = kHorizontalPadding;
    const double width = kWidth - 2 * kHorizontalPadding;
    m_source->setColor(::Lingmo::Shell::Panel::secondaryTextColor());
    m_state->setColor(::Lingmo::Shell::Panel::secondaryTextColor());
    m_heading->setColor(::Lingmo::Shell::Panel::secondaryTextColor());
    double y = top;
    const double sourceHeight = m_source->heightForWidth(width);
    m_source->setGeometry({x, y, width, sourceHeight});
    y += sourceHeight;
    if (m_state->isVisible()) {
        const double stateHeight = m_state->heightForWidth(width);
        m_state->setGeometry({x, y + 3, width, stateHeight});
        y += 3 + stateHeight;
    }
    if (m_profiles.empty())
        return;
    // Spaced like the separator above the settings link.
    const double margin = std::max(0.0, kSeparatorSpacing - kSectionSpacing);
    const double separator = y + kSectionSpacing + margin;
    m_separator->setGeometry({x, separator, width, 1});
    y = separator + 1 + margin + kSectionSpacing;
    m_heading->setGeometry({x, y, width, m_heading->sizeHint().height});
    y += m_heading->sizeHint().height + kSectionSpacing;
    for (PanelRow *row : m_rows) {
        if (!row->isVisible())
            continue;
        const double height = row->heightForWidth(kWidth);
        row->setGeometry({kBackgroundMargin, y, kWidth - 2 * kBackgroundMargin, height});
        y += height + kListSpacing;
    }
}

} // namespace Lingmo::Shell
