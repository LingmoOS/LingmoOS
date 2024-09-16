// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAUDIOSTREAM_P_H
#define DAUDIOSTREAM_P_H

#include "daudiostream.h"
#include "dtkaudiomanager_global.h"

#include <QObject>
#include <DExpected>

DAUDIOMANAGER_BEGIN_NAMESPACE
DCORE_USE_NAMESPACE

class DAudioDevice;
class DPlatformAudioDevice;
class DPlatformAudioInputDevice;
class DPlatformAudioOutputDevice;
class DPlatformAudioStream : public QObject, public QSharedData
{
    Q_OBJECT

    Q_PROPERTY(bool mute READ mute WRITE setMute NOTIFY muteChanged)
    Q_PROPERTY(double fade READ fade WRITE setFade NOTIFY fadeChanged)
    Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(double balance READ balance WRITE setBalance NOTIFY balanceChanged)
    Q_PROPERTY(bool supportBalance READ supportBalance NOTIFY supportBalanceChanged)
    Q_PROPERTY(bool supportFade READ supportFade NOTIFY supportFadeChanged)
    Q_PROPERTY(quint32 card READ card NOTIFY cardChanged)
public:
    explicit DPlatformAudioStream(DPlatformAudioDevice *parent = nullptr);
    virtual bool mute() const = 0;
    virtual double fade() const = 0;
    virtual double volume() const = 0;
    virtual double balance() const = 0;

    virtual bool supportBalance() const = 0;
    virtual bool supportFade() const = 0;

    virtual DAudioStream *create() = 0;

    quint32 card() const;
    QString name() const;
    void setName(const QString &name);
    bool isPlay() const;
    void setIsPlay(bool isPlay);
public Q_SLOTS:
    virtual DExpected<void> setMute(bool mute) = 0;
    virtual DExpected<void> setFade(double fade) = 0;
    virtual DExpected<void> setVolume(double volume) = 0;
    virtual DExpected<void> setBalance(double balance) = 0;

Q_SIGNALS:
    void muteChanged(bool mute);
    void fadeChanged(double fade);
    void volumeChanged(double volume);
    void balanceChanged(double balance);

    void supportBalanceChanged(bool supportBalance);
    void supportFadeChanged(bool supportFade);

    void cardChanged(quint32 card);

protected:
    QString m_name;
    DPlatformAudioDevice *m_device = nullptr;
    bool m_isPlay = false;
};

class DPlatformAudioInputStream : public DPlatformAudioStream
{
    Q_OBJECT

public:
    explicit DPlatformAudioInputStream(DPlatformAudioOutputDevice *parent = nullptr);
    virtual ~DPlatformAudioInputStream() override;

    virtual DAudioStream *create() override { return new DAudioInputStream(this); }
};

class LIBDTKAUDIOMANAGERSHARED_EXPORT DPlatformAudioOutputStream : public DPlatformAudioStream
{
    Q_OBJECT

public:
    explicit DPlatformAudioOutputStream(DPlatformAudioInputDevice *parent = nullptr);
    virtual ~DPlatformAudioOutputStream() override;

    virtual DAudioStream *create() override { return new DAudioOutputStream(this); }
};
DAUDIOMANAGER_END_NAMESPACE

#endif
