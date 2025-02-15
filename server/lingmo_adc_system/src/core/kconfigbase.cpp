/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
    SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
    SPDX-FileCopyrightText: 1997-1999 Matthias Kalle Dalheimer <kalle@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kconfigbase.h"

#include "kconfiggroup.h"

#include <QString>

bool KConfigBase::hasGroup(const QString &group) const
{
    return hasGroupImpl(group.toUtf8());
}

bool KConfigBase::hasGroup(const char *group) const
{
    return hasGroupImpl(QByteArray(group));
}

bool KConfigBase::hasGroup(const QByteArray &group) const
{
    return hasGroupImpl(group);
}

KConfigGroup KConfigBase::group(const QByteArray &b)
{
    return groupImpl(b);
}

KConfigGroup KConfigBase::group(const QString &str)
{
    return groupImpl(str.toUtf8());
}

KConfigGroup KConfigBase::group(const char *str)
{
    return groupImpl(QByteArray(str));
}

const KConfigGroup KConfigBase::group(const QByteArray &b) const
{
    return groupImpl(b);
}

const KConfigGroup KConfigBase::group(const QString &s) const
{
    return groupImpl(s.toUtf8());
}

const KConfigGroup KConfigBase::group(const char *s) const
{
    return groupImpl(QByteArray(s));
}

void KConfigBase::deleteGroup(const QByteArray &group, WriteConfigFlags flags)
{
    deleteGroupImpl(group, flags);
}

void KConfigBase::deleteGroup(const QString &group, WriteConfigFlags flags)
{
    deleteGroupImpl(group.toUtf8(), flags);
}

void KConfigBase::deleteGroup(const char *group, WriteConfigFlags flags)
{
    deleteGroupImpl(QByteArray(group), flags);
}

bool KConfigBase::isGroupImmutable(const QByteArray &aGroup) const
{
    return isGroupImmutableImpl(aGroup);
}

bool KConfigBase::isGroupImmutable(const QString &aGroup) const
{
    return isGroupImmutableImpl(aGroup.toUtf8());
}

bool KConfigBase::isGroupImmutable(const char *aGroup) const
{
    return isGroupImmutableImpl(QByteArray(aGroup));
}

KConfigBase::~KConfigBase()
{
}

KConfigBase::KConfigBase()
{
}

void KConfigBase::virtual_hook(int, void *)
{
}
