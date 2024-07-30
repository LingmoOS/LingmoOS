/*
    SPDX-FileCopyrightText: 2014-2015 Harald Sitter <sitter@kde.org>
    SPDX-FileCopyrightText: 2021 Nicolas Fella

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <PulseAudioQt/Sink>

#include <QObject>

class SoundThemeConfig;

class SpeakerTest : public QObject
{
    Q_OBJECT
    Q_PROPERTY(PulseAudioQt::Sink *sink READ sink WRITE setSink NOTIFY sinkChanged)
    Q_PROPERTY(QStringList playingChannels READ playingChannels NOTIFY playingChannelsChanged)
public:
    explicit SpeakerTest(QObject *parent = nullptr);

    PulseAudioQt::Sink *sink() const;
    void setSink(PulseAudioQt::Sink *sink);
    Q_SIGNAL void sinkChanged();

    QStringList playingChannels() const;
    Q_SIGNAL void playingChannelsChanged();
    void playingFinished(const QString &name, int errorCode);

    Q_INVOKABLE void testChannel(const QString &name);
    Q_SIGNAL void showErrorMessage(const QString &message);

private:
    PulseAudioQt::Sink *m_sink;
    QStringList m_playingChannels;
    SoundThemeConfig *m_config = nullptr;
};
