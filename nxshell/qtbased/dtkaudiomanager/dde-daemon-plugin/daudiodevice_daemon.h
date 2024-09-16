// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAUDIODEVICE_DAEMON_H
#define DAUDIODEVICE_DAEMON_H

#include "dtkaudiomanager_global.h"
#include "daudiodevice_p.h"

#include <QDBusArgument>
#include <QObject>
#include <DDBusInterface>
#include <DExpected>

DAUDIOMANAGER_BEGIN_NAMESPACE
using DTK_CORE_NAMESPACE::DDBusInterface;
using DTK_CORE_NAMESPACE::DExpected;
class DAudioCard;
class DAudioDevicePrivate;
struct DAudioPortInfo_p {
    QString name;
    QString description;
    uchar available;
};

const QDBusArgument &operator>>(const QDBusArgument &arg, DAudioPortInfo_p &port);
QDBusArgument &operator<<(QDBusArgument &arg, const DAudioPortInfo_p &port);

class DDAemonDeviceInterface : public QObject
{
    Q_OBJECT

public:
    explicit DDAemonDeviceInterface(DDBusInterface *inter, DPlatformAudioDevice *owner = nullptr);

    bool mute() const;
    double fade() const;
    double volume() const;
    double balance() const;
    bool supportBalance() const;
    bool supportFade() const;
    double baseVolume() const;
    QString activePort() const;
    void setActivePort(const QString &portName);
    QList<QString> ports() const;

public Q_SLOTS:
    DExpected<void> setMute(bool mute);
    DExpected<void> setFade(double fade);
    DExpected<void> setVolume(double volume);
    DExpected<void> setBalance(double balance);

Q_SIGNALS:
    void MuteChanged(bool mute);
    void FadeChanged(double fade);
    void VolumeChanged(double volume);
    void BalanceChanged(double balance);

    void SupportBalanceChanged(bool supportBalance);
    void SupportFadeChanged(bool supportFade);
    void BaseVolumeChanged(double baseVolume);
    void ActivePortChanged(DAudioPortInfo_p port);

public:
     QScopedPointer<DDBusInterface> m_inter;
     DPlatformAudioDevice *m_owner = nullptr;
};
class LIBDTKAUDIOMANAGERSHARED_EXPORT DDaemonAudioInputDevice : public DPlatformAudioInputDevice
{
    Q_OBJECT
public:
    explicit DDaemonAudioInputDevice(const QString &path, DPlatformAudioCard *card = nullptr);
    virtual ~DDaemonAudioInputDevice() override;

    virtual bool mute() const override;
    virtual double fade() const override;
    virtual double volume() const override;
    virtual double balance() const override;

    virtual bool supportBalance() const override;
    virtual bool supportFade() const override;
    virtual double baseVolume() const override;

    virtual double meterVolume() const override;
    virtual QString activePort() const override;
    virtual void setActivePort(const QString &portName) override;
    virtual QList<QString> ports() const override;

    void compareAndDestroyStreams(const QList<QString> &nowStreams);
public Q_SLOTS:
    virtual DExpected<void> setMute(bool mute) override;
    virtual DExpected<void> setFade(double fade) override;
    virtual DExpected<void> setVolume(double volume) override;
    virtual DExpected<void> setBalance(double balance) override;

private:
    void ensureMeter();

private:
    QScopedPointer<DDAemonDeviceInterface> m_interface;
    QScopedPointer<QDBusInterface> m_meterInter;
};

class LIBDTKAUDIOMANAGERSHARED_EXPORT DDaemonAudioOutputDevice : public DPlatformAudioOutputDevice
{
    Q_OBJECT
public:
    explicit DDaemonAudioOutputDevice(const QString &path, DPlatformAudioCard *parent = nullptr);
    virtual ~DDaemonAudioOutputDevice() override;

    virtual bool mute() const override;
    virtual double fade() const override;
    virtual double volume() const override;
    virtual double balance() const override;

    virtual bool supportBalance() const override;
    virtual bool supportFade() const override;
    virtual double baseVolume() const override;

    virtual double meterVolume() const override;
    virtual QString activePort() const override;
    virtual void setActivePort(const QString &portName) override;
    virtual QList<QString> ports() const override;

    void compareAndDestroyStreams(const QList<QString> &nowStreams);

public Q_SLOTS:
    virtual DExpected<void> setMute(bool mute) override;
    virtual DExpected<void> setFade(double fade) override;
    virtual DExpected<void> setVolume(double volume) override;
    virtual DExpected<void> setBalance(double balance) override;

private:
    QScopedPointer<DDAemonDeviceInterface> m_interface;
};
DAUDIOMANAGER_END_NAMESPACE

Q_DECLARE_METATYPE(DTK_AUDIOMANAGER_NAMESPACE::DAudioPortInfo_p)

#endif
