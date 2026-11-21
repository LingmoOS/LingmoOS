// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "base/signal.h"

namespace Lingmo::Shell {

// A Wi-Fi row of the network panel (AppletProxyModel's wireless rows).
struct WirelessNetwork {
    enum class State {
        Activating,
        Activated,
        Deactivating,
        Deactivated,
    };
    // The kinds of security the panel acts on.
    enum class Security {
        None,
        Unknown,
        // Joined with a password the secret agent asks for first.
        Password,
        // Joined through Settings' window, which asks for a user name too.
        Enterprise,
        Other,
    };

    std::string key;
    std::string name;
    std::string icon;
    State state = State::Deactivated;
    Security security = Security::None;
    std::string connectionPath;
    std::string devicePath;
    std::string specificPath;

    bool operator==(const WirelessNetwork &) const = default;
};

struct BluetoothDevice {
    std::string path;
    std::string name;
    std::string icon;
    bool connected = false;
    bool paired = false;
    bool busy = false;
    int batteryPercentage = -1;

    bool operator==(const BluetoothDevice &) const = default;
};

// An output of the sound panel.
struct AudioOutput {
    uint32_t id = 0;
    std::string name;
    bool isDefault = false;

    bool operator==(const AudioOutput &) const = default;
};

// The media card's player (MprisItem).
struct MediaState {
    // A player that is not stopped; a paused one stays.
    bool hasPlayer = false;
    bool playing = false;
    std::string title;
    std::string identity;
    std::string artist;
    std::string artUrl;
    bool canControl = false;
    bool canGoPrevious = false;
    bool canGoNext = false;
    bool canPlay = false;
    bool canPause = false;

    bool operator==(const MediaState &) const = default;
};

// One application behind the privacy dot (DeviceUsers).
struct DeviceUser {
    std::string name;
    std::string icon;
    bool camera = false;
    bool microphone = false;
    bool systemAudio = false;

    bool operator==(const DeviceUser &) const = default;
};

// Everything the control centre and the status panels show and change: the
// session's services, or a test's. Getters answer from what is known now;
// setters return at once and the change comes back through `changed`.
class ControlCenterServices
{
public:
    virtual ~ControlCenterServices() = default;

    // --- Network (NetworkManager) ---
    virtual bool wirelessHardwareEnabled() const = 0;
    virtual bool wirelessEnabled() const = 0;
    virtual bool hotspotActive() const = 0;
    virtual std::string hotspotName() const = 0;
    // The Wi-Fi network this machine is on, and its strength glyph.
    virtual std::string wirelessName() const = 0;
    virtual std::string wirelessIcon() const = 0;
    // Connected first, as the panel lists them.
    virtual std::vector<WirelessNetwork> wirelessNetworks() const = 0;
    virtual void enableWireless(bool enable) = 0;
    virtual void requestScan() = 0;
    virtual void stopHotspot() = 0;
    // What a row's click does (NetworkPanel's onClicked); true when the panel
    // closes for a window that asks for more.
    virtual bool activateNetwork(const WirelessNetwork &network) = 0;

    // --- Bluetooth (BlueZ) ---
    virtual bool bluetoothAvailable() const = 0;
    virtual bool bluetoothEnabled() const = 0;
    virtual void setBluetoothEnabled(bool enabled) = 0;
    virtual std::vector<BluetoothDevice> bluetoothDevices() const { return {}; }
    virtual void requestBluetoothScan() {}
    virtual void stopBluetoothScan() {}
    virtual void toggleBluetoothDevice(const BluetoothDevice &) {}

    // --- Switches ---
    virtual void setDarkMode(bool dark) = 0;
    virtual bool doNotDisturb() const = 0;
    virtual void setDoNotDisturb(bool enabled) = 0;
    virtual bool cameraEnabled() const = 0;
    virtual void setCameraEnabled(bool enabled) = 0;
    virtual bool microphoneEnabled() const = 0;
    virtual void setMicrophoneEnabled(bool enabled) = 0;
    virtual std::vector<DeviceUser> deviceUsers() const = 0;

    // --- Media (MPRIS) ---
    virtual MediaState media() const = 0;
    virtual void mediaPrevious() = 0;
    virtual void mediaPlay() = 0;
    virtual void mediaPause() = 0;
    virtual void mediaNext() = 0;

    // --- Display (org.lingmo.Services.Brightness), percent ---
    virtual bool brightnessEnabled() const = 0;
    virtual int brightness() const = 0;
    virtual void setBrightness(int value) = 0;

    // --- Sound (PipeWire), PipeWire's cubic scale, 1 being normal ---
    virtual bool hasSink() const = 0;
    virtual double volume() const = 0;
    virtual bool muted() const = 0;
    virtual void setVolume(double volume) = 0;
    virtual void setMuted(bool muted) = 0;
    virtual std::vector<AudioOutput> outputs() const = 0;
    virtual void setDefaultOutput(uint32_t id) = 0;

    // --- Battery (UPower) and power profiles ---
    virtual int chargePercent() const = 0;
    virtual bool onBattery() const = 0;
    virtual std::string batteryStatus() const = 0;
    virtual std::vector<std::string> powerProfiles() const = 0;
    virtual std::string powerProfile() const = 0;
    virtual void setPowerProfile(const std::string &profile) = 0;

    // `lingmo-settings` with these arguments.
    virtual void openSettings(const std::vector<std::string> &arguments) = 0;

    // Anything above changed.
    base::Signal<> changed;
    // A VPN went down on its own, with the profile's name and why.
    base::Signal<const std::string &, const std::string &> vpnFailed;
    // The applications behind the privacy dot, from whoever owns the uses
    // (the bar's DeviceUsage); none while unset.
    std::function<std::vector<DeviceUser>()> deviceUsersSource;
};

// audio-volume-{muted,low,medium,high}-symbolic for a volume, as the bar and
// the sliders name it.
std::string volumeIconName(bool hasSink, double volume);

} // namespace Lingmo::Shell

