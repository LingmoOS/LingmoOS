/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 1999-2005 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "partbase.h"
#include "partbase_p.h"

using namespace KParts;

PartBase::PartBase()
    : d_ptr(new PartBasePrivate(this))
{
}

PartBase::PartBase(PartBasePrivate &dd)
    : d_ptr(&dd)
{
}

PartBase::~PartBase() = default;

void PartBase::setPartObject(QObject *obj)
{
    Q_D(PartBase);

    d->m_obj = obj;
}

QObject *PartBase::partObject() const
{
    Q_D(const PartBase);

    return d->m_obj;
}
