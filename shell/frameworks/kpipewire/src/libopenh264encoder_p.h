/*
    SPDX-FileCopyrightText: 2023 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2023 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2023 Arjen Hiemstra <ahiemstra@heimr.nl>
    SPDX-FileCopyrightText: 2024 Fabian Vogt <fabian@ritter-vogt.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "encoder_p.h"

/**
 * A software encoder that uses ffmpeg + libopenh264 to encode to H.264.
 */
class LibOpenH264Encoder : public SoftwareEncoder
{
public:
    LibOpenH264Encoder(H264Profile profile, PipeWireProduce *produce);

    bool initialize(const QSize &size) override;

protected:
    int percentageToAbsoluteQuality(const std::optional<quint8> &quality) override;
    void applyEncodingPreference(AVDictionary *options) override;

private:
    H264Profile m_profile = H264Profile::Main;
};
