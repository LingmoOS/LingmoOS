// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dinputdevicegeneric.h"
#include "dinputdevicegeneric_p.h"
#include "dtkinputdevices_types.h"

DDEVICE_BEGIN_NAMESPACE

using DCORE_NAMESPACE::DError;
using DCORE_NAMESPACE::DUnexpected;
DInputDeviceGenericPrivate::DInputDeviceGenericPrivate(DInputDeviceGeneric *q)
    : DObjectPrivate(q)
    , m_info(DeviceInfoInitializer)
    , m_enabled(true)
{
}

DInputDeviceGeneric::DInputDeviceGeneric(DInputDeviceGenericPrivate &dd, const DeviceInfo &info, bool enabled, QObject *parent)
    : QObject(parent)
    , DObject(dd)
{
    D_D(DInputDeviceGeneric);
    d->m_info = info;
    d->m_enabled = enabled;
}

DInputDeviceGeneric::DInputDeviceGeneric(const DeviceInfo &info, bool enabled, QObject *parent)
    : DInputDeviceGeneric(*new DInputDeviceGenericPrivate(this), info, enabled, parent)
{
}

DInputDeviceGeneric::~DInputDeviceGeneric() = default;

quint32 DInputDeviceGeneric::id() const
{
    D_DC(DInputDeviceGeneric);
    return d->m_info.id;
}

QString DInputDeviceGeneric::name() const
{
    D_DC(DInputDeviceGeneric);
    return d->m_info.name;
}

DeviceType DInputDeviceGeneric::type() const
{
    D_DC(DInputDeviceGeneric);
    return d->m_info.type;
}

bool DInputDeviceGeneric::enabled() const
{
    D_DC(DInputDeviceGeneric);
    return d->m_enabled;
}

DExpected<void> DInputDeviceGeneric::reset()
{
    return DUnexpected<>{DError{ErrorCode::InvalidCall, "Generic device does not have a reset method!"}};
}

void DInputDeviceGeneric::setDeviceInfo(const DeviceInfo &info)
{
    D_D(DInputDeviceGeneric);
    d->m_info = info;
}

void DInputDeviceGeneric::setEnabled(bool enabled)
{
    D_D(DInputDeviceGeneric);
    d->m_enabled = enabled;
    Q_EMIT this->enabledChanged(enabled);
}

void DInputDeviceGeneric::setId(quint32 id)
{
    D_D(DInputDeviceGeneric);
    d->m_info.id = id;
}

void DInputDeviceGeneric::setName(const QString &name)
{
    D_D(DInputDeviceGeneric);
    d->m_info.name = name;
}

void DInputDeviceGeneric::setType(DeviceType type)
{
    D_D(DInputDeviceGeneric);
    d->m_info.type = type;
}
DDEVICE_END_NAMESPACE
