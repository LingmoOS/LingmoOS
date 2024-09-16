// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daudiostream_daemon.h"
#include "daudiodevice.h"
#include "daemonhelpers.hpp"

#include <QDBusArgument>
#include <QDebug>
#include <QDBusReply>

DAUDIOMANAGER_BEGIN_NAMESPACE
using DTK_CORE_NAMESPACE::DExpected;
using DTK_CORE_NAMESPACE::DUnexpected;
using DTK_CORE_NAMESPACE::DError;

DDAemonStreamInterface::DDAemonStreamInterface(DDBusInterface *inter, DPlatformAudioStream *owner)
    : m_inter(inter)
    , m_owner(owner)
{
    connect(this, &DDAemonStreamInterface::MuteChanged, m_owner, &DPlatformAudioStream::muteChanged);
    connect(this, &DDAemonStreamInterface::FadeChanged, m_owner, &DPlatformAudioStream::fadeChanged);
    connect(this, &DDAemonStreamInterface::VolumeChanged, m_owner, &DPlatformAudioStream::volumeChanged);
    connect(this, &DDAemonStreamInterface::BalanceChanged, m_owner, &DPlatformAudioStream::balanceChanged);

    connect(this, &DDAemonStreamInterface::SupportBalanceChanged, m_owner, &DPlatformAudioStream::supportBalanceChanged);
    connect(this, &DDAemonStreamInterface::SupportFadeChanged, m_owner, &DPlatformAudioStream::supportFadeChanged);
}

bool DDAemonStreamInterface::mute() const
{
    return qdbus_cast<bool>(m_inter->property("Mute"));
}

double DDAemonStreamInterface::fade() const
{
    return qdbus_cast<double>(m_inter->property("Fade"));
}

double DDAemonStreamInterface::volume() const
{
    return qdbus_cast<double>(m_inter->property("Volume"));
}

double DDAemonStreamInterface::balance() const
{
    return qdbus_cast<double>(m_inter->property("Balance"));
}

bool DDAemonStreamInterface::supportBalance() const
{
    return qdbus_cast<bool>(m_inter->property("SupportBalance"));
}

bool DDAemonStreamInterface::supportFade() const
{
    return qdbus_cast<bool>(m_inter->property("SupportFade"));
}

DExpected<void> DDAemonStreamInterface::setMute(bool mute)
{
    QDBusReply<void> reply = m_inter->call("SetMute", mute);
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    }
    return {};
}

DExpected<void> DDAemonStreamInterface::setFade(double fade)
{
    QDBusReply<void> reply = m_inter->call("SetFade", fade);
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    }
    return {};
}

DExpected<void> DDAemonStreamInterface::setVolume(double volume)
{
    QDBusReply<void> reply = m_inter->call("SetVolume", volume, m_owner->isPlay());
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    }
    return {};
}

DExpected<void> DDAemonStreamInterface::setBalance(double balance)
{
    QDBusReply<void> reply = m_inter->call("SetBalance", balance, m_owner->isPlay());
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    }
    return {};
}

DDaemonInputStream::DDaemonInputStream(const QString &path, DPlatformAudioOutputDevice *parent)
    : DPlatformAudioInputStream (parent)
    , m_interface(new DDAemonStreamInterface(
                      DDaemonInternal::newAudioInterface2(
                          this, path, DDaemonInternal::AudioServiceSinkInputInterface),
                      this))
{
    setName(DDaemonInternal::streamName(path));
}

DDaemonInputStream::~DDaemonInputStream()
{

}

bool DDaemonInputStream::mute() const
{
    return m_interface->mute();
}

double DDaemonInputStream::fade() const
{
    return m_interface->fade();
}

double DDaemonInputStream::volume() const
{
    return m_interface->volume();
}

double DDaemonInputStream::balance() const
{
    return m_interface->balance();
}

bool DDaemonInputStream::supportBalance() const
{
    return m_interface->supportBalance();
}

bool DDaemonInputStream::supportFade() const
{
    return m_interface->supportFade();
}

DExpected<void> DDaemonInputStream::setMute(bool mute)
{
    return m_interface->setMute(mute);
}

DExpected<void> DDaemonInputStream::setFade(double fade)
{
    return m_interface->setFade(fade);
}

DExpected<void> DDaemonInputStream::setVolume(double volume)
{
    return m_interface->setVolume(volume);
}

DExpected<void> DDaemonInputStream::setBalance(double balance)
{
    return m_interface->setBalance(balance);
}

DDaemonOutputStream::DDaemonOutputStream(const QString &path, DPlatformAudioInputDevice *parent)
    : DPlatformAudioOutputStream (parent)
    , m_interface(new DDAemonStreamInterface(
                      DDaemonInternal::newAudioInterface2(
                          this, path, DDaemonInternal::AudioServiceSourceOutputInterface),
                      this))
{
    setName(DDaemonInternal::streamName(path));
}

DDaemonOutputStream::~DDaemonOutputStream()
{

}

bool DDaemonOutputStream::mute() const
{
    return m_interface->mute();
}

double DDaemonOutputStream::fade() const
{
    return m_interface->fade();
}

double DDaemonOutputStream::volume() const
{
    return m_interface->volume();
}

double DDaemonOutputStream::balance() const
{
    return m_interface->balance();
}

bool DDaemonOutputStream::supportBalance() const
{
    return m_interface->supportBalance();
}

bool DDaemonOutputStream::supportFade() const
{
    return m_interface->supportFade();
}

DExpected<void> DDaemonOutputStream::setMute(bool mute)
{
    return m_interface->setMute(mute);
}

DExpected<void> DDaemonOutputStream::setFade(double fade)
{
    return m_interface->setFade(fade);
}

DExpected<void> DDaemonOutputStream::setVolume(double volume)
{
    return m_interface->setVolume(volume);
}

DExpected<void> DDaemonOutputStream::setBalance(double balance)
{
    return m_interface->setBalance(balance);
}

DAUDIOMANAGER_END_NAMESPACE
