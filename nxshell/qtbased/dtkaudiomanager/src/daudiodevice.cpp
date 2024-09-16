// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daudiodevice.h"
#include "daudiostream.h"
#include "daudiocard.h"
#include "daudiodevice_p.h"

#include <QDebug>

DAUDIOMANAGER_BEGIN_NAMESPACE

DAudioDevice::DAudioDevice(DPlatformAudioDevice *d, DAudioCard *parent)
    : QObject (parent)
    , d(d)
{
    connect(d, &DPlatformAudioDevice::muteChanged, this, &DAudioDevice::muteChanged);
    connect(d, &DPlatformAudioDevice::fadeChanged, this, &DAudioDevice::fadeChanged);
    connect(d, &DPlatformAudioDevice::volumeChanged, this, &DAudioDevice::volumeChanged);
    connect(d, &DPlatformAudioDevice::balanceChanged, this, &DAudioDevice::balanceChanged);

    connect(d, &DPlatformAudioDevice::supportBalanceChanged, this, &DAudioDevice::supportBalanceChanged);
    connect(d, &DPlatformAudioDevice::supportFadeChanged, this, &DAudioDevice::supportFadeChanged);
    connect(d, &DPlatformAudioDevice::baseVolumeChanged, this, &DAudioDevice::baseVolumeChanged);

    connect(d, &DPlatformAudioDevice::meterVolumeChanged, this, &DAudioDevice::meterVolumeChanged);

    connect(d, &DPlatformAudioDevice::nameChanged, this, &DAudioDevice::nameChanged);
    connect(d, &DPlatformAudioDevice::descriptionChanged, this, &DAudioDevice::descriptionChanged);
    connect(d, &DPlatformAudioDevice::activePortChanged, this, &DAudioDevice::activePortChanged);


    connect(d, &DPlatformAudioDevice::streamAdded, this, &DAudioDevice::streamAdded);
    connect(d, &DPlatformAudioDevice::streamRemoved, this, &DAudioDevice::streamRemoved);

}

DAudioDevice::~DAudioDevice()
{
}

bool DAudioDevice::mute() const
{
    return d->mute();
}

double DAudioDevice::fade() const
{
    return d->fade();
}

double DAudioDevice::volume() const
{
    return d->volume();
}

double DAudioDevice::balance() const
{
    return d->balance();
}

bool DAudioDevice::supportBalance() const
{
    return d->supportBalance();
}

bool DAudioDevice::supportFade() const
{
    return d->supportFade();
}

double DAudioDevice::baseVolume() const
{
    return d->baseVolume();
}

double DAudioDevice::meterVolume() const
{
    return d->meterVolume();
}

QString DAudioDevice::activePort() const
{
    return d->activePort();
}

void DAudioDevice::setActivePort(const QString &portName) const
{
    return d->setActivePort(portName);
}

QList<DAudioPortPtr> DAudioDevice::ports() const
{
    QList<DAudioPortPtr> result;
    for (auto item : d->ports()) {
        if (auto port = d->m_card->portByName(item)) {
            result << DAudioPortPtr(port->create());
        }
    }
    return result;
}

QString DAudioDevice::name() const
{
    return d->name();
}

QString DAudioDevice::description() const
{
    return d->description();
}

quint32 DAudioDevice::card() const
{
    return d->card();
}

QList<DAudioStreamPtr> DAudioDevice::streams() const
{
    QList<DAudioStreamPtr> result;
    for (auto item : d->m_streams) {
        result << DAudioStreamPtr(item->create());
    }
    return result;
}

DAudioStreamPtr DAudioDevice::stream(const QString &streamName) const
{
    QList<DAudioStreamPtr> result;
    for (auto item : d->m_streams) {
        if (item->name() == streamName)
            return DAudioStreamPtr(item->create());
    }
    return nullptr;
}

bool DAudioDevice::isPlay() const
{
    return d->isPlay();
}

DExpected<void> DAudioDevice::setMute(bool mute)
{
    d->setMute(mute);
    return {};
}

DExpected<void> DAudioDevice::setFade(double fade)
{
    d->setFade(fade);
    return {};
}

DExpected<void> DAudioDevice::setVolume(double volume)
{
    d->setVolume(volume);
    return {};
}

DExpected<void> DAudioDevice::setBalance(double balance)
{
    d->setBalance(balance);
    return {};
}

void DAudioDevice::setIsPlay(bool isPlay)
{
    if (d->isPlay() == isPlay)
        return;
    d->setIsPlay(isPlay);
    Q_EMIT isPlayChanged(isPlay);
}

DAudioInputDevice::DAudioInputDevice(DPlatformAudioInputDevice *d, DAudioCard *parent)
    : DAudioDevice(d, parent)
{
    Q_ASSERT(d);
}

DAudioOutputDevice::DAudioOutputDevice(DPlatformAudioOutputDevice *d, DAudioCard *parent)
    : DAudioDevice(d, parent)
{
}

DPlatformAudioInputDevice::DPlatformAudioInputDevice(DPlatformAudioCard *card)
    : DPlatformAudioDevice(card)
{
}

DPlatformAudioInputDevice::~DPlatformAudioInputDevice()
{
}

DPlatformAudioDevice::DPlatformAudioDevice(DPlatformAudioCard *card)
    : m_card(card)
{

}

DPlatformAudioDevice::~DPlatformAudioDevice()
{
}

bool DPlatformAudioDevice::isPlay() const
{
    return m_isPlay;
}

void DPlatformAudioDevice::setIsPlay(bool isPlay)
{
    m_isPlay = isPlay;
}

bool DPlatformAudioDevice::isDefault() const
{
    return m_isDefault;
}

void DPlatformAudioDevice::setDefault(bool isDefault)
{
    m_isDefault = isDefault;
}

quint32 DPlatformAudioDevice::card() const
{
    if (m_card) {
        m_card->id();
    }
    return 0;
}

void DPlatformAudioDevice::addStream(DPlatformAudioStream *stream)
{
    m_streams.append(QExplicitlySharedDataPointer(stream));
    Q_EMIT streamAdded(stream->name());
}

void DPlatformAudioDevice::removeStream(const QString &streamName)
{
    bool hasChanged = false;
    for (auto item : m_streams) {
        if (item->name() == streamName) {
            m_streams.removeOne(item);
            break;
        }
    }
    if (hasChanged) {
        Q_EMIT streamRemoved(streamName);
    }
}

DPlatformAudioStream *DPlatformAudioDevice::stream(const QString &streamName)
{
    for (auto item : m_streams) {
        if (item->name() == streamName)
            return item.data();
    }
    return nullptr;
}

QString DPlatformAudioDevice::name() const
{
    return m_name;
}

void DPlatformAudioDevice::setName(const QString &name)
{
    if (name == m_name)
        return;
    m_name = name;
    Q_EMIT nameChanged(m_name);
}

QString DPlatformAudioDevice::description() const
{
    return m_description;
}

void DPlatformAudioDevice::setDescription(const QString &description)
{
    if (description == m_description)
        return;
    m_description = description;
    Q_EMIT descriptionChanged(m_description);
}

DPlatformAudioOutputDevice::DPlatformAudioOutputDevice(DPlatformAudioCard *card)
    : DPlatformAudioDevice(card)
{
}

DPlatformAudioOutputDevice::~DPlatformAudioOutputDevice()
{
}
DAUDIOMANAGER_END_NAMESPACE
