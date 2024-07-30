/*
    SPDX-FileCopyrightText: 2014-2015 Harald Sitter <sitter@kde.org>
    SPDX-FileCopyrightText: 2021 Nicolas Fella

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "speakertest.h"

#include "canberracontext.h"
#include "soundthemeconfig.h"

namespace
{
struct CallbackData {
    SpeakerTest *object = nullptr;
    QString name;
};

// Wrapper for camberra *ca_finish_callback_t
void finish_callback(ca_context *c, unsigned int id, int error_code, void *userdata)
{
    Q_UNUSED(c)
    Q_UNUSED(id)

    if (userdata == nullptr) {
        return;
    }

    auto *cb_data = static_cast<CallbackData *>(userdata);
    cb_data->object->playingFinished(cb_data->name, error_code);

    delete (cb_data);
};
}

SpeakerTest::SpeakerTest(QObject *parent)
    : QObject(parent)
    , m_config(new SoundThemeConfig(this))
{
}

PulseAudioQt::Sink *SpeakerTest::sink() const
{
    return m_sink;
}

void SpeakerTest::setSink(PulseAudioQt::Sink *sink)
{
    if (m_sink != sink) {
        m_sink = sink;
        Q_EMIT sinkChanged();
    }
}

void SpeakerTest::testChannel(const QString &name)
{
    auto context = PulseAudioQt::CanberraContext::instance()->canberra();
    if (!context) {
        return;
    }

    m_playingChannels << name;
    Q_EMIT playingChannelsChanged();

    ca_context_set_driver(context, "pulse");

    char dev[64];
    snprintf(dev, sizeof(dev), "%lu", (unsigned long)m_sink->index());
    ca_context_change_device(context, dev);

    void *cb_data = new CallbackData{this, name};

    ca_proplist *proplist;
    ca_proplist_create(&proplist);

    ca_proplist_sets(proplist, CA_PROP_MEDIA_ROLE, "test");
    ca_proplist_sets(proplist, CA_PROP_MEDIA_NAME, name.toLatin1().constData());
    ca_proplist_sets(proplist, CA_PROP_CANBERRA_FORCE_CHANNEL, name.toLatin1().data());
    ca_proplist_sets(proplist, CA_PROP_CANBERRA_ENABLE, "1");
    ca_proplist_sets(proplist, CA_PROP_CANBERRA_XDG_THEME_NAME, m_config->soundTheme().toLatin1().constData());

    // there is no subwoofer sound in the freedesktop theme https://gitlab.freedesktop.org/xdg/xdg-sound-theme/-/issues/7
    const QString sound_name = (name == QLatin1String("lfe")) ? QStringLiteral("audio-channel-rear-center") : QStringLiteral("audio-channel-%1").arg(name);

    int error_code = CA_SUCCESS;
    for (const QString &soundName : {sound_name,
                                     QStringLiteral("audio-test-signal"), // Fallback sounds
                                     QStringLiteral("bell-window-system"),
                                     QString()}) {
        if (soundName.isEmpty()) {
            // We are here because all of the fallback sounds failed to play
            playingFinished(name, error_code);
            break;
        }

        ca_proplist_sets(proplist, CA_PROP_EVENT_ID, soundName.toLatin1().data());
        error_code = ca_context_play_full(context, 0, proplist, finish_callback, cb_data);
        if (error_code == CA_SUCCESS) {
            break;
        }
    }

    ca_context_change_device(context, nullptr);
    ca_proplist_destroy(proplist);
}

QStringList SpeakerTest::playingChannels() const
{
    return m_playingChannels;
}

void SpeakerTest::playingFinished(const QString &name, int errorCode)
{
    m_playingChannels.removeOne(name);
    Q_EMIT playingChannelsChanged();

    if (errorCode != CA_SUCCESS) {
        Q_EMIT showErrorMessage(ca_strerror(errorCode));
    };
}
