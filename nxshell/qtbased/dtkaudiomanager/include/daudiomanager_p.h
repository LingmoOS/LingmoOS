// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAUDIOMANAGER_P_H
#define DAUDIOMANAGER_P_H

#include "daudiocard_p.h"
#include "daudiodevice_p.h"
#include "daudiostream_p.h"
#include "dtkaudiomanager_global.h"

#include <QObject>
#include <QSharedDataPointer>
#include <DExpected>

DAUDIOMANAGER_BEGIN_NAMESPACE
DCORE_USE_NAMESPACE

class DPlatformAudioCard;
class DPlatformAudioInputDevice;
class DPlatformAudioOutputDevice;
class DAudioManagerPrivate : public QObject
{
    Q_OBJECT

public:
    explicit DAudioManagerPrivate(QObject *parent = nullptr);
    virtual ~DAudioManagerPrivate();
    virtual void reset() = 0;
    virtual void setReConnectionEnabled(const bool enable) = 0;

    virtual bool increaseVolume() const = 0;
    virtual bool reduceNoise() const = 0;
    virtual double maxVolume() const = 0;

    void addCard(DPlatformAudioCard *card);
    void removeCard(const quint32 cardId);
    DPlatformAudioCard *cardById(const quint32 cardId) const;
    void addInputDevice(DPlatformAudioInputDevice *device);
    void removeInputDevice(const QString &deviceName);
    void addOutputDevice(DPlatformAudioOutputDevice *device);
    void removeOutputDevice(const QString &deviceName);

public Q_SLOTS:
    virtual DExpected<void> setIncreaseVolume(bool increaseVolume) = 0;
    virtual DExpected<void> setReduceNoise(bool reduceNoise) = 0;

Q_SIGNALS:
    void deviceAdded(const QString &name, const bool isInputDevice);
    void deviceRemoved(const QString &name, const bool isInputDevice);
    void cardsChanged();
    void defaultInputDeviceChanged(const QString &name);
    void defaultOutputDeviceChanged(const QString &name);

    void increaseVolumeChanged(bool increaseVolume);
    void reduceNoiseChanged(bool reduceNoise);
    void maxVolumeChanged(double maxVolume);

public:
    QList<QExplicitlySharedDataPointer<DPlatformAudioCard>> m_cards;
    QList<QExplicitlySharedDataPointer<DPlatformAudioInputDevice>> m_inputDevices;
    QList<QExplicitlySharedDataPointer<DPlatformAudioOutputDevice>> m_outputDevices;
};
DAUDIOMANAGER_END_NAMESPACE

#endif
