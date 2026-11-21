// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

#include <algorithm>

#include <pipewire/pipewire.h>
#include <spa/param/audio/format-utils.h>

class TestStream
{
public:
    // A playback stream plays a constant level, which a volume monitor reads back. Each
    // stream is its own application, so WirePlumber restores no other stream's volume onto it.
    TestStream(pw_core *core, const std::string &name, const std::string &target, bool capture = false,
               float level = 0.0f)
        : m_capture(capture)
        , m_level(level)
    {
        // Controls must use a different client connection from the exported stream.
        m_core = pw_context_connect(pw_core_get_context(core), nullptr, 0);
        if (!m_core)
            return;
        auto *properties = pw_properties_new(PW_KEY_NODE_NAME, name.c_str(),
            PW_KEY_MEDIA_TYPE, "Audio", PW_KEY_MEDIA_CATEGORY, capture ? "Capture" : "Playback",
            PW_KEY_APP_NAME, name.c_str(), PW_KEY_TARGET_OBJECT, target.c_str(), nullptr);
        m_stream = pw_stream_new(m_core, name.c_str(), properties);
        if (!m_stream)
            return;
        pw_stream_add_listener(m_stream, &m_listener, &events, this);
        spa_audio_info_raw format {};
        format.format = SPA_AUDIO_FORMAT_F32;
        format.rate = 48000;
        format.channels = 2;
        format.position[0] = SPA_AUDIO_CHANNEL_FL;
        format.position[1] = SPA_AUDIO_CHANNEL_FR;
        uint8_t buffer[512];
        spa_pod_builder builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
        const spa_pod *params[] = {spa_format_audio_raw_build(&builder, SPA_PARAM_EnumFormat, &format)};
        connected = pw_stream_connect(m_stream, capture ? PW_DIRECTION_INPUT : PW_DIRECTION_OUTPUT,
            PW_ID_ANY, static_cast<pw_stream_flags>(PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS), params, 1) >= 0;
    }

    ~TestStream()
    {
        if (m_stream) {
            spa_hook_remove(&m_listener);
            pw_stream_destroy(m_stream);
        }
        if (m_core)
            pw_core_disconnect(m_core);
    }

    bool connected = false;
    bool streaming = false;

private:
    pw_core *m_core = nullptr;
    pw_stream *m_stream = nullptr;
    spa_hook m_listener {};
    bool m_capture;
    float m_level;

    inline static const pw_stream_events events = [] {
        pw_stream_events events {};
        events.version = PW_VERSION_STREAM_EVENTS;
        events.state_changed = [](void *data, pw_stream_state, pw_stream_state state, const char *) {
            static_cast<TestStream *>(data)->streaming = state == PW_STREAM_STATE_STREAMING;
        };
        events.process = [](void *data) {
            auto *self = static_cast<TestStream *>(data);
            pw_buffer *buffer = pw_stream_dequeue_buffer(self->m_stream);
            if (!buffer)
                return;
            if (!self->m_capture && buffer->buffer->n_datas > 0) {
                spa_data &data = buffer->buffer->datas[0];
                if (data.data && data.chunk) {
                    constexpr uint32_t stride = 2 * sizeof(float);
                    const uint32_t frames = buffer->requested
                        ? std::min<uint64_t>(buffer->requested, data.maxsize / stride) : data.maxsize / stride;
                    std::fill_n(static_cast<float *>(data.data), frames * 2, self->m_level);
                    data.chunk->offset = 0;
                    data.chunk->size = frames * stride;
                    data.chunk->stride = stride;
                    buffer->size = frames;
                }
            }
            pw_stream_queue_buffer(self->m_stream, buffer);
        };
        return events;
    }();
};
