/*
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kglobalshortcutinfo.h"
#include "kglobalshortcutinfo_p.h"

KGlobalShortcutInfo::KGlobalShortcutInfo()
    : d(new KGlobalShortcutInfoPrivate)
{
}

KGlobalShortcutInfo::KGlobalShortcutInfo(const KGlobalShortcutInfo &rhs)
    : QObject()
    , d(new KGlobalShortcutInfoPrivate)
{
    d->contextUniqueName = rhs.d->contextUniqueName;
    d->contextFriendlyName = rhs.d->contextFriendlyName;
    d->componentFriendlyName = rhs.d->componentFriendlyName;
    d->componentUniqueName = rhs.d->componentUniqueName;
    d->friendlyName = rhs.d->friendlyName;
    d->uniqueName = rhs.d->uniqueName;
    d->keys = rhs.d->keys;
    d->defaultKeys = rhs.d->defaultKeys;
}

KGlobalShortcutInfo::~KGlobalShortcutInfo()
{
    delete d;
}

KGlobalShortcutInfo &KGlobalShortcutInfo::operator=(const KGlobalShortcutInfo &rhs)
{
    KGlobalShortcutInfo tmp(rhs);
    KGlobalShortcutInfoPrivate *swap;
    swap = d;
    d = tmp.d;
    tmp.d = swap;
    return *this;
}

QString KGlobalShortcutInfo::contextFriendlyName() const
{
    return d->contextFriendlyName.isEmpty() ? d->contextUniqueName : d->contextFriendlyName;
}

QString KGlobalShortcutInfo::contextUniqueName() const
{
    return d->contextUniqueName;
}

QString KGlobalShortcutInfo::componentFriendlyName() const
{
    return d->componentFriendlyName.isEmpty() ? d->componentUniqueName : d->componentFriendlyName;
}

QString KGlobalShortcutInfo::componentUniqueName() const
{
    return d->componentUniqueName;
}

QList<QKeySequence> KGlobalShortcutInfo::defaultKeys() const
{
    return d->defaultKeys;
}

QString KGlobalShortcutInfo::friendlyName() const
{
    return d->friendlyName;
}

QList<QKeySequence> KGlobalShortcutInfo::keys() const
{
    return d->keys;
}

QString KGlobalShortcutInfo::uniqueName() const
{
    return d->uniqueName;
}

#include "moc_kglobalshortcutinfo.cpp"
