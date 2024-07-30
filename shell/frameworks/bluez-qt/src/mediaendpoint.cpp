/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2018 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "mediaendpoint.h"
#include "a2dp-codecs.h"
#include "mediaendpoint_p.h"

namespace BluezQt
{
MediaEndpoint::MediaEndpoint(const Configuration &configuration, QObject *parent)
    : QObject(parent)
    , d(new MediaEndpointPrivate(configuration))
{
}

MediaEndpoint::~MediaEndpoint() = default;

QDBusObjectPath MediaEndpoint::objectPath() const
{
    return d->m_objectPath;
}

const QVariantMap &MediaEndpoint::properties() const
{
    return d->m_properties;
}

void MediaEndpoint::setConfiguration(const QString &transportObjectPath, const QVariantMap &properties)
{
    Q_EMIT configurationSet(transportObjectPath, properties);
}

void MediaEndpoint::selectConfiguration(const QByteArray &capabilities, const Request<QByteArray> &request)
{
    switch (d->m_configuration.codec) {
    case MediaEndpoint::Codec::Sbc: {
        if (capabilities.size() != sizeof(a2dp_sbc_t)) {
            Q_EMIT configurationSelected(capabilities, QByteArray());
            request.reject();
            return;
        }

        a2dp_sbc_t caps = *reinterpret_cast<const a2dp_sbc_t *>(capabilities.constData());
        if (caps.frequency & SBC_SAMPLING_FREQ_44100) {
            caps.frequency = SBC_SAMPLING_FREQ_44100;
        } else if (caps.frequency & SBC_SAMPLING_FREQ_48000) {
            caps.frequency = SBC_SAMPLING_FREQ_48000;
        } else {
            break;
        }

        if (caps.channel_mode & SBC_CHANNEL_MODE_STEREO) {
            caps.channel_mode = SBC_CHANNEL_MODE_STEREO;
        } else if (caps.channel_mode & SBC_CHANNEL_MODE_JOINT_STEREO) {
            caps.channel_mode = SBC_CHANNEL_MODE_JOINT_STEREO;
        } else {
            break;
        }

        if (caps.block_length & SBC_BLOCK_LENGTH_16) {
            caps.block_length = SBC_BLOCK_LENGTH_16;
        } else if (caps.block_length & SBC_BLOCK_LENGTH_12) {
            caps.block_length = SBC_BLOCK_LENGTH_12;
        } else if (caps.block_length & SBC_BLOCK_LENGTH_8) {
            caps.block_length = SBC_BLOCK_LENGTH_8;
        } else if (caps.block_length & SBC_BLOCK_LENGTH_4) {
            caps.block_length = SBC_BLOCK_LENGTH_4;
        } else {
            break;
        }

        if (caps.subbands & SBC_SUBBANDS_8) {
            caps.subbands = SBC_SUBBANDS_8;
        } else if (caps.subbands & SBC_SUBBANDS_4) {
            caps.subbands = SBC_SUBBANDS_4;
        } else {
            break;
        }

        if (caps.allocation_method & SBC_ALLOCATION_LOUDNESS) {
            caps.allocation_method = SBC_ALLOCATION_LOUDNESS;
        } else if (caps.allocation_method & SBC_ALLOCATION_SNR) {
            caps.allocation_method = SBC_ALLOCATION_SNR;
        } else {
            break;
        }

        caps.min_bitpool = std::max(caps.min_bitpool, static_cast<uint8_t>(MIN_BITPOOL));
        caps.max_bitpool = std::min(caps.max_bitpool, static_cast<uint8_t>(MAX_BITPOOL));

        const QByteArray configuration(reinterpret_cast<const char *>(&caps), sizeof(caps));
        Q_EMIT configurationSelected(capabilities, configuration);
        request.accept(configuration);
        return;

        break;
    }
    case MediaEndpoint::Codec::Aac:
        if (capabilities.size() != sizeof(a2dp_aac_t)) {
            Q_EMIT configurationSelected(capabilities, QByteArray());
            request.reject();
            return;
        }

        // TODO: implement AAC. However selectConfiguration seems not to be used by iOS nor Android.
        Q_EMIT configurationSelected(capabilities, QByteArray());
        request.reject();
        return;

        break;
    }

    Q_EMIT configurationSelected(capabilities, QByteArray());
    request.reject();
}

void MediaEndpoint::clearConfiguration(const QString &transportObjectPath)
{
    Q_EMIT configurationCleared(transportObjectPath);
}

void MediaEndpoint::release()
{
}

} // namespace BluezQt

#include "moc_mediaendpoint.cpp"
