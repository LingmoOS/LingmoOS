/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "ksycocaentry.h"
#include "ksycocaentry_p.h"
#include <QIODevice>
#include <ksycoca.h>

KSycocaEntryPrivate::KSycocaEntryPrivate(QDataStream &_str, int iOffset)
    : offset(iOffset)
    , deleted(false)
{
    _str >> path;
}

KSycocaEntry::KSycocaEntry()
    : d_ptr(nullptr)
{
}

KSycocaEntry::KSycocaEntry(KSycocaEntryPrivate &d)
    : d_ptr(&d)
{
}

KSycocaEntry::~KSycocaEntry() = default;

bool KSycocaEntry::isType(KSycocaType t) const
{
    return d_ptr->isType(t);
}

KSycocaType KSycocaEntry::sycocaType() const
{
    return d_ptr->sycocaType();
}

QString KSycocaEntry::entryPath() const
{
    Q_D(const KSycocaEntry);
    return d->path;
}

QString KSycocaEntry::storageId() const
{
    Q_D(const KSycocaEntry);
    return d->storageId();
}

bool KSycocaEntry::isDeleted() const
{
    Q_D(const KSycocaEntry);
    return d->deleted;
}

void KSycocaEntry::setDeleted(bool deleted)
{
    Q_D(KSycocaEntry);
    d->deleted = deleted;
}

bool KSycocaEntry::isSeparator() const
{
    return d_ptr == nullptr || isType(KST_KServiceSeparator);
}

int KSycocaEntry::offset() const
{
    Q_D(const KSycocaEntry);
    return d->offset;
}

void KSycocaEntryPrivate::save(QDataStream &s)
{
    offset = s.device()->pos(); // store position in member variable
    s << qint32(sycocaType()) << path;
}

bool KSycocaEntry::isValid() const
{
    Q_D(const KSycocaEntry);
    return d && d->isValid();
}

QString KSycocaEntry::name() const
{
    Q_D(const KSycocaEntry);
    return d->name();
}
