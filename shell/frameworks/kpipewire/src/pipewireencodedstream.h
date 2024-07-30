/*
    SPDX-FileCopyrightText: 2023 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <QObject>

#include "pipewirebaseencodedstream.h"
#include <kpipewire_export.h>

struct PipeWireCursor;
class PipeWirePacketPrivate;

class KPIPEWIRE_EXPORT PipeWireEncodedStream : public PipeWireBaseEncodedStream
{
    Q_OBJECT
public:
    PipeWireEncodedStream(QObject *parent = nullptr);
    ~PipeWireEncodedStream() override;

    class Packet
    {
    public:
        Packet(bool isKey, const QByteArray &data);

        /// Whether the packet represents a key frame
        bool isKeyFrame() const;
        QByteArray data() const;

        std::shared_ptr<PipeWirePacketPrivate> d;
    };

Q_SIGNALS:
    /// will be emitted when the stream initializes as well as when the value changes
    void sizeChanged(const QSize &size);
    void cursorChanged(const PipeWireCursor &cursor);
    void newPacket(const Packet &packet);

protected:
    std::unique_ptr<PipeWireProduce> makeProduce() override;
};
