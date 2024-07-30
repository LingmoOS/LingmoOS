/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 1999-2005 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KPARTS_PARTBASE_P_H
#define _KPARTS_PARTBASE_P_H

#include "partbase.h"

namespace KParts
{
class PartBasePrivate
{
public:
    Q_DECLARE_PUBLIC(PartBase)

    PartBasePrivate(PartBase *qq)
        : q_ptr(qq)
        , m_obj(nullptr)
    {
    }

    virtual ~PartBasePrivate()
    {
    }

    PartBase *q_ptr;
    QObject *m_obj;
};

} // namespace

#endif
