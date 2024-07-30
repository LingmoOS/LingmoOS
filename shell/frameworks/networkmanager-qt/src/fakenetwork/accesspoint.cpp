/*
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "accesspoint.h"

AccessPoint::AccessPoint(QObject *parent)
    : QObject(parent)
    , m_flags(0)
    , m_frequency(0)
    , m_maxBitrate(0)
    , m_mode(0)
    , m_rsnFlags(0)
    , m_strength(0)
    , m_wpaFlags(0)
{
}

AccessPoint::~AccessPoint()
{
}

uint AccessPoint::flags() const
{
    return m_flags;
}

uint AccessPoint::frequency() const
{
    return m_frequency;
}

QString AccessPoint::hwAddress() const
{
    return m_hwAddress;
}

uint AccessPoint::maxBitrate() const
{
    return m_maxBitrate;
}

uint AccessPoint::mode() const
{
    return m_mode;
}

uint AccessPoint::rsnFlags() const
{
    return m_rsnFlags;
}

QByteArray AccessPoint::ssid() const
{
    return m_ssid;
}

uchar AccessPoint::strength() const
{
    return m_strength;
}

uint AccessPoint::wpaFlags() const
{
    return m_wpaFlags;
}

QString AccessPoint::accessPointPath() const
{
    return m_apPath;
}

void AccessPoint::setAccessPointPath(const QString &path)
{
    m_apPath = path;
}

void AccessPoint::setFlags(uint flags)
{
    m_flags = flags;
}

void AccessPoint::setFrequency(uint frequency)
{
    m_frequency = frequency;
}

void AccessPoint::setHwAddress(const QString &hwAddress)
{
    m_hwAddress = hwAddress;
}

void AccessPoint::setMaxBitrate(uint bitrate)
{
    m_maxBitrate = bitrate;
}

void AccessPoint::setMode(uint mode)
{
    m_mode = mode;
}

void AccessPoint::setRsnFlags(uint flags)
{
    m_rsnFlags = flags;
}

void AccessPoint::setSsid(const QByteArray &ssid)
{
    m_ssid = ssid;
}

void AccessPoint::setStrength(uchar strength)
{
    m_strength = strength;
}

void AccessPoint::setWpaFlags(uint flags)
{
    m_wpaFlags = flags;
}

#include "moc_accesspoint.cpp"
