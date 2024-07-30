/*
    SPDX-FileCopyrightText: 2023 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include "pipewireproduce_p.h"

class PipeWireEncodeProduce : public PipeWireProduce
{
    Q_OBJECT
public:
    PipeWireEncodeProduce(PipeWireBaseEncodedStream::Encoder encoder, uint nodeId, uint fd, const Fraction &framerate, PipeWireEncodedStream *stream);

    void processPacket(AVPacket *packet) override;
    void processFrame(const PipeWireFrame &frame) override;

Q_SIGNALS:
    void newPacket(const PipeWireEncodedStream::Packet &packetData);

private:
    PipeWireEncodedStream *const m_encodedStream;
    QSize m_size;
    PipeWireCursor m_cursor;
};
