/*
    SPDX-FileCopyrightText: 2023 Bharadwaj Raju <bharadwaj.raju777@gmail.com>
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <KConfigWatcher>
#include <kdedmodule.h>

#include <PulseAudioQt/Context>
#include <PulseAudioQt/Models>

#include "globalconfig.h"
#include "osdservice.h"
#include "preferreddevice.h"
#include "volumefeedback.h"

namespace PulseAudioQt
{
class Device;
} // namespace PulseAudioQt

using namespace Qt::Literals::StringLiterals;

// DEFAULT_SINK_NAME in module-always-sink.c
constexpr QLatin1String DUMMY_OUTPUT_NAME = "auto_null"_L1;

class AudioShortcutsService : public KDEDModule
{
    Q_OBJECT
public:
    AudioShortcutsService(QObject *parent, const QList<QVariant> &);

private:
    static qint64 boundVolume(qint64 volume, int maxVolume);
    static int volumePercent(qint64 volume);
    static QString nameForDevice(const PulseAudioQt::Device *device);
    int changeVolumePercent(PulseAudioQt::Device *device, int deltaPercent);
    void handleDefaultSinkChange();
    void handleNewSink();
    void muteVolume();
    void enableGlobalMute();
    void disableGlobalMute();
    void playFeedback(int sinkIdx);
    void showMute(int percent);
    void showVolume(int percent);
    void showMicMute(int percent);
    void showMicVolume(int percent);

    PulseAudioQt::SinkModel *m_sinkModel = nullptr;
    PulseAudioQt::SourceModel *m_sourceModel = nullptr;
    PulseAudioQt::CardModel *m_cardModel = nullptr;
    GlobalConfig *m_globalConfig;
    OsdServiceInterface *m_osdDBusInterface;
    VolumeFeedback *m_feedback;
    bool m_initialDefaultSinkSet = false;
    PreferredDevice m_preferredDevice;
};
