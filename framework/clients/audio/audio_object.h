// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "base/signal.h"

struct spa_dict;
struct spa_pod;

namespace Lingmo::Audio {

class Backend;

// A PipeWire node (a sink, a source or a stream) or a card, as the backend
// keeps it from the daemon's events.
class AudioObject
{
public:
    enum Kind {
        Sink,
        Source,
        Playback,
        Capture,
        Card,
    };
    enum State {
        UnknownState,
        RunningState,
        IdleState,
        SuspendedState,
    };
    enum Availability {
        Unknown,
        Available,
        Unavailable,
    };

    // A card's profile, or a route (a port) with its direction and where it applies.
    struct Choice
    {
        int index = -1;
        std::string name;
        std::string description;
        int priority = 0;
        int availability = Unknown;
        uint32_t direction = 0;
        std::vector<int> devices;
        std::vector<int> profiles;
    };

    static constexpr uint32_t InvalidId = 0xffffffff;
    static constexpr double MinimalVolume = 0.0;
    static constexpr double NormalVolume = 1.0;
    static constexpr double MaximalVolume = 1.5;

    AudioObject(Backend* backend, uint32_t id, Kind kind);

    uint32_t index() const { return m_id; }
    Kind kind() const { return m_kind; }
    std::string name() const;
    std::string description() const;
    // A built-in card's node is named after the controller; its active port says what it is.
    std::string displayName() const;
    std::string iconName() const;
    const std::map<std::string, std::string>& properties() const { return m_properties; }
    std::string property(const std::string& key) const;
    // The loudest channel, on the cubic scale people hear.
    double volume() const;
    bool muted() const { return m_muted; }
    bool hasVolume() const { return !m_volumes.empty(); }
    bool volumeWritable() const { return m_writable && hasVolume(); }
    std::vector<double> channelVolumes() const;
    // Short channel names, such as "FL".
    std::vector<std::string> channels() const;
    bool isDefault() const;
    int state() const { return m_state; }
    bool isVirtualDevice() const;
    uint32_t cardIndex() const;
    // The card's bus, such as "pci", "usb" or "bluetooth"; empty for virtual
    // nodes and for SoC codecs, which have no bus.
    std::string bus() const;
    // The card's routes this node can use.
    std::vector<Choice> ports() const;
    int activePortIndex() const;
    const std::vector<Choice>& profiles() const { return m_profiles; }
    int activeProfileIndex() const;
    // The node a stream plays to or records from.
    uint32_t deviceIndex() const { return m_deviceId; }
    bool isCorked() const { return m_state != RunningState; }

    void setVolume(double volume);
    void setMuted(bool muted);
    void setChannelVolumes(const std::vector<double>& volumes);
    void setChannelVolume(int channel, double volume);
    void setDefault(bool enable);
    void setActivePortIndex(int index);
    void setActiveProfileIndex(int index);
    void setDeviceIndex(uint32_t id);
    void switchStreams();

    static double fromLinear(float volume);
    static float toLinear(double volume);

    base::Signal<> changed;

    // For the backend and its tests.
    void updateProperties(const spa_dict* properties);
    void updateVolume(const spa_pod* param);
    void updateProfile(const spa_pod* param, bool current);
    // Returns the device of a current route, -1 otherwise.
    int updateRoute(const spa_pod* param, bool current);
    int profileDevice() const;
    const std::vector<Choice>& routes() const { return m_routes; }
    const std::map<int, int>& activeRoutes() const { return m_activeRoutes; }

private:
    friend class Backend;

    const AudioObject* cardObject() const;

    Backend* m_backend;
    uint32_t m_id;
    Kind m_kind;
    std::map<std::string, std::string> m_properties;
    std::vector<float> m_volumes;
    std::vector<uint32_t> m_channels;
    bool m_channelVolume = false;
    bool m_muted = false;
    bool m_writable = false;
    int m_state = UnknownState;
    uint32_t m_deviceId = InvalidId;
    std::vector<Choice> m_profiles;
    std::vector<Choice> m_routes;
    std::map<int, int> m_activeRoutes;
    std::map<int, std::vector<uint8_t>> m_routeProperties;
    int m_profile = -1;
};

} // namespace Lingmo::Audio
