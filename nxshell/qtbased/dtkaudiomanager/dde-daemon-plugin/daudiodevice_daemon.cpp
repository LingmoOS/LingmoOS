// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daudiodevice_daemon.h"
#include "daudiostream.h"
#include "daudiocard.h"
#include "daemonhelpers.hpp"

#include <QDBusAbstractInterface>
#include <QDBusArgument>
#include <QDBusReply>
#include <QDebug>
#include <QDBusMetaType>

DAUDIOMANAGER_BEGIN_NAMESPACE
using DTK_CORE_NAMESPACE::DExpected;
using DTK_CORE_NAMESPACE::DUnexpected;
using DTK_CORE_NAMESPACE::DError;

static void registDBusStructInfo()
{
    qRegisterMetaType<DAudioPortInfo_p>("DAudioPortInfo_p");
    qDBusRegisterMetaType<DAudioPortInfo_p>();
    qDBusRegisterMetaType<QList<DAudioPortInfo_p>>();
}
Q_CONSTRUCTOR_FUNCTION(registDBusStructInfo)

const QDBusArgument &operator>>(const QDBusArgument &arg, DAudioPortInfo_p &port)
{
    arg.beginStructure();
    arg >> port.name;
    arg >> port.description;
    arg >> port.available;
    arg.endStructure();
    return arg;
}

QDBusArgument &operator<<(QDBusArgument &arg, const DAudioPortInfo_p &port)
{
    arg.beginStructure();
    arg << port.name;
    arg << port.description;
    arg << port.available;
    arg.endStructure();
    return arg;
}

DDAemonDeviceInterface::DDAemonDeviceInterface(DDBusInterface *inter, DPlatformAudioDevice *owner)
    : m_inter(inter)
    , m_owner(owner)
{
    connect(this, &DDAemonDeviceInterface::MuteChanged, m_owner, &DPlatformAudioDevice::muteChanged);
    connect(this, &DDAemonDeviceInterface::FadeChanged, m_owner, &DPlatformAudioDevice::fadeChanged);
    connect(this, &DDAemonDeviceInterface::VolumeChanged, m_owner, &DPlatformAudioDevice::volumeChanged);
    connect(this, &DDAemonDeviceInterface::BalanceChanged, m_owner, &DPlatformAudioDevice::balanceChanged);
    connect(this, &DDAemonDeviceInterface::ActivePortChanged, m_owner, [this](DAudioPortInfo_p port) {
        Q_EMIT m_owner->activePortChanged(port.name);
    });


    connect(this, &DDAemonDeviceInterface::SupportBalanceChanged, m_owner, &DPlatformAudioDevice::supportBalanceChanged);
    connect(this, &DDAemonDeviceInterface::SupportFadeChanged, m_owner, &DPlatformAudioDevice::supportFadeChanged);
    connect(this, &DDAemonDeviceInterface::BaseVolumeChanged, m_owner, &DPlatformAudioDevice::baseVolumeChanged);
}

bool DDAemonDeviceInterface::mute() const
{
    return qdbus_cast<bool>(m_inter->property("Mute"));
}

double DDAemonDeviceInterface::fade() const
{
    return qdbus_cast<double>(m_inter->property("Fade"));
}

double DDAemonDeviceInterface::volume() const
{
    return qdbus_cast<double>(m_inter->property("Volume"));
}

double DDAemonDeviceInterface::balance() const
{
    return qdbus_cast<double>(m_inter->property("Balance"));
}

bool DDAemonDeviceInterface::supportBalance() const
{
    return qdbus_cast<bool>(m_inter->property("SupportBalance"));
}

bool DDAemonDeviceInterface::supportFade() const
{
    return qdbus_cast<bool>(m_inter->property("SupportFade"));
}

double DDAemonDeviceInterface::baseVolume() const
{
    return qdbus_cast<bool>(m_inter->property("BaseVolume"));
}

QString DDAemonDeviceInterface::activePort() const
{
    auto activePort = qdbus_cast<DAudioPortInfo_p>(m_inter->property("ActivePort"));
    return activePort.name;
}

void DDAemonDeviceInterface::setActivePort(const QString &portName)
{
    m_inter->call("setActivePort", portName);
}

QList<QString> DDAemonDeviceInterface::ports() const
{
    QList<QString> result;
    auto ports = qdbus_cast<QList<DAudioPortInfo_p>>(m_inter->property("Ports"));
    for (auto item : ports) {
        result << item.name;
    }
    return result;
}

DExpected<void> DDAemonDeviceInterface::setMute(bool mute)
{
    QDBusReply<void> reply = m_inter->call("SetMute", mute);
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    }
    return {};
}

DExpected<void> DDAemonDeviceInterface::setFade(double fade)
{
    QDBusReply<void> reply = m_inter->call("SetFade", fade);
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    }
    return {};
}

DExpected<void> DDAemonDeviceInterface::setVolume(double volume)
{
    QDBusReply<void> reply = m_inter->call("SetVolume", volume, m_owner->isPlay());
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    }
    return {};
}

DExpected<void> DDAemonDeviceInterface::setBalance(double balance)
{
    QDBusReply<void> reply = m_inter->call("SetBalance", balance, m_owner->isPlay());
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    }
    return {};
}

DDaemonAudioInputDevice::DDaemonAudioInputDevice(const QString &path, DPlatformAudioCard *card)
    : DPlatformAudioInputDevice (card)
    , m_interface(new DDAemonDeviceInterface(
                      DDaemonInternal::newAudioInterface2(
                          this, path, DDaemonInternal::AudioServiceSourceInterface),
                      this))
{
    setName(DDaemonInternal::deviceName(path));
}

DDaemonAudioInputDevice::~DDaemonAudioInputDevice()
{
    m_streams.clear();
}

bool DDaemonAudioInputDevice::mute() const
{
    return m_interface->mute();
}

double DDaemonAudioInputDevice::fade() const
{
    return m_interface->fade();
}

double DDaemonAudioInputDevice::volume() const
{
    return m_interface->volume();
}

double DDaemonAudioInputDevice::balance() const
{
    return m_interface->balance();
}

bool DDaemonAudioInputDevice::supportBalance() const
{
    return m_interface->supportBalance();
}

bool DDaemonAudioInputDevice::supportFade() const
{
    return m_interface->supportFade();
}

double DDaemonAudioInputDevice::baseVolume() const
{
    return m_interface->baseVolume();
}

double DDaemonAudioInputDevice::meterVolume() const
{
    const_cast<DDaemonAudioInputDevice *>(this)->ensureMeter();

    return m_meterInter ? qdbus_cast<double>(m_meterInter->property("Volume")) : 0.0;
}

QString DDaemonAudioInputDevice::activePort() const
{
    return m_interface->activePort();
}

void DDaemonAudioInputDevice::setActivePort(const QString &portName)
{
    if (m_card->portByName(portName)) {
        m_interface->setActivePort(portName);
    }
}

QList<QString> DDaemonAudioInputDevice::ports() const
{
    return m_interface->ports();
}

void DDaemonAudioInputDevice::compareAndDestroyStreams(const QList<QString> &nowStreams)
{
    auto tmp = m_streams;
    for (auto item : tmp) {
        if (nowStreams.contains(item->name()))
            continue;
        removeStream(item->name());
    }
}

DExpected<void> DDaemonAudioInputDevice::setMute(bool mute)
{
    return m_interface->setMute(mute);
}

DExpected<void> DDaemonAudioInputDevice::setFade(double fade)
{
    return m_interface->setFade(fade);
}

DExpected<void> DDaemonAudioInputDevice::setVolume(double volume)
{
    return m_interface->setVolume(volume);
}

DExpected<void> DDaemonAudioInputDevice::setBalance(double balance)
{
    return m_interface->setBalance(balance);
}

void DDaemonAudioInputDevice::ensureMeter()
{
    if (m_meterInter && !m_meterInter->isValid()) {
        m_meterInter.reset();
    }
    if (!m_meterInter) {

        QDBusReply<QDBusObjectPath> reply = m_interface->m_inter->call("GetMeter");
        if (!reply.isValid()) {
            qWarning() << "Can't get Meter" << reply.error();
            return;
        }
        const auto path = reply.value().path();
        auto inter = DDaemonInternal::newAudioInterface(path, DDaemonInternal::AudioServiceMeterInterface);
        if (!inter->isValid()) {
            qWarning() << "Error:" << inter->lastError();
            inter->deleteLater();
            return;
        }
        m_meterInter.reset(inter);
    }
    m_meterInter->call("Tick");
}

DDaemonAudioOutputDevice::DDaemonAudioOutputDevice(const QString &path, DPlatformAudioCard *parent)
    : DPlatformAudioOutputDevice (parent)
    , m_interface(new DDAemonDeviceInterface(
                      DDaemonInternal::newAudioInterface2(
                          this, path, DDaemonInternal::AudioServiceSinkInterface),
                      this))
{
    setName(DDaemonInternal::deviceName(path));
}

DDaemonAudioOutputDevice::~DDaemonAudioOutputDevice()
{
}

bool DDaemonAudioOutputDevice::mute() const
{
    return m_interface->mute();
}

double DDaemonAudioOutputDevice::fade() const
{
    return m_interface->fade();
}

double DDaemonAudioOutputDevice::volume() const
{
    return m_interface->volume();
}

double DDaemonAudioOutputDevice::balance() const
{
    return m_interface->balance();
}

bool DDaemonAudioOutputDevice::supportBalance() const
{
    return m_interface->supportBalance();
}

bool DDaemonAudioOutputDevice::supportFade() const
{
    return m_interface->supportFade();
}

double DDaemonAudioOutputDevice::baseVolume() const
{
    return m_interface->baseVolume();
}

double DDaemonAudioOutputDevice::meterVolume() const
{
    return 0.0;
}

QString DDaemonAudioOutputDevice::activePort() const
{
    return m_interface->activePort();
}

void DDaemonAudioOutputDevice::setActivePort(const QString &portName)
{
    if (m_card->portByName(portName)) {
        m_interface->setActivePort(portName);
    }
}

QList<QString> DDaemonAudioOutputDevice::ports() const
{
    return m_interface->ports();
}

void DDaemonAudioOutputDevice::compareAndDestroyStreams(const QList<QString> &nowStreams)
{
    auto tmp = m_streams;
    for (auto item : tmp) {
        if (nowStreams.contains(item->name()))
            continue;
        removeStream(item->name());
    }
}

DExpected<void> DDaemonAudioOutputDevice::setMute(bool mute)
{
    return m_interface->setMute(mute);
}

DExpected<void> DDaemonAudioOutputDevice::setFade(double fade)
{
    return m_interface->setFade(fade);
}

DExpected<void> DDaemonAudioOutputDevice::setVolume(double volume)
{
    return m_interface->setVolume(volume);
}

DExpected<void> DDaemonAudioOutputDevice::setBalance(double balance)
{
    return m_interface->setBalance(balance);
}
DAUDIOMANAGER_END_NAMESPACE

