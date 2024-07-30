// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2014-2015 Harald Sitter <sitter@kde.org>
// SPDX-FileCopyrightText: 2016 David Rosca <nowrep@gmail.com>

#pragma once

#include <QObject>

#include <PulseAudioQt/Sink>
#include <PulseAudioQt/Source>

class PreferredDevice : public QObject
{
    Q_OBJECT
    Q_PROPERTY(PulseAudioQt::Sink *sink MEMBER m_sink NOTIFY sinkChanged)
    Q_PROPERTY(PulseAudioQt::Source *source MEMBER m_source NOTIFY sourceChanged)
public:
    explicit PreferredDevice(QObject *parent = nullptr);
    [[nodiscard]] PulseAudioQt::Sink *sink() const;
    [[nodiscard]] PulseAudioQt::Source *source() const;

Q_SIGNALS:
    void sinkChanged();
    void sourceChanged();

private:
    void updatePreferredSink();
    void updatePreferredSource();
    [[nodiscard]] PulseAudioQt::Sink *findPreferredSink() const;
    PulseAudioQt::Sink *m_sink = nullptr;
    PulseAudioQt::Source *m_source = nullptr;
};
