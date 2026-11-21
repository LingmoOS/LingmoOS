// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/audio/volume_monitor.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include <spa/param/audio/format-utils.h>

#include "lingmo/audio/backend.h"

namespace Lingmo::Audio {

const pw_stream_events VolumeMonitor::streamEvents = [] {
    pw_stream_events events{};
    events.version = PW_VERSION_STREAM_EVENTS;
    events.process = [](void* data) { static_cast<VolumeMonitor*>(data)->process(); };
    events.state_changed = [](void* data, pw_stream_state, pw_stream_state state, const char*) {
        auto* self = static_cast<VolumeMonitor*>(data);
        const bool available = state == PW_STREAM_STATE_STREAMING;
        self->updateAvailable(available);
        if (!available)
            self->updateVolume(0.0);
    };
    return events;
}();

VolumeMonitor::VolumeMonitor(std::shared_ptr<Backend> backend)
    : m_backend(std::move(backend))
{
    m_subscriptions.push_back(m_backend->disconnecting.connect([this] { setTarget(nullptr); }));
    m_subscriptions.push_back(m_backend->objectRemoved.connect([this](std::shared_ptr<AudioObject> object) {
        if (m_target.lock() == object)
            setTarget(nullptr);
    }));
}

VolumeMonitor::~VolumeMonitor()
{
    stop();
}

void VolumeMonitor::setTarget(const std::shared_ptr<AudioObject>& target)
{
    if (m_target.lock() == target)
        return;
    stop();
    m_target = target;
    targetChanged.emit();
    if (!target || !m_backend->isConnected() || m_backend->object(target->index()) != target
        || target->kind() == AudioObject::Card)
        return;
    const std::string serial = target->property(PW_KEY_OBJECT_SERIAL);
    if (serial.empty())
        return;
    const bool sink = target->kind() == AudioObject::Sink;
    // A passive link never wakes its peer: right for a sink, which plays anyway,
    // but a microphone nobody else records from would stay suspended.
    auto* properties = pw_properties_new(PW_KEY_MEDIA_TYPE, "Audio", PW_KEY_MEDIA_CATEGORY, "Capture",
                                         PW_KEY_NODE_NAME, "lingmo.internal.monitor", PW_KEY_MEDIA_ROLE, "DSP",
                                         PW_KEY_TARGET_OBJECT, serial.c_str(), "stream.capture.sink",
                                         sink ? "true" : "false", "node.passive", sink ? "true" : "false",
                                         "node.dont-fallback", "true", "node.linger", "true", nullptr);
    m_stream = pw_stream_new(m_backend->core(), "Cutefish volume monitor", properties);
    if (!m_stream)
        return;
    pw_stream_add_listener(m_stream, &m_listener, &streamEvents, this);
    spa_audio_info_raw format{};
    format.format = SPA_AUDIO_FORMAT_F32;
    format.rate = 48000;
    format.channels = 1;
    format.position[0] = SPA_AUDIO_CHANNEL_MONO;
    uint8_t buffer[512];
    spa_pod_builder builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
    const spa_pod* params[] = {spa_format_audio_raw_build(&builder, SPA_PARAM_EnumFormat, &format)};
    if (pw_stream_connect(m_stream, PW_DIRECTION_INPUT, PW_ID_ANY,
                          static_cast<pw_stream_flags>(PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS), params,
                          1)
        < 0)
        stop();
}

void VolumeMonitor::process()
{
    double peak = 0.0;
    uint32_t frames = 0;
    pw_buffer* buffer;
    while ((buffer = pw_stream_dequeue_buffer(m_stream))) {
        const spa_buffer* spaBuffer = buffer->buffer;
        if (spaBuffer->n_datas > 0) {
            const spa_data& data = spaBuffer->datas[0];
            if (data.data && data.chunk && data.chunk->offset <= data.maxsize) {
                const uint32_t size = std::min(data.chunk->size, data.maxsize - data.chunk->offset);
                const auto* samples = static_cast<const uint8_t*>(data.data) + data.chunk->offset;
                frames += size / sizeof(float);
                for (uint32_t offset = 0; offset + sizeof(float) <= size; offset += sizeof(float)) {
                    float value;
                    std::memcpy(&value, samples + offset, sizeof(value));
                    if (std::isfinite(value))
                        peak = std::max(peak, double(std::abs(value)));
                }
            }
        }
        pw_stream_queue_buffer(m_stream, buffer);
    }
    if (frames == 0)
        return;
    // Rise at once but fall at 30 dB/s, as a level meter does; raw per-buffer
    // peaks jump by tens of dB and make the meter flicker.
    const double fall = std::pow(10.0, -30.0 / 20.0 * frames / 48000.0);
    updateVolume(std::clamp(std::max(peak, m_volume * fall), 0.0, 1.0));
}

void VolumeMonitor::updateVolume(double volume)
{
    if (m_volume != volume) {
        m_volume = volume;
        volumeChanged.emit();
    }
}

void VolumeMonitor::updateAvailable(bool available)
{
    if (m_available != available) {
        m_available = available;
        availableChanged.emit();
    }
}

void VolumeMonitor::stop()
{
    if (m_stream) {
        spa_hook_remove(&m_listener);
        pw_stream_destroy(m_stream);
        m_stream = nullptr;
    }
    updateAvailable(false);
    updateVolume(0.0);
}

} // namespace Lingmo::Audio
