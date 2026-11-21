// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "lingmo/audio/audio_model.h"
#include "lingmo/audio/audio_object.h"
#include "lingmo/audio/backend.h"
#include "shell/osd/osd.h"
#include "shell/statusbar/brightness.h"

namespace Lingmo::Shell {

namespace {

// The levels the on-screen display shows: the default sink and source, the
// screen and the keyboard backlight.
class SessionOsdLevels final : public OsdLevels
{
public:
    explicit SessionOsdLevels(base::dbus::Connection *bus)
        : m_backend(audio::Backend::shared())
        , m_sinks(audio::AudioObject::Sink, m_backend)
        , m_sources(audio::AudioObject::Source, m_backend)
        , m_brightness(bus)
    {
        const auto changed = [this] { this->changed(); };
        for (audio::AudioModel *model : {&m_sinks, &m_sources}) {
            m_subscriptions.push_back(model->dataChanged.connect(changed));
            m_subscriptions.push_back(model->defaultNodeChanged.connect(changed));
            m_subscriptions.push_back(model->rowsChanged.connect(changed));
        }
        m_subscriptions.push_back(m_brightness.valueChanged.connect(changed));
        m_subscriptions.push_back(m_brightness.enabledChanged.connect(changed));
        m_subscriptions.push_back(m_keyboard.valueChanged.connect(changed));
        m_subscriptions.push_back(m_keyboard.maximumChanged.connect(changed));
        m_subscriptions.push_back(m_keyboard.changedByFirmware.connect([this] {
            keyboardChangedByFirmware();
        }));
    }

    bool hasSink() const override { return bool(m_sinks.defaultNode()); }
    double volume() const override { return sink() ? sink()->volume() : 0; }
    bool muted() const override { return sink() && sink()->muted(); }
    void setVolume(double volume) override
    {
        if (auto node = sink())
            node->setVolume(volume);
    }
    void setMuted(bool muted) override
    {
        if (auto node = sink())
            node->setMuted(muted);
    }

    bool hasMicrophone() const override { return bool(m_sources.defaultNode()); }
    double microphoneVolume() const override { return source() ? source()->volume() : 0; }
    bool microphoneMuted() const override { return source() && source()->muted(); }
    void setMicrophoneMuted(bool muted) override
    {
        if (auto node = source())
            node->setMuted(muted);
    }

    bool brightnessEnabled() const override { return m_brightness.isEnabled(); }
    int brightness() const override { return m_brightness.value(); }
    void setBrightness(int value) override { m_brightness.setValue(value); }

    int keyboardMaximum() const override { return m_keyboard.maximum(); }
    int keyboardBrightness() const override { return m_keyboard.value(); }
    void setKeyboardBrightness(int value) override { m_keyboard.setValue(value); }

private:
    std::shared_ptr<audio::AudioObject> sink() const { return m_sinks.defaultNode(); }
    std::shared_ptr<audio::AudioObject> source() const { return m_sources.defaultNode(); }

    std::shared_ptr<audio::Backend> m_backend;
    audio::AudioModel m_sinks;
    audio::AudioModel m_sources;
    Brightness m_brightness;
    KeyboardBrightness m_keyboard;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace

std::unique_ptr<OsdLevels> createSessionOsdLevels(base::dbus::Connection *sessionBus)
{
    return std::make_unique<SessionOsdLevels>(sessionBus);
}

} // namespace Lingmo::Shell
