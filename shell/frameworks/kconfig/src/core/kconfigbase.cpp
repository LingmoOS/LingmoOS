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
    return hasGroupImpl(group);
}

KConfigGroup KConfigBase::group(const QString &str)
{
    return groupImpl(str);
}

const KConfigGroup KConfigBase::group(const QString &s) const
{
    return groupImpl(s);
}

void KConfigBase::deleteGroup(const QString &group, WriteConfigFlags flags)
{
    deleteGroupImpl(group, flags);
}

bool KConfigBase::isGroupImmutable(const QString &aGroup) const
{
    return isGroupImmutableImpl(aGroup);
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
