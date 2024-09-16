// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAUDIODEVICE_P_H
#define DAUDIODEVICE_P_H

#include "dtkaudiomanager_global.h"
#include "daudiodevice.h"
#include "daudiocard_p.h"
#include "daudiostream_p.h"
#include "dtkcore_global.h"

#include <QObject>
#include <DExpected>

DAUDIOMANAGER_BEGIN_NAMESPACE
DCORE_USE_NAMESPACE

class DPlatformAudioDevice : public QObject, public QSharedData
{
    Q_OBJECT
public:
    explicit DPlatformAudioDevice(DPlatformAudioCard *card = nullptr);
    virtual ~DPlatformAudioDevice() override;

    virtual DAudioDevice *create() = 0;

    virtual bool mute() const = 0;
    virtual double fade() const = 0;
    virtual double volume() const = 0;
    virtual double balance() const = 0;

    virtual bool supportBalance() const = 0;
    virtual bool supportFade() const = 0;
    virtual double baseVolume() const = 0;

    virtual double meterVolume() const = 0;
    virtual QString activePort() const = 0;
    virtual void setActivePort(const QString &portName) = 0;
    virtual QList<QString> ports() const = 0;

    bool isPlay() const;
    void setIsPlay(bool isPlay);
    bool isDefault() const;
    void setDefault(bool isDefault);
    quint32 card() const;
    void addStream(DPlatformAudioStream *stream);
    void removeStream(const QString &streamName);
    DPlatformAudioStream *stream(const QString &streamName);

    QString name() const;
    void setName(const QString &name);
    QString description() const;
    void setDescription(const QString &description);

public Q_SLOTS:
    virtual DExpected<void> setMute(bool mute) = 0;
    virtual DExpected<void> setFade(double fade) = 0;
    virtual DExpected<void> setVolume(double volume) = 0;
    virtual DExpected<void> setBalance(double balance) = 0;

Q_SIGNALS:
    void streamAdded(const QString &name);
    void streamRemoved(const QString &name);

    void muteChanged(bool mute);
    void fadeChanged(double fade);
    void volumeChanged(double volume);
    void balanceChanged(double balance);

    void supportBalanceChanged(bool supportBalance);
    void supportFadeChanged(bool supportFade);
    void baseVolumeChanged(double baseVolume);

    void meterVolumeChanged(double meterVolume);

    void nameChanged(QString name);
    void descriptionChanged(QString description);
    void activePortChanged(QString portName);

public:
    DPlatformAudioCard *m_card = nullptr;
    QList<QExplicitlySharedDataPointer<DPlatformAudioStream>> m_streams;
    QString m_key;
    QString m_name;
    QString m_description;
    bool m_isDefault = false;
    bool m_isPlay = false;
};

class DPlatformAudioInputDevice : public DPlatformAudioDevice
{
    Q_OBJECT
public:
    explicit DPlatformAudioInputDevice(DPlatformAudioCard *card = nullptr);
    virtual ~DPlatformAudioInputDevice() override;

    virtual DAudioDevice *create() override
    {
        return new DAudioInputDevice(this);
    }
};

class DPlatformAudioOutputDevice : public DPlatformAudioDevice
{
    Q_OBJECT
public:
    explicit DPlatformAudioOutputDevice(DPlatformAudioCard *card = nullptr);
    virtual ~DPlatformAudioOutputDevice() override;

    virtual DAudioDevice *create() override
    {
        return new DAudioOutputDevice(this);
    }
};
DAUDIOMANAGER_END_NAMESPACE

#endif
