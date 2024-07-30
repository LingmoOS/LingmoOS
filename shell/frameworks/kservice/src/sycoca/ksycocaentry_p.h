/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KSYCOCAENTRYPRIVATE_H
#define KSYCOCAENTRYPRIVATE_H

#include "ksycocaentry.h"

// clang-format off
#define K_SYCOCATYPE( type, baseclass ) \
    bool isType(KSycocaType t) const override { if (t == type) return true; return baseclass::isType(t);} \
    KSycocaType sycocaType() const override { return type; }
// clang-format on

class KSycocaEntryPrivate
{
public:
    explicit KSycocaEntryPrivate(const QString &path_)
        : offset(0)
        , deleted(false)
        , path(path_)
    {
    }

    KSycocaEntryPrivate(QDataStream &_str, int iOffset);

    virtual ~KSycocaEntryPrivate()
    {
    }

    // Don't forget to call the parent class
    // first if you override this function.
    virtual void save(QDataStream &s);

    virtual bool isType(KSycocaType t) const
    {
        return (t == KST_KSycocaEntry);
    }

    virtual KSycocaType sycocaType() const
    {
        return KST_KSycocaEntry;
    }

    virtual bool isValid() const
    {
        return !name().isEmpty();
    }

    virtual QString name() const = 0;

    virtual QString storageId() const
    {
        return name();
    }

    int offset;
    bool deleted;
    QString path;
};

#endif
