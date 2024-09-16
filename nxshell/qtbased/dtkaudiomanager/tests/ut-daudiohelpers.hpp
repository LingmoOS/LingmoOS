// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "gtest/gtest.h"

#include "daudioport.h"
#include "daudioport_p.h"

#include "daudiocard.h"
#include "daudiocard_p.h"

#include "daudiodevice.h"
#include "daudiodevice_p.h"

#include "daudiostream.h"
#include "daudiostream_p.h"

#include "daudiostream.h"
#include "daudiomanager_p.h"

#include <QPointer>
#include <DExpected>

DAUDIOMANAGER_USE_NAMESPACE
using DTK_CORE_NAMESPACE::DExpected;

static const QString TestAudioPortName("test port");
static const QString TestAudioPortDescription("port for testing");
class TestAudioPort : public DPlatformAudioPort
{
public:
    explicit TestAudioPort(DPlatformAudioCard *card = nullptr, const QString &name = TestAudioPortName)
        : DPlatformAudioPort(card)
    {
        setName(name);
        setDescription(TestAudioPortDescription);
    }
    inline virtual ~TestAudioPort() override;
};

TestAudioPort::~TestAudioPort() {}

static const QString TestAudioCardName("test card");
static const quint32 TestAudioCardId(0);
class TestAudioCard : public DPlatformAudioCard
{
public:
    explicit TestAudioCard(const QString &name = TestAudioCardName)
        : DPlatformAudioCard()
    {
        setName(name);
        setId(TestAudioCardId);
    }
    inline virtual ~TestAudioCard() override;
};

TestAudioCard::~TestAudioCard() {}

static const QString TestAudioInputDeviceName("test input device");
static const QString TestAudioInputDeviceDescription("input device for testing");
class TestAudioInputDevice : public DPlatformAudioInputDevice
{
public:
    explicit TestAudioInputDevice(DPlatformAudioCard *card = nullptr)
        : DPlatformAudioInputDevice (card)
    {
        setName(TestAudioInputDeviceName);
        setDescription(TestAudioInputDeviceDescription);
    }
    inline virtual ~TestAudioInputDevice() override;

    virtual bool mute() const override
    {
        return m_mute;
    }
    virtual double fade() const override
    {
        return m_fade;
    }
    virtual double volume() const override
    {
        return m_volume;
    }
    virtual double balance() const override
    {
        return m_balance;
    }
    virtual bool supportBalance() const override
    {
        return m_supportBalance;
    }
    virtual bool supportFade() const override
    {
        return m_supportFade;
    }
    virtual double baseVolume() const override
    {
        return m_baseVolume;
    }
    virtual QString activePort() const override
    {
        return m_port;
    }
    virtual void setActivePort(const QString &portName) override
    {
        if (auto port = m_card->portByName(portName)) {
            m_port = portName;
        }
    }
    virtual QList<QString> ports() const override
    {
        return m_ports;
    }

public Q_SLOTS:
    virtual DExpected<void> setMute(bool mute) override
    {
        if (m_mute == mute)
            return {};
        m_mute = mute;
        Q_EMIT muteChanged(m_mute);
        return {};
    }
    virtual DExpected<void> setFade(double fade) override
    {
        if (qFuzzyCompare(m_fade, fade))
            return {};
        m_fade = fade;
        Q_EMIT fadeChanged(m_fade);
        return {};
    }
    virtual DExpected<void> setVolume(double volume) override
    {
        m_volume = volume;
        return {};
    }
    virtual DExpected<void> setBalance(double balance) override
    {
        m_balance = balance;
        return {};
    }
    virtual double meterVolume() const override
    {
        return m_meterVolume;
    }

public:
    bool m_mute = false;
    double m_fade = 1.0;
    double m_volume = 1.0;
    double m_balance = 1.0;
    bool m_supportBalance = false;
    bool m_supportFade = false;
    double m_baseVolume = 1.0;
    double m_meterVolume = 1.0;
    QString m_port;
    QStringList m_ports;
};

TestAudioInputDevice::~TestAudioInputDevice() {}

static const QString TestAudioOutputDeviceName("test output device");
static const QString TestAudioOutputDeviceDescription("output device for testing");
class TestAudioOutputDevice : public DPlatformAudioOutputDevice
{
public:
    explicit TestAudioOutputDevice(DPlatformAudioCard *card = nullptr)
        : DPlatformAudioOutputDevice (card)
    {
        setName(TestAudioOutputDeviceName);
        setDescription(TestAudioOutputDeviceDescription);
    }
    inline virtual ~TestAudioOutputDevice() override;

    virtual bool mute() const override
    {
        return m_mute;
    }
    virtual double fade() const override
    {
        return m_fade;
    }
    virtual double volume() const override
    {
        return m_volume;
    }
    virtual double balance() const override
    {
        return m_balance;
    }
    virtual bool supportBalance() const override
    {
        return m_supportBalance;
    }
    virtual bool supportFade() const override
    {
        return m_supportFade;
    }
    virtual double baseVolume() const override
    {
        return m_baseVolume;
    }
    virtual double meterVolume() const override
    {
        return m_meterVolume;
    }
    virtual QString activePort() const override
    {
        return m_port;
    }
    virtual void setActivePort(const QString &portName) override
    {
        if (auto port = m_card->portByName(portName)) {
            m_port = portName;
        }
    }
    virtual QList<QString> ports() const override
    {
        return m_ports;
    }

public Q_SLOTS:
    virtual DExpected<void> setMute(bool mute) override
    {
        if (m_mute == mute)
            return {};
        m_mute = mute;
        Q_EMIT muteChanged(m_mute);
        return {};
    }
    virtual DExpected<void> setFade(double fade) override
    {
        if (qFuzzyCompare(m_fade, fade))
            return {};
        m_fade = fade;
        Q_EMIT fadeChanged(m_fade);
        return {};
    }
    virtual DExpected<void> setVolume(double volume) override
    {
        m_volume = volume;
        return {};
    }
    virtual DExpected<void> setBalance(double balance) override
    {
        m_balance = balance;
        return {};
    }

public:
    bool m_mute = false;
    double m_fade = 1.0;
    double m_volume = 1.0;
    double m_balance = 1.0;
    bool m_supportBalance = false;
    bool m_supportFade = false;
    double m_baseVolume = 1.0;
    double m_meterVolume = 1.0;
    QString m_port;
    QStringList m_ports;
};

TestAudioOutputDevice::~TestAudioOutputDevice() {}

static const QString TestAudioInputStreamName("test input  stream device");
static const QString TestAudioInputStreamDescription("input stream for testing");
class TestAudioInputStream: public DPlatformAudioInputStream
{
public:
    explicit TestAudioInputStream(DPlatformAudioOutputDevice *parent = nullptr)
        : DPlatformAudioInputStream(parent)
    {
    }

    inline virtual ~TestAudioInputStream() override;

    virtual bool mute() const override
    {
        return m_mute;
    }
    virtual double fade() const override
    {
        return m_fade;
    }
    virtual double volume() const override
    {
        return m_volume;
    }
    virtual double balance() const override
    {
        return m_balance;
    }
    virtual bool supportBalance() const override
    {
        return m_supportBalance;
    }
    virtual bool supportFade() const override
    {
        return m_supportFade;
    }

public Q_SLOTS:
    virtual DExpected<void> setMute(bool mute) override
    {
        if (m_mute == mute)
            return {};
        m_mute = mute;
        Q_EMIT muteChanged(m_mute);
        return {};
    }
    virtual DExpected<void> setFade(double fade) override
    {
        if (qFuzzyCompare(m_fade, fade))
            return {};
        m_fade = fade;
        Q_EMIT fadeChanged(m_fade);
        return {};
    }
    virtual DExpected<void> setVolume(double volume) override
    {
        m_volume = volume;
        return {};
    }
    virtual DExpected<void> setBalance(double balance) override
    {
        m_balance = balance;
        return {};
    }

public:
    bool m_mute = false;
    double m_fade = 1.0;
    double m_volume = 1.0;
    double m_balance = 1.0;
    bool m_supportBalance = false;
    bool m_supportFade = false;
    double m_meterVolume = 1.0;
};

TestAudioInputStream::~TestAudioInputStream() {}

static const QString TestAudioOutputStreamName("test input  stream device");
static const QString TestAudioOutputStreamDescription("input stream for testing");
class TestAudioOutputStream: public DPlatformAudioOutputStream
{
public:
    explicit TestAudioOutputStream(DPlatformAudioInputDevice *parent = nullptr)
        : DPlatformAudioOutputStream(parent)
    {
    }

    inline virtual ~TestAudioOutputStream() override;

    virtual bool mute() const override
    {
        return m_mute;
    }
    virtual double fade() const override
    {
        return m_fade;
    }
    virtual double volume() const override
    {
        return m_volume;
    }
    virtual double balance() const override
    {
        return m_balance;
    }
    virtual bool supportBalance() const override
    {
        return m_supportBalance;
    }
    virtual bool supportFade() const override
    {
        return m_supportFade;
    }

public Q_SLOTS:
    virtual DExpected<void> setMute(bool mute) override
    {
        if (m_mute == mute)
            return {};
        m_mute = mute;
        Q_EMIT muteChanged(m_mute);
        return {};
    }
    virtual DExpected<void> setFade(double fade) override
    {
        if (qFuzzyCompare(m_fade, fade))
            return {};
        m_fade = fade;
        Q_EMIT fadeChanged(m_fade);
        return {};
    }
    virtual DExpected<void> setVolume(double volume) override
    {
        m_volume = volume;
        return {};
    }
    virtual DExpected<void> setBalance(double balance) override
    {
        m_balance = balance;
        return {};
    }

public:
    bool m_mute = false;
    double m_fade = 1.0;
    double m_volume = 1.0;
    double m_balance = 1.0;
    bool m_supportBalance = false;
    bool m_supportFade = false;
    double m_meterVolume = 1.0;
};

TestAudioOutputStream::~TestAudioOutputStream() {}

class TestAudioManager : public DAudioManagerPrivate
{
public:
    inline virtual ~TestAudioManager() override;

    virtual void reset() override
    {
    }
    virtual void setReConnectionEnabled(const bool enable) override
    {
        Q_UNUSED(enable);
    }
    virtual bool increaseVolume() const override
    {
        return m_increaseVolume;
    }
    virtual bool reduceNoise() const override
    {
        return m_reduceNoise;
    }
    virtual double maxVolume() const override
    {
        return m_maxVolume;
    }

public slots:
    virtual DExpected<void> setIncreaseVolume(bool increaseVolume) override
    {
        m_increaseVolume = increaseVolume;
        return {};
    }
    virtual DExpected<void> setReduceNoise(bool reduceNoise) override
    {
        m_reduceNoise = reduceNoise;
        return {};
    }

public:
    bool m_reconnectionEnabled = false;
    bool m_increaseVolume = false;
    bool m_reduceNoise = false;
    double m_maxVolume = 1.0;
};

TestAudioManager::~TestAudioManager() {}
