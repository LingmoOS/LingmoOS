/*
 * SPDX-FileCopyrightText: 2015 Dominik Haumann <dhaumann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "QuotaItem.h"

#include <QDebug>

QuotaItem::QuotaItem()
    : m_iconName(QStringLiteral("quota"))
    , m_mountPoint()
    , m_usage(0)
    , m_mountString()
    , m_usedString()
{
}

QString QuotaItem::iconName() const
{
    return m_iconName;
}

void QuotaItem::setIconName(const QString &name)
{
    m_iconName = name;
}

QString QuotaItem::mountPoint() const
{
    return m_mountPoint;
}

void QuotaItem::setMountPoint(const QString &mountPoint)
{
    m_mountPoint = mountPoint;
}

int QuotaItem::usage() const
{
    return m_usage;
}

void QuotaItem::setUsage(int usage)
{
    m_usage = usage;
}

QString QuotaItem::mountString() const
{
    return m_mountString;
}

void QuotaItem::setMountString(const QString &mountString)
{
    m_mountString = mountString;
}

QString QuotaItem::usedString() const
{
    return m_usedString;
}

void QuotaItem::setUsedString(const QString &usedString)
{
    m_usedString = usedString;
}

QString QuotaItem::freeString() const
{
    return m_freeString;
}

void QuotaItem::setFreeString(const QString &freeString)
{
    m_freeString = freeString;
}

bool QuotaItem::operator==(const QuotaItem &other) const
{
    // clang-format off
    return m_mountPoint == other.m_mountPoint
        && m_iconName == other.m_iconName
        && m_usage == other.m_usage
        && m_mountString == other.m_mountString
        && m_usedString == other.m_usedString
        && m_freeString == other.m_freeString;
    // clang-format on
}

bool QuotaItem::operator!=(const QuotaItem &other) const
{
    return !(*this == other);
}
