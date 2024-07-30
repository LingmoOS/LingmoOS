/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2018 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "mediaendpoint_p.h"

#include "a2dp-codecs.h"
#include "services.h"

namespace BluezQt
{
MediaEndpointPrivate::MediaEndpointPrivate(const MediaEndpoint::Configuration &configuration)
    : m_configuration(configuration)
{
    init(configuration);
}

void MediaEndpointPrivate::init(const MediaEndpoint::Configuration &configuration)
{
    const QString uuid = QStringLiteral("UUID");
    const QString codec = QStringLiteral("Codec");
    const QString capabilities = QStringLiteral("Capabilities");

    QString objectPath = QStringLiteral("/MediaEndpoint");

    switch (configuration.role) {
    case MediaEndpoint::Role::AudioSource:
        m_properties[uuid] = Services::AudioSource;
        objectPath += QStringLiteral("/Source");
        break;
    case MediaEndpoint::Role::AudioSink:
        m_properties[uuid] = Services::AudioSink;
        objectPath += QStringLiteral("/Sink");
        break;
    }

    switch (configuration.codec) {
    case MediaEndpoint::Codec::Sbc:
        m_properties[codec] = QVariant::fromValue(uchar(A2DP_CODEC_SBC));
        m_properties[capabilities] = QByteArray(reinterpret_cast<const char *>(&sbcCapabilities), sizeof(sbcCapabilities));
        objectPath += QStringLiteral("/Sbc");
        break;
    case MediaEndpoint::Codec::Aac:
        m_properties[codec] = QVariant::fromValue(uchar(A2DP_CODEC_MPEG24));
        m_properties[capabilities] = QByteArray(reinterpret_cast<const char *>(&aacCapabilities), sizeof(aacCapabilities));
        objectPath += QStringLiteral("/Aac");
        break;
    }

    m_objectPath.setPath(objectPath);
}

} // namespace BluezQt
