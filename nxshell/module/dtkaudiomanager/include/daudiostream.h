// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAUDIOSTREAM_H
#define DAUDIOSTREAM_H

#include <dtkaudiomanager_global.h>
#include <dexpected.h>

#include <QObject>
#include <QSharedDataPointer>

DAUDIOMANAGER_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DExpected;

class DAudioDevice;
class DPlatformAudioStream;
class LIBDTKAUDIOMANAGERSHARED_EXPORT DAudioStream : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool mute READ mute WRITE setMute NOTIFY muteChanged)
    Q_PROPERTY(double fade READ fade WRITE setFade NOTIFY fadeChanged)
    Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(double balance READ balance WRITE setBalance NOTIFY balanceChanged)
    Q_PROPERTY(bool supportBalance READ supportBalance NOTIFY supportBalanceChanged)
    Q_PROPERTY(bool supportFade READ supportFade NOTIFY supportFadeChanged)
    Q_PROPERTY(quint32 card READ card NOTIFY cardChanged)
    Q_PROPERTY(bool isPlay READ isPlay WRITE setIsPlay NOTIFY isPlayChanged)
public:
    explicit DAudioStream(DPlatformAudioStream *d, DAudioDevice *parent = nullptr);
    virtual ~DAudioStream() override;

    virtual bool mute() const;
    virtual double fade() const;
    virtual double volume() const;
    virtual double balance() const;
    virtual bool supportBalance() const;
    virtual bool supportFade() const;

    quint32 card() const;

    QString name() const;
    bool isPlay() const;
    void setIsPlay(bool isPlay);
public Q_SLOTS:
    virtual DExpected<void> setMute(bool mute);
    virtual DExpected<void> setFade(double fade);
    virtual DExpected<void> setVolume(double volume);
    virtual DExpected<void> setBalance(double balance);

Q_SIGNALS:
    void muteChanged(bool mute);
    void fadeChanged(double fade);
    void volumeChanged(double volume);
    void balanceChanged(double balance);

    void supportBalanceChanged(bool supportBalance);
    void supportFadeChanged(bool supportFade);

    void cardChanged(quint32 card);
    void isPlayChanged(bool isPlay);

private:
    QExplicitlySharedDataPointer<DPlatformAudioStream> d;
};

class DPlatformAudioInputStream;
class LIBDTKAUDIOMANAGERSHARED_EXPORT DAudioInputStream : public DAudioStream
{
    Q_OBJECT

public:
    explicit DAudioInputStream(DPlatformAudioInputStream *d, DAudioDevice *parent = nullptr);
    virtual ~DAudioInputStream() override;

};

class DPlatformAudioOutputStream;
class LIBDTKAUDIOMANAGERSHARED_EXPORT DAudioOutputStream : public DAudioStream
{
    Q_OBJECT

public:
    explicit DAudioOutputStream(DPlatformAudioOutputStream *d, DAudioDevice *parent = nullptr);
    virtual ~DAudioOutputStream() override;
};
DAUDIOMANAGER_END_NAMESPACE

#endif
