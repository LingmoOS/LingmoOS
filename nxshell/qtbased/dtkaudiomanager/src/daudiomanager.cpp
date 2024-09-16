// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later


#include "daudiomanager.h"
#include "daudiomanager_p.h"
#include "daudiofactory_p.h"

#include <QDebug>
#include <QLoggingCategory>

DAUDIOMANAGER_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DExpected;
using DCORE_NAMESPACE::DUnexpected;
using DCORE_NAMESPACE::DError;

// daudiomanager log category
#ifndef QT_DEBUG
Q_LOGGING_CATEGORY(amLog, "dtk.audiomanager", QtInfoMsg)
#else
Q_LOGGING_CATEGORY(amLog, "dtk.audiomanager")
#endif

DAudioManager::DAudioManager(QObject *parent)
    : DAudioManager(QString(), parent)
{
}

DAudioManager::DAudioManager(const QString &backendType, QObject *parent)
    : DAudioManager(DAudioFactory::createAudioManager(backendType), parent)
{

}

DAudioManager::DAudioManager(DAudioManagerPrivate *d, QObject *parent)
    : QObject(parent)
    , d(d)
{
    Q_ASSERT(this->d);

    connect(this->d.data(), &DAudioManagerPrivate::deviceAdded, this, &DAudioManager::deviceAdded);
    connect(this->d.data(), &DAudioManagerPrivate::deviceRemoved, this, &DAudioManager::deviceRemoved);
    connect(this->d.data(), &DAudioManagerPrivate::cardsChanged, this, &DAudioManager::cardsChanged);
    connect(this->d.data(), &DAudioManagerPrivate::defaultInputDeviceChanged, this, &DAudioManager::defaultInputDevice);
    connect(this->d.data(), &DAudioManagerPrivate::defaultOutputDeviceChanged, this, &DAudioManager::defaultOutputDeviceChanged);
    connect(this->d.data(), &DAudioManagerPrivate::increaseVolumeChanged, this, &DAudioManager::increaseVolumeChanged);
    connect(this->d.data(), &DAudioManagerPrivate::reduceNoiseChanged, this, &DAudioManager::reduceNoiseChanged);
    connect(this->d.data(), &DAudioManagerPrivate::maxVolumeChanged, this, &DAudioManager::maxVolumeChanged);
}

DAudioManager::~DAudioManager()
{
    d->m_cards.clear();

    d->m_inputDevices.clear();

    d->m_outputDevices.clear();
}

QList<DAudioCardPtr> DAudioManager::cards() const
{
    QList<DAudioCardPtr> result;
    for (auto item : d->m_cards) {
        result << DAudioCardPtr(item->create());
    }
    return result;
}

DAudioCardPtr DAudioManager::card(const quint32 cardId) const
{
    if (auto card = d->cardById(cardId))
        return DAudioCardPtr(card->create());

    return nullptr;
}

QList<DAudioCardPtr> DAudioManager::availableCards() const
{
    QList<DAudioCardPtr> result;
    for (auto item : d->m_cards) {
        if (item->enabled()) {
            result << DAudioCardPtr(item->create());
        }
    }
    return result;
}

QList<DAudioInputDevicePtr> DAudioManager::inputDevices() const
{
    QList<DAudioInputDevicePtr> result;
    for (auto item : d->m_inputDevices) {
        result << DAudioInputDevicePtr(dynamic_cast<DAudioInputDevice *>(item->create()));
    }
    return result;
}

QList<DAudioOutputDevicePtr> DAudioManager::outputDevices() const
{
    QList<DAudioOutputDevicePtr> result;
    for (auto item : d->m_outputDevices) {
        result << DAudioOutputDevicePtr(dynamic_cast<DAudioOutputDevice *>(item->create()));
    }
    return result;
}

DAudioInputDevicePtr DAudioManager::defaultInputDevice() const
{
    for (auto item : d->m_inputDevices) {
        if (item->isDefault())
            return DAudioInputDevicePtr(dynamic_cast<DAudioInputDevice *>(item->create()));
    }
    return nullptr;
}

DAudioOutputDevicePtr DAudioManager::defaultOutputDevice() const
{
    for (auto item : d->m_outputDevices) {
        if (item->isDefault())
            return DAudioOutputDevicePtr(dynamic_cast<DAudioOutputDevice *>(item->create()));
    }
    return nullptr;
}

DAudioInputDevicePtr DAudioManager::inputDevice(const QString &deviceName) const
{
    for (auto item : d->m_inputDevices) {
        if (item->name() == deviceName)
            return DAudioInputDevicePtr(dynamic_cast<DAudioInputDevice *>(item->create()));
    }
    return nullptr;
}

DAudioOutputDevicePtr DAudioManager::outputDevice(const QString &deviceName) const
{
    for (auto item : d->m_outputDevices) {
        if (item->name() == deviceName)
            return DAudioOutputDevicePtr(dynamic_cast<DAudioOutputDevice *>(item->create()));
    }
    return nullptr;
}

void DAudioManager::reset()
{
    d->reset();
}

void DAudioManager::setReConnectionEnabled(const bool enable)
{
    d->setReConnectionEnabled(enable);
}

void DAudioManager::setPort(const quint32 cardId, const QString &portName)
{
    if (auto card = d->cardById(cardId)) {
        if (auto port = card->portByName(portName)) {
            port->setActive(true);
        }
    }
}

void DAudioManager::setPortEnabled(const quint32 cardId, const QString &portName, bool enabled)
{
    if (auto card = d->cardById(cardId)) {
        if (auto port = card->portByName(portName)) {
            port->setEnabled(enabled);
        }
    }
}

bool DAudioManager::increaseVolume() const
{
    return d->increaseVolume();
}

bool DAudioManager::reduceNoise() const
{
    return d->reduceNoise();
}

double DAudioManager::maxVolume() const
{
    return d->maxVolume();
}

DExpected<void> DAudioManager::setIncreaseVolume(bool increaseVolume)
{
    return d->setIncreaseVolume(increaseVolume);
}

DExpected<void> DAudioManager::setReduceNoise(bool reduceNoise)
{
    d->setReduceNoise(reduceNoise);
    return {};
}

DAudioManagerPrivate::DAudioManagerPrivate(QObject *parent)
    : QObject (parent)
{

}

DAudioManagerPrivate::~DAudioManagerPrivate()
{

}

void DAudioManagerPrivate::addCard(DPlatformAudioCard *card)
{
    m_cards.append(QExplicitlySharedDataPointer(card));
    Q_EMIT cardsChanged();
}

void DAudioManagerPrivate::removeCard(const quint32 cardId)
{
    for (auto item : m_cards) {
        if (item->id() == cardId) {
            m_cards.removeOne(item);
            Q_EMIT cardsChanged();
            break;
        }
    }
}

DPlatformAudioCard *DAudioManagerPrivate::cardById(const quint32 cardId) const
{
    auto iter = std::find_if(m_cards.begin(), m_cards.end(), [cardId](auto card) {
        return card->id() == cardId;
    });
    return iter != m_cards.end() ? iter->data() : nullptr;
}

void DAudioManagerPrivate::addInputDevice(DPlatformAudioInputDevice *device)
{
    m_inputDevices.append(QExplicitlySharedDataPointer(device));
    Q_EMIT deviceAdded(device->name(), true);
}

void DAudioManagerPrivate::removeInputDevice(const QString &deviceName)
{
    for (auto item : m_inputDevices) {
        if (item->name() == deviceName) {
            m_inputDevices.removeOne(item);
            Q_EMIT deviceRemoved(deviceName, true);
            break;
        }
    }
}

void DAudioManagerPrivate::addOutputDevice(DPlatformAudioOutputDevice *device)
{
    m_outputDevices.append(QExplicitlySharedDataPointer(device));
    Q_EMIT deviceAdded(device->name(), false);
}

void DAudioManagerPrivate::removeOutputDevice(const QString &deviceName)
{
    for (auto item : m_outputDevices) {
        if (item->name() == deviceName) {
            m_outputDevices.removeOne(item);
            Q_EMIT deviceRemoved(deviceName, false);
            break;
        }
    }
}
DAUDIOMANAGER_END_NAMESPACE
