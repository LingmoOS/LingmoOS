// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daudiocard.h"
#include "daudiocard_p.h"
#include "daudioport_p.h"
#include "daudioport.h"
#include "daudiofactory_p.h"

#include <QDebug>

DAUDIOMANAGER_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DExpected;
using DCORE_NAMESPACE::DUnexpected;
using DCORE_NAMESPACE::DError;

DAudioCard::DAudioCard(DPlatformAudioCard *d)
    : d(d)
{
    Q_ASSERT(d);
    connect(d, &DPlatformAudioCard::enabledChanged, this, &DAudioCard::enabledChanged);
}

DAudioCard::~DAudioCard()
{

}

QList<DAudioPortPtr> DAudioCard::ports() const
{
    QList<DAudioPortPtr> result;
    for (auto item : d->m_ports)
        result << DAudioPortPtr(item->create());

    return result;
}

QString DAudioCard::name() const
{
    return d->name();
}

quint32 DAudioCard::id() const
{
    return d->id();
}

DAudioCard::Type DAudioCard::type() const
{
    return DAudioCard::Normal;
}

bool DAudioCard::enabled() const
{
    return d->enabled();
}

DAudioBluetoothCard::DAudioBluetoothCard(DPlatformAudioBluetoothCard *d)
    : DAudioCard (d)
{
    connect(d, &DPlatformAudioBluetoothCard::modeChanged, this, &DAudioBluetoothCard::modeChanged);
    connect(d, &DPlatformAudioBluetoothCard::modeOptionsChanged, this, &DAudioBluetoothCard::modeOptionsChanged);
}

DAudioBluetoothCard::~DAudioBluetoothCard()
{
}

QString DAudioBluetoothCard::mode() const
{
    return dynamic_cast<DPlatformAudioBluetoothCard *>(d.data())->mode();
}

QStringList DAudioBluetoothCard::modeOptions() const
{
    return dynamic_cast<DPlatformAudioBluetoothCard *>(d.data())->modeOptions();
}

DAudioCard::Type DAudioBluetoothCard::type() const
{
    return DAudioCard::Bluetooth;
}

DExpected<void> DAudioBluetoothCard::setMode(QString &mode)

{
    dynamic_cast<DPlatformAudioBluetoothCard *>(d.data())->setMode(mode);
    return {};
}

DPlatformAudioCard::DPlatformAudioCard(QObject *parent)
    : QObject (parent)
{

}

DPlatformAudioCard::~DPlatformAudioCard()
{
}

quint32 DPlatformAudioCard::id() const
{
    return m_id;
}

void DPlatformAudioCard::setId(const quint32 id)
{
    m_id = id;
}

QString DPlatformAudioCard::name() const
{
    return m_name;
}

void DPlatformAudioCard::setName(const QString &name)
{
    m_name = name;
}

bool DPlatformAudioCard::enabled() const
{
    return m_enabled;
}

void DPlatformAudioCard::setEnabled(const bool enabled)
{
    if (m_enabled == enabled)
        return;
    m_enabled = enabled;
    Q_EMIT enabledChanged(m_enabled);
}

void DPlatformAudioCard::addPort(DPlatformAudioPort *port)
{
    m_ports.append(QExplicitlySharedDataPointer(port));
}

DPlatformAudioPort *DPlatformAudioCard::portByName(const QString &portName) const
{
    auto iter = std::find_if(m_ports.begin(), m_ports.end(), [portName](auto port) {
        return port->name() == portName;
    });
    return iter != m_ports.end() ? iter->data() : nullptr;
}

QList<DPlatformAudioPort *> DPlatformAudioCard::ports() const
{
    QList<DPlatformAudioPort *> result;
    for (auto item : m_ports) {
        result << item.data();
    }
    return result;
}

DAUDIOMANAGER_END_NAMESPACE
