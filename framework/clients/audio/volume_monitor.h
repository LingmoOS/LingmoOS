// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>
#include <vector>

#include <pipewire/pipewire.h>

#include "base/signal.h"
#include "lingmo/audio/audio_object.h"

namespace Lingmo::Audio {

class Backend;

// The peak level of a sink or source, read through a capture stream of its
// own while a target is set.
class VolumeMonitor
{
public:
    explicit VolumeMonitor(std::shared_ptr<Backend> backend);
    ~VolumeMonitor();

    VolumeMonitor(const VolumeMonitor&) = delete;
    VolumeMonitor& operator=(const VolumeMonitor&) = delete;

    std::shared_ptr<AudioObject> target() const { return m_target.lock(); }
    void setTarget(const std::shared_ptr<AudioObject>& target);
    // 0 to 1, falling at 30 dB/s.
    double volume() const { return m_volume; }
    bool isAvailable() const { return m_available; }

    base::Signal<> targetChanged;
    base::Signal<> volumeChanged;
    base::Signal<> availableChanged;

private:
    static const pw_stream_events streamEvents;
    void stop();
    void process();
    void updateVolume(double volume);
    void updateAvailable(bool available);

    std::shared_ptr<Backend> m_backend;
    std::weak_ptr<AudioObject> m_target;
    pw_stream* m_stream = nullptr;
    spa_hook m_listener{};
    double m_volume = 0.0;
    bool m_available = false;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace Lingmo::Audio
