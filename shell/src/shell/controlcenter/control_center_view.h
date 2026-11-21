// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <functional>

#include "base/timer.h"
#include "ui/view.h"

namespace Lingmo::ui {
class Label;
}

namespace Lingmo::Shell {

class CardItem;
class ControlCenterServices;
class DeviceUsersTile;
class ExpandButton;
class MediaTile;
class TileSlider;

// The control centre's content without its window: the device-users tile,
// the primary cards, an expand button, the secondary cards, the media card
// and the two sliders, 360 wide.
class ControlCenterView : public ui::View
{
public:
    static constexpr double kWidth = 360;
    // Between the tiles and round them.
    static constexpr double kGap = 10;

    explicit ControlCenterView(ControlCenterServices &services);
    ~ControlCenterView() override;

    // Reads the services again.
    void refresh();
    // Takes its text again after the session language changed.
    void retranslate();
    // The window height at `scale`: the layout's, on whole device pixels.
    double windowHeight(double scale) const;

    CardItem *wirelessCard() const { return m_wireless; }
    CardItem *bluetoothCard() const { return m_bluetooth; }
    CardItem *darkModeCard() const { return m_darkMode; }
    CardItem *doNotDisturbCard() const { return m_doNotDisturb; }
    CardItem *cameraCard() const { return m_camera; }
    MediaTile *media() const { return m_media; }
    TileSlider *brightnessSlider() const { return m_brightness; }
    TileSlider *soundSlider() const { return m_sound; }

    // Press-and-hold on a card hides the control centre for Settings.
    std::function<void()> onHideRequested;
    std::function<void()> onWirelessDetailsRequested;
    std::function<void()> onBluetoothDetailsRequested;
    // The layout's height may have changed.
    std::function<void()> onHeightChanged;

protected:
    void layout() override;

private:
    double layoutHeight() const;
    void applyExpanded();

    ControlCenterServices &m_services;
    DeviceUsersTile *m_users;
    // Primary row (always visible): Wi-Fi, Bluetooth, Dark Mode, Do Not Disturb.
    CardItem *m_wireless;
    CardItem *m_bluetooth;
    CardItem *m_darkMode;
    CardItem *m_doNotDisturb;
    // Expand button between primary and secondary rows.
    ExpandButton *m_expandButton;
    // Secondary row (collapsible): Camera, Microphone, Hotspot, Power Profile.
    CardItem *m_camera;
    CardItem *m_microphone;
    CardItem *m_hotspot;
    CardItem *m_powerProfile;
    // Media tile (only visible while a player is active).
    MediaTile *m_media;
    // Sliders.
    TileSlider *m_brightness;
    TileSlider *m_sound;
    base::Timer m_brightnessTimer;
    bool m_darkModeShown = false;
    bool m_expanded = false;
};

} // namespace Lingmo::Shell

