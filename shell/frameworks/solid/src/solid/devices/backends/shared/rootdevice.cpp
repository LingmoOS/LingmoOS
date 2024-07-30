/*
    SPDX-FileCopyrightText: 2010 Mario Bensi <mbensi@ipsquad.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "rootdevice.h"

using namespace Solid::Backends::Shared;

RootDevice::RootDevice(const QString &udi, const QString &parentUdi)
    : Solid::Ifaces::Device()
    , m_udi(udi)
    , m_parentUdi(parentUdi)
    , m_vendor("KDE")
{
}

RootDevice::~RootDevice()
{
}

QString RootDevice::udi() const
{
    return m_udi;
}

QString RootDevice::parentUdi() const
{
    return m_parentUdi;
}

QString RootDevice::vendor() const
{
    return m_vendor;
}

void RootDevice::setVendor(const QString &vendor)
{
    m_vendor = vendor;
}

QString RootDevice::product() const
{
    return m_product;
}

void RootDevice::setProduct(const QString &product)
{
    m_product = product;
}

QString RootDevice::icon() const
{
    return m_icon;
}

void RootDevice::setIcon(const QString &icon)
{
    m_icon = icon;
}

QStringList RootDevice::emblems() const
{
    return m_emblems;
}

void RootDevice::setEmblems(const QStringList &emblems)
{
    m_emblems = emblems;
}

QString RootDevice::description() const
{
    return m_description;
}

void RootDevice::setDescription(const QString &description)
{
    m_description = description;
}

bool RootDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &) const
{
    return false;
}

QObject *RootDevice::createDeviceInterface(const Solid::DeviceInterface::Type &)
{
    return nullptr;
}

#include "moc_rootdevice.cpp"
