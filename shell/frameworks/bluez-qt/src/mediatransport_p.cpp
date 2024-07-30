/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "mediatransport_p.h"
#include "a2dp-codecs.h"
#include "macros.h"
#include "utils.h"

namespace BluezQt
{
static MediaTransport::State stringToState(const QString &state)
{
    if (state == QLatin1String("pending")) {
        return MediaTransport::State::Pending;
    } else if (state == QLatin1String("active")) {
        return MediaTransport::State::Active;
    }
    return MediaTransport::State::Idle;
}

static AudioCodec intToCodec(int value)
{
    switch (value) {
    case A2DP_CODEC_SBC:
        return AudioCodec::Sbc;
        break;
    case A2DP_CODEC_MPEG24:
        return AudioCodec::Aac;
        break;
    }

    return AudioCodec::Invalid;
}

static AudioSampleRate byteArrayToSampleRate(AudioCodec codec, const QByteArray &buffer)
{
    switch (codec) {
    case AudioCodec::Sbc: {
        if (buffer.size() != sizeof(a2dp_sbc_t)) {
            return AudioSampleRate::Invalid;
        }

        a2dp_sbc_t sbcConfig = *reinterpret_cast<const a2dp_sbc_t *>(buffer.constData());
        switch (sbcConfig.frequency) {
        case SBC_SAMPLING_FREQ_44100:
            return AudioSampleRate::Rate44100;
            break;
        case SBC_SAMPLING_FREQ_48000:
            return AudioSampleRate::Rate48000;
            break;
        }
        break;
    }
    case AudioCodec::Aac: {
        if (buffer.size() != sizeof(a2dp_aac_t)) {
            return AudioSampleRate::Invalid;
        }

        a2dp_aac_t aacConfig = *reinterpret_cast<const a2dp_aac_t *>(buffer.constData());
        switch (AAC_GET_FREQUENCY(aacConfig)) {
        case AAC_SAMPLING_FREQ_44100:
            return AudioSampleRate::Rate44100;
            break;
        case AAC_SAMPLING_FREQ_48000:
            return AudioSampleRate::Rate48000;
            break;
        }
        break;
    }
    default:
        break;
    }

    return AudioSampleRate::Invalid;
}

MediaTransportPrivate::MediaTransportPrivate(const QString &path, const QVariantMap &properties)
    : QObject()
    , m_dbusInterface(Strings::orgBluez(), path, DBusConnection::orgBluez())
    , m_dbusProperties(nullptr)
    , m_path(path)
{
    DBusConnection::orgBluez().connect(Strings::orgBluez(),
                                       path,
                                       Strings::orgFreedesktopDBusProperties(),
                                       QStringLiteral("PropertiesChanged"),
                                       this,
                                       SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
    init(properties);
}

void MediaTransportPrivate::init(const QVariantMap &properties)
{
    m_dbusProperties = new DBusProperties(Strings::orgBluez(), m_path, DBusConnection::orgBluez(), this);

    m_volume = properties.value(QStringLiteral("Volume")).toUInt();
    m_state = stringToState(properties.value(QStringLiteral("State")).toString());
    m_configuration.codec = intToCodec(properties.value(QStringLiteral("Codec")).toInt());
    m_configuration.sampleRate = byteArrayToSampleRate(m_configuration.codec, properties.value(QStringLiteral("Configuration")).toByteArray());
}

void MediaTransportPrivate::onPropertiesChanged(const QString &interface, const QVariantMap &changed, const QStringList &invalidated)
{
    if (interface != Strings::orgBluezMediaTransport1()) {
        return;
    }

    for (auto it = changed.constBegin(); it != changed.constEnd(); ++it) {
        const QString &key = it.key();
        const QVariant &value = it.value();

        if (key == QLatin1String("Volume")) {
            m_volume = value.toUInt();
            Q_EMIT q.lock()->volumeChanged(m_volume);
        } else if (key == QLatin1String("State")) {
            m_state = stringToState(value.toString());
            Q_EMIT q.lock()->stateChanged(m_state);
        }
    }

    for (const QString &property : invalidated) {
        if (property == QLatin1String("Volume")) {
            m_volume = 0;
            Q_EMIT q.lock()->volumeChanged(m_volume);
        } else if (property == QLatin1String("State")) {
            PROPERTY_INVALIDATED(m_state, MediaTransport::State::Idle, stateChanged);
        }
    }
}

} // namespace BluezQt

#include "moc_mediatransport_p.cpp"
