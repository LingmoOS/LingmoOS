// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/controlcenter/control_center_view.h"

#include <cmath>
#include <vector>

#include "base/i18n.h"
#include "shell/controlcenter/control_center_services.h"
#include "shell/controlcenter/level_slider.h"
#include "shell/controlcenter/panel_style.h"
#include "shell/controlcenter/tiles.h"
#include "ui/label.h"
#include "ui/menu.h"
#include "ui/round_image_button.h"
#include "ui/theme.h"
#include "ui/window.h"

using Lingmo::i18n::tr;

namespace Lingmo::Shell {

// The slider writes the backlight once the knob rests this long.
static constexpr std::chrono::milliseconds kBrightnessDelay {100};

static double windowScale(const ui::View *view)
{
    const ui::Window *window = view->window();
    return window && window->scale() > 0 ? window->scale() : 1;
}

ControlCenterView::ControlCenterView(ControlCenterServices &services)
    : m_services(services)
{
    m_users = addChild<DeviceUsersTile>();

    // ── Primary row (always visible) ──────────────────────────────────────
    m_wireless = addChild<CardItem>();
    m_wireless->setGlyph("network-wireless-connected-100");
    m_wireless->setTitle(tr("Wi-Fi"));
    m_wireless->onClicked = [this] { m_services.enableWireless(!m_wireless->isChecked()); };
    m_wireless->onPressAndHold = [this] {
        if (onWirelessDetailsRequested) {
            onWirelessDetailsRequested();
            return;
        }
        if (onHideRequested)
            onHideRequested();
        m_services.openSettings({"-m", "wlan"});
    };

    m_bluetooth = addChild<CardItem>();
    m_bluetooth->setArtwork("bluetooth-symbolic");
    m_bluetooth->setTitle(tr("Bluetooth"));
    m_bluetooth->onClicked = [this] { m_services.setBluetoothEnabled(!m_services.bluetoothEnabled()); };
    m_bluetooth->onPressAndHold = [this] {
        if (onBluetoothDetailsRequested) {
            onBluetoothDetailsRequested();
            return;
        }
        if (onHideRequested)
            onHideRequested();
        m_services.openSettings({"-m", "bluetooth"});
    };

    m_darkMode = addChild<CardItem>();
    m_darkMode->setArtwork("dark-mode");
    m_darkMode->setTitle(tr("Dark Mode"));
    m_darkMode->onClicked = [this] { m_services.setDarkMode(!ui::Theme::current().darkMode); };

    m_doNotDisturb = addChild<CardItem>();
    m_doNotDisturb->setArtwork("do-not-disturb");
    m_doNotDisturb->setTitle(tr("Do Not Disturb"));
    m_doNotDisturb->onClicked = [this] { m_services.setDoNotDisturb(!m_services.doNotDisturb()); };

    // ── Expand button ─────────────────────────────────────────────────────
    m_expandButton = addChild<ExpandButton>();
    m_expandButton->onClicked = [this] {
        m_expanded = !m_expanded;
        applyExpanded();
        if (onHeightChanged)
            onHeightChanged();
    };

    // ── Secondary row (collapsible) ───────────────────────────────────────
    m_camera = addChild<CardItem>();
    m_camera->setArtwork("camera");
    m_camera->setTitle(tr("Camera"));
    m_camera->onClicked = [this] { m_services.setCameraEnabled(!m_camera->isChecked()); };

    m_microphone = addChild<CardItem>();
    m_microphone->setArtwork("microphone");
    m_microphone->setTitle(tr("Microphone"));
    m_microphone->onClicked = [this] { m_services.setMicrophoneEnabled(!m_microphone->isChecked()); };

    m_hotspot = addChild<CardItem>();
    m_hotspot->setArtwork("hotspot");
    m_hotspot->setTitle(tr("Hotspot"));
    m_hotspot->onClicked = [this] { m_services.stopHotspot(); };

    m_powerProfile = addChild<CardItem>();
    m_powerProfile->setArtwork("power-profile");
    m_powerProfile->setTitle(tr("Power Mode"));

    // ── Media tile ────────────────────────────────────────────────────────
    m_media = addChild<MediaTile>();
    m_media->onPrevious = [this] { m_services.mediaPrevious(); };
    m_media->onPlay = [this] { m_services.mediaPlay(); };
    m_media->onPause = [this] { m_services.mediaPause(); };
    m_media->onNext = [this] { m_services.mediaNext(); };

    // ── Sliders ───────────────────────────────────────────────────────────
    // TRANSLATORS: the title of the screen brightness slider.
    m_brightness = addChild<TileSlider>(tr("Display"));
    m_brightness->slider()->setRange(1, 100, 1);
    m_brightness->slider()->setIcon(::Lingmo::Shell::Panel::artwork("brightness", false));
    m_brightnessTimer.setSingleShot(true);
    m_brightnessTimer.setInterval(kBrightnessDelay);
    m_brightnessTimer.onTimeout = [this] {
        m_services.setBrightness(int(std::lround(m_brightness->slider()->visualValue())));
    };
    m_brightness->slider()->onMoved = [this](double) { m_brightnessTimer.start(); };

    m_sound = addChild<TileSlider>(tr("Sound"));
    m_sound->slider()->setRange(0, 1, 0.01);
    m_sound->slider()->onMoved = [this](double value) {
        if (!m_services.hasSink())
            return;
        m_services.setVolume(value);
        m_services.setMuted(value == 0);
    };

    // Power button — only shown in efficient mode (setEfficientMode(true)).
    m_powerButton = addChild<ui::RoundImageButton>();
    m_powerButton->setIconName("system-shutdown");
    m_powerButton->setButtonSize(kPowerButtonHeight);
    m_powerButton->setVisible(false);
    m_powerButton->onClicked = [this] {
        if (!m_powerMenu)
            m_powerMenu = std::make_unique<ui::Menu>();
        m_powerMenu->clear();
        m_powerMenu->addItem(tr("Suspend"),    [this] { if (onPowerRequested) onPowerRequested(0); });
        m_powerMenu->addItem(tr("Restart"),    [this] { if (onPowerRequested) onPowerRequested(1); });
        m_powerMenu->addItem(tr("Shut Down"),  [this] { if (onPowerRequested) onPowerRequested(2); });
        m_powerMenu->addSeparator();
        m_powerMenu->addItem(tr("Lock Screen"),[this] { if (onPowerRequested) onPowerRequested(3); });
        m_powerMenu->addItem(tr("Log Out"),    [this] { if (onPowerRequested) onPowerRequested(4); });
        m_powerMenu->popupFor(m_powerButton);
    };

    applyExpanded();
    refresh();
}

ControlCenterView::~ControlCenterView() = default;

void ControlCenterView::applyExpanded()
{
    m_expandButton->setExpanded(m_expanded);
    m_expandButton->setLabel(tr("More"), tr("Less"));
    m_camera->setVisible(m_expanded);
    m_microphone->setVisible(m_expanded);
    m_hotspot->setVisible(m_expanded);
    m_powerProfile->setVisible(m_expanded);
}

void ControlCenterView::retranslate()
{
    m_wireless->setTitle(tr("Wi-Fi"));
    m_bluetooth->setTitle(tr("Bluetooth"));
    m_darkMode->setTitle(tr("Dark Mode"));
    m_doNotDisturb->setTitle(tr("Do Not Disturb"));
    m_camera->setTitle(tr("Camera"));
    m_microphone->setTitle(tr("Microphone"));
    m_hotspot->setTitle(tr("Hotspot"));
    m_powerProfile->setTitle(tr("Power Mode"));
    m_expandButton->setLabel(tr("More"), tr("Less"));
    // TRANSLATORS: the title of the screen brightness slider.
    m_brightness->setTitle(tr("Display"));
    m_sound->setTitle(tr("Sound"));
    m_users->retranslate();
    refresh();
}

void ControlCenterView::setEfficientMode(bool efficient)
{
    if (m_powerButton)
        m_powerButton->setVisible(efficient);
    if (onHeightChanged)
        onHeightChanged();
}

void ControlCenterView::refresh()
{
    const double before = layoutHeight();

    m_users->setUsers(m_services.deviceUsers());
    m_users->setVisible(!m_users->users().empty());

    m_wireless->setVisible(m_services.wirelessHardwareEnabled());
    m_wireless->setChecked(m_services.wirelessEnabled());
    const std::string network = m_services.wirelessName();
    m_wireless->setStatus(!m_wireless->isChecked() ? tr("Off") : !network.empty() ? network : tr("On"));

    m_bluetooth->setVisible(m_services.bluetoothAvailable());
    m_bluetooth->setChecked(m_services.bluetoothEnabled());
    m_bluetooth->setStatus(m_bluetooth->isChecked() ? tr("On") : tr("Off"));

    m_darkMode->setChecked(ui::Theme::current().darkMode);
    m_darkMode->setStatus(m_darkMode->isChecked() ? tr("On") : tr("Off"));
    m_doNotDisturb->setChecked(m_services.doNotDisturb());
    m_doNotDisturb->setStatus(m_doNotDisturb->isChecked() ? tr("On") : tr("Off"));
    m_camera->setChecked(m_services.cameraEnabled());
    m_camera->setStatus(m_camera->isChecked() ? tr("On") : tr("Off"));
    m_microphone->setChecked(m_services.microphoneEnabled());
    m_microphone->setStatus(m_microphone->isChecked() ? tr("On") : tr("Off"));

    // Hotspot: only relevant when hardware supports it.
    const bool hotspotOn = m_services.hotspotActive();
    m_hotspot->setChecked(hotspotOn);
    m_hotspot->setStatus(hotspotOn ? tr("On") : tr("Off"));
    m_hotspot->onClicked = [this] {
        if (m_services.hotspotActive())
            m_services.stopHotspot();
    };

    // Power profile: cycle through available profiles on click.
    const std::vector<std::string> profiles = m_services.powerProfiles();
    if (profiles.empty()) {
        m_powerProfile->setStatus(tr("Off"));
    } else {
        const std::string &current = m_services.powerProfile();
        m_powerProfile->setStatus(current.empty() ? profiles.front() : current);
    }
    m_powerProfile->setChecked(!profiles.empty());
    m_powerProfile->onClicked = [this, profiles] {
        if (profiles.size() < 2)
            return;
        const std::string current = m_services.powerProfile();
        size_t idx = 0;
        for (size_t i = 0; i < profiles.size(); ++i) {
            if (profiles[i] == current) {
                idx = i;
                break;
            }
        }
        m_services.setPowerProfile(profiles[(idx + 1) % profiles.size()]);
    };

    m_media->setState(m_services.media());
    m_media->setVisible(m_services.media().hasPlayer);

    m_brightness->setVisible(m_services.brightnessEnabled());
    m_brightness->slider()->setValue(m_services.brightness());

    const bool sink = m_services.hasSink();
    m_sound->setVisible(sink);
    m_sound->slider()->setValue(sink ? m_services.volume() : 0);
    m_sound->slider()->setIcon(::Lingmo::Shell::Panel::artwork(volumeIconName(sink, m_services.volume()), false));

    requestLayout();
    if (layoutHeight() != before && onHeightChanged)
        onHeightChanged();
}

double ControlCenterView::layoutHeight() const
{
    const double width = kWidth - 2 * kGap;
    double height = 0;
    int sections = 0;

    auto add = [&](double h) {
        height += h;
        ++sections;
    };

    if (m_users->isVisible())
        add(m_users->heightForWidth(width));

    // Primary cards (always 4 → 2 rows).
    {
        int cards = 0;
        for (const CardItem *card : {m_wireless, m_bluetooth, m_darkMode, m_doNotDisturb})
            cards += card->isVisible();
        const int rows = (cards + 1) / 2;
        if (rows > 0)
            add(rows * CardItem::kHeight + (rows - 1) * kGap);
    }

    // Expand button.
    add(ExpandButton::kHeight);

    // Secondary cards (only when expanded).
    if (m_expanded) {
        int cards = 0;
        for (const CardItem *card : {m_camera, m_microphone, m_hotspot, m_powerProfile})
            cards += card->isVisible();
        const int rows = (cards + 1) / 2;
        if (rows > 0)
            add(rows * CardItem::kHeight + (rows - 1) * kGap);
    }

    if (m_media->isVisible())
        add(MediaTile::kHeight);

    if (m_brightness->isVisible())
        add(m_brightness->heightForWidth(width));

    if (m_sound->isVisible())
        add(m_sound->heightForWidth(width));

    if (m_powerButton && m_powerButton->isVisible())
        add(kPowerButtonHeight);

    return height + (sections - 1) * kGap;
}

double ControlCenterView::windowHeight(double scale) const
{
    // Whole device pixels: the tiles' edges stay on the pixel grid.
    double size = std::ceil(layoutHeight() + kGap * 2);
    for (int i = 0; i < 120 && std::abs(size * scale - std::round(size * scale)) > 0.001; ++i)
        ++size;
    return size;
}

void ControlCenterView::layout()
{
    // A theme change relays the view out: the dark mode card follows it.
    if (m_darkModeShown != ui::Theme::current().darkMode) {
        m_darkModeShown = ui::Theme::current().darkMode;
        m_darkMode->setChecked(m_darkModeShown);
        m_darkMode->setStatus(m_darkModeShown ? tr("On") : tr("Off"));
        m_media->setState(m_services.media());
    }

    const double width = kWidth - 2 * kGap;
    double y = kGap;

    if (m_users->isVisible()) {
        const double height = m_users->heightForWidth(width);
        m_users->setGeometry({kGap, y, width, height});
        y += height + kGap;
    }

    const double cellWidth = (width - kGap) / 2;

    auto placeCards = [&](std::vector<CardItem *> cards) {
        int index = 0;
        for (CardItem *card : cards) {
            if (!card->isVisible())
                continue;
            const int row = index / 2;
            const int column = index % 2;
            card->setGeometry({kGap + column * (cellWidth + kGap), y + row * (CardItem::kHeight + kGap),
                               cellWidth, CardItem::kHeight});
            ++index;
        }
        const int rows = (index + 1) / 2;
        if (rows > 0)
            y += rows * CardItem::kHeight + (rows - 1) * kGap + kGap;
    };

    // Primary row.
    placeCards({m_wireless, m_bluetooth, m_darkMode, m_doNotDisturb});

    // Expand button.
    m_expandButton->setGeometry({kGap, y, width, ExpandButton::kHeight});
    y += ExpandButton::kHeight + kGap;

    // Secondary row (only when expanded).
    if (m_expanded)
        placeCards({m_camera, m_microphone, m_hotspot, m_powerProfile});

    if (m_media->isVisible()) {
        m_media->setGeometry({kGap, y, width, MediaTile::kHeight});
        y += MediaTile::kHeight + kGap;
    }

    for (TileSlider *slider : {m_brightness, m_sound}) {
        if (!slider->isVisible())
            continue;
        const double height = slider->heightForWidth(width);
        slider->setGeometry({kGap, y, width, height});
        y += height + kGap;
    }

    // Power button row (efficient mode only) — centred at the bottom.
    if (m_powerButton && m_powerButton->isVisible()) {
        const double btnSize = kPowerButtonHeight;
        m_powerButton->setGeometry({(kWidth - btnSize) / 2, y, btnSize, btnSize});
        y += btnSize + kGap;
    }
}

std::string volumeIconName(bool hasSink, double volume)
{
    const double level = hasSink ? volume * 100.0 : -1;
    if (level <= 0)
        return "audio-volume-muted-symbolic";
    if (level <= 25)
        return "audio-volume-low-symbolic";
    if (level <= 75)
        return "audio-volume-medium-symbolic";
    return "audio-volume-high-symbolic";
}

} // namespace Lingmo::Shell
