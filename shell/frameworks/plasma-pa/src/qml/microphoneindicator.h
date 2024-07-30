/*
    SPDX-FileCopyrightText: 2019 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2020 MBition GmbH
        Author: Kai Uwe Broulik <kai_uwe.broulik@mbition.io>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include "osdservice.h"

#include <QList>
#include <QObject>
#include <QPersistentModelIndex>
#include <QPointer>

#include "preferreddevice.h"

class QAction;
class QTimer;

class KStatusNotifierItem;

namespace PulseAudioQt
{
class Source;
class SourceModel;
class SourceOutputModel;
}

class MicrophoneIndicator : public QObject
{
    Q_OBJECT

public:
    explicit MicrophoneIndicator(QObject *parent = nullptr);
    ~MicrophoneIndicator() override;

    Q_INVOKABLE void init();

Q_SIGNALS:
    void enabledChanged();

private:
    void scheduleUpdate();
    void update();

    bool muted() const;
    void setMuted(bool muted);
    void toggleMuted();

    void adjustVolume(int direction);

    static int volumePercent(PulseAudioQt::Source *source);
    void showOsd();

    QList<QModelIndex> recordingApplications() const;
    QString toolTipForApps(const QList<QModelIndex> &apps) const;
    QString sourceOutputDisplayName(const QModelIndex &idx) const;

    PreferredDevice m_preferredDevice;
    PulseAudioQt::SourceModel *const m_sourceModel = nullptr; // microphone devices
    PulseAudioQt::SourceOutputModel *const m_sourceOutputModel = nullptr; // recording streams

    KStatusNotifierItem *m_sni = nullptr;
    QPointer<QAction> m_muteAction;
    QPointer<QAction> m_dontAgainAction;

    QList<QPersistentModelIndex> m_mutedIndices;

    OsdServiceInterface *m_osdDBusInterface = nullptr;
    bool m_showOsdOnUpdate = false;

    int m_wheelDelta = 0;

    QTimer *const m_updateTimer;
};
