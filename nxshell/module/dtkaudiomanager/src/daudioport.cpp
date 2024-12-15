// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daudioport.h"
#include "daudioport_p.h"
#include "daudiocard_p.h"

#include "daudiofactory_p.h"

#include <QDebug>

DAUDIOMANAGER_BEGIN_NAMESPACE

DAudioPort::DAudioPort(DPlatformAudioPort *d)
    :d(d)
{
    Q_ASSERT(d);

    connect(d, &DPlatformAudioPort::nameChanged, this, &DAudioPort::nameChanged);
    connect(d, &DPlatformAudioPort::descriptionChanged, this, &DAudioPort::descriptionChanged);
    connect(d, &DPlatformAudioPort::enabledChanged, this, &DAudioPort::enabledChanged);
    connect(d, &DPlatformAudioPort::availableChanged, this, &DAudioPort::availableChanged);
}

DAudioPort::~DAudioPort()
{

}

void DAudioPort::setEnabled(const bool enabled)
{
    d->setEnabled(enabled);
}

bool DAudioPort::available() const
{
    return d->available();
}

bool DAudioPort::isEnabled() const
{
    return d->isEnabled();
}

int DAudioPort::direction() const
{
    return d->direction();
}

QString DAudioPort::name() const
{
    return d->name();
}

QString DAudioPort::description() const
{
    return d->description();
}

DPlatformAudioPort::DPlatformAudioPort(DPlatformAudioCard *card)
    : m_card(card)
{
    if (m_card) {
        m_card->addPort(this);
    }
}

DPlatformAudioPort::~DPlatformAudioPort()
{
}

bool DPlatformAudioPort::isEnabled() const
{
    return m_isEnabled;
}

void DPlatformAudioPort::setEnabled(const bool enabled)
{
    if (m_isEnabled == enabled)
        return;
    m_isEnabled = enabled;
    Q_EMIT enabledChanged(m_isEnabled);
}

bool DPlatformAudioPort::available() const
{
    return m_available;
}

void DPlatformAudioPort::setAvailable(const bool available)
{
    m_available = available;
}

bool DPlatformAudioPort::isActive() const
{
    return m_isActive;
}

void DPlatformAudioPort::setActive(const int active)
{
    m_isActive = active;
}

int DPlatformAudioPort::direction() const
{
    return m_direction;
}

void DPlatformAudioPort::setDirection(const int direction)
{
    m_direction = direction;
}

QString DPlatformAudioPort::name() const
{
    return m_name;
}

void DPlatformAudioPort::setName(const QString &name)
{
    if (m_name == name)
        return;
    m_name = name;
    Q_EMIT nameChanged(m_name);
}

QString DPlatformAudioPort::description() const
{
    return m_description;
}

void DPlatformAudioPort::setDescription(const QString &description)
{
    if (m_description == description)
        return;
    m_description = description;
    Q_EMIT descriptionChanged(m_description);
}

DAUDIOMANAGER_END_NAMESPACE
