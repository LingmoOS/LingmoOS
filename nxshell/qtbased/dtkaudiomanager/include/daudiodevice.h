// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later


#ifndef DAUDIODEVICE_H
#define DAUDIODEVICE_H

#include <dtkaudiomanager_global.h>
#include <QObject>
#include <QSharedDataPointer>
#include <DExpected>

DAUDIOMANAGER_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DExpected;

class DAudioCard;
class DAudioPort;
class DAudioStream;
class DAudioInputStream;
class DAudioOutputStream;
using DAudioPortPtr = QSharedPointer<DAudioPort>;
using DAudioStreamPtr = QSharedPointer<DAudioStream>;
using DAudioInputStreamPtr = QSharedPointer<DAudioInputStream>;
using DAudioOutputStreamPtr = QSharedPointer<DAudioOutputStream>;
class DPlatformAudioDevice;
class LIBDTKAUDIOMANAGERSHARED_EXPORT DAudioDevice : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool mute READ mute WRITE setMute NOTIFY muteChanged)
    Q_PROPERTY(double fade READ fade WRITE setFade NOTIFY fadeChanged)
    Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(double balance READ balance WRITE setBalance NOTIFY balanceChanged)
    Q_PROPERTY(bool supportBalance READ supportBalance NOTIFY supportBalanceChanged)
    Q_PROPERTY(bool supportFade READ supportFade NOTIFY supportFadeChanged)
    Q_PROPERTY(double baseVolume READ baseVolume NOTIFY baseVolumeChanged)
    Q_PROPERTY(double meterVolume READ meterVolume NOTIFY meterVolumeChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
    Q_PROPERTY(bool isPlay READ isPlay WRITE setIsPlay NOTIFY isPlayChanged)
    Q_PROPERTY(QString activePort READ activePort WRITE setActivePort NOTIFY activePortChanged)

public:
    explicit DAudioDevice(DPlatformAudioDevice *d, DAudioCard *parent = nullptr);
    virtual ~DAudioDevice() override;

    virtual bool mute() const;
    virtual double fade() const;
    virtual double volume() const;
    virtual double balance() const;
    virtual bool supportBalance() const;
    virtual bool supportFade() const;
    virtual double baseVolume() const;

    virtual double meterVolume() const;
    virtual QString activePort() const;
    virtual void setActivePort(const QString &portName) const;
    virtual QList<DAudioPortPtr> ports() const;

    QString name() const;
    QString description() const;
    quint32 card() const;

    QList<DAudioStreamPtr> streams() const;
    DAudioStreamPtr stream(const QString &streamName) const;

    bool isPlay() const;
    void setIsPlay(bool isPlay);

public Q_SLOTS:
    virtual DExpected<void> setMute(bool mute);
    virtual DExpected<void> setFade(double fade);
    virtual DExpected<void> setVolume(double volume);
    virtual DExpected<void> setBalance(double balance);


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
    void isPlayChanged(bool isPlay);
    void activePortChanged(QString portName);

protected:
    QExplicitlySharedDataPointer<DPlatformAudioDevice> d;
};

class DPlatformAudioInputDevice;
class LIBDTKAUDIOMANAGERSHARED_EXPORT DAudioInputDevice : public DAudioDevice
{
    Q_OBJECT

public:
    explicit DAudioInputDevice(DPlatformAudioInputDevice *d, DAudioCard *parent = nullptr);

private:
    Q_DISABLE_COPY(DAudioInputDevice)
    friend class DPlatformAudioInputDevice;
};

class DPlatformAudioOutputDevice;
class LIBDTKAUDIOMANAGERSHARED_EXPORT DAudioOutputDevice : public DAudioDevice
{
    Q_OBJECT

public:
    explicit DAudioOutputDevice(DPlatformAudioOutputDevice *d, DAudioCard *parent = nullptr);

private:
    Q_DISABLE_COPY(DAudioOutputDevice)
    friend class DPlatformAudioOutputDevice;
};
DAUDIOMANAGER_END_NAMESPACE

#endif
