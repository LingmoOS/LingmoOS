/*
    SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <QObject>

#include "pipewirebaseencodedstream.h"
#include <kpipewire_export.h>

struct PipeWireRecordPrivate;

class KPIPEWIRE_EXPORT PipeWireRecord : public PipeWireBaseEncodedStream
{
    Q_OBJECT
    Q_PROPERTY(QString output READ output WRITE setOutput NOTIFY outputChanged)
    Q_PROPERTY(QString extension READ extension NOTIFY encoderChanged)
public:
    PipeWireRecord(QObject *parent = nullptr);
    ~PipeWireRecord() override;

    QString output() const;
    void setOutput(const QString &output);
    QString extension() const;

    // Only for compatibility with 5.27
    KPIPEWIRE_DEPRECATED QString currentExtension() const
    {
        return extension();
    }

Q_SIGNALS:
    void outputChanged(const QString &output);

private:
    std::unique_ptr<PipeWireProduce> makeProduce() override;

    QScopedPointer<PipeWireRecordPrivate> d;
};
