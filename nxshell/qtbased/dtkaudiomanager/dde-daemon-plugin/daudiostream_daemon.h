// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAUDIOSTREAM_DAEMON_H
#define DAUDIOSTREAM_DAEMON_H

#include "dtkaudiomanager_global.h"
#include "daudiostream_p.h"
#include "daudiodevice_p.h"

#include <QObject>
#include <DDBusInterface>
#include <DExpected>

DAUDIOMANAGER_BEGIN_NAMESPACE
using DTK_CORE_NAMESPACE::DDBusInterface;
using DTK_CORE_NAMESPACE::DExpected;
class DAudioDevice;

class DPlatformAudioInputStream;
class DDAemonStreamInterface : public QObject
{
    Q_OBJECT
public:
    explicit DDAemonStreamInterface(DDBusInterface *inter, DPlatformAudioStream *owner = nullptr);

    bool mute() const;
    double fade() const;
    double volume() const;
    double balance() const;
    bool supportBalance() const;
    bool supportFade() const;

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

public:
     QScopedPointer<DDBusInterface> m_inter;
     DPlatformAudioStream *m_owner = nullptr;
};

class LIBDTKAUDIOMANAGERSHARED_EXPORT DDaemonInputStream : public DPlatformAudioInputStream
{
    Q_OBJECT

public:
    explicit DDaemonInputStream(const QString &path, DPlatformAudioOutputDevice *parent = nullptr);
    virtual ~DDaemonInputStream() override;

    virtual bool mute() const override;
    virtual double fade() const override;
    virtual double volume() const override;
    virtual double balance() const override;
    virtual bool supportBalance() const override;
    virtual bool supportFade() const override;

public Q_SLOTS:
    virtual DExpected<void> setMute(bool mute) override;
    virtual DExpected<void> setFade(double fade) override;
    virtual DExpected<void> setVolume(double volume) override;
    virtual DExpected<void> setBalance(double balance) override;

private:
    QScopedPointer<DDAemonStreamInterface> m_interface;
};

class DPlatformAudioOutputStream;
class LIBDTKAUDIOMANAGERSHARED_EXPORT DDaemonOutputStream : public DPlatformAudioOutputStream
{
    Q_OBJECT

public:
    explicit DDaemonOutputStream(const QString &path, DPlatformAudioInputDevice *parent = nullptr);
    virtual ~DDaemonOutputStream() override;

    virtual bool mute() const override;
    virtual double fade() const override;
    virtual double volume() const override;
    virtual double balance() const override;
    virtual bool supportBalance() const override;
    virtual bool supportFade() const override;

public Q_SLOTS:
    virtual DExpected<void> setMute(bool mute) override;
    virtual DExpected<void> setFade(double fade) override;
    virtual DExpected<void> setVolume(double volume) override;
    virtual DExpected<void> setBalance(double balance) override;

private:
    QScopedPointer<DDAemonStreamInterface> m_interface;
};
DAUDIOMANAGER_END_NAMESPACE

#endif
