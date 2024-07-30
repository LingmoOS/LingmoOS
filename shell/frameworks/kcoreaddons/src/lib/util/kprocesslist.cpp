/*
    This file is part of the KDE Frameworks

    SPDX-FileCopyrightText: 2011 Nokia Corporation and/or its subsidiary(-ies).
    SPDX-FileCopyrightText: 2019 David Hallas <david@davidhallas.dk>

    SPDX-License-Identifier: LGPL-2.1-only WITH Qt-LGPL-exception-1.1 OR LicenseRef-Qt-Commercial
*/

#include "kprocesslist.h"
#include "kprocesslist_p.h"

using namespace KProcessList;

KProcessInfoPrivate::KProcessInfoPrivate()
{
}

KProcessInfo::KProcessInfo()
    : d_ptr(new KProcessInfoPrivate)
{
}

KProcessInfo::KProcessInfo(qint64 pid, const QString &command, const QString &user)
    : KProcessInfo(pid, command, command, user)
{
}

KProcessInfo::KProcessInfo(qint64 pid, const QString &command, const QString &name, const QString &user)
    : d_ptr(new KProcessInfoPrivate)
{
    d_ptr->valid = true;
    d_ptr->pid = pid;
    d_ptr->name = name;
    d_ptr->command = command;
    d_ptr->user = user;
}

KProcessInfo::KProcessInfo(const KProcessInfo &other)
    : d_ptr(new KProcessInfoPrivate)
{
    *this = other;
}

KProcessInfo::~KProcessInfo()
{
}

KProcessInfo &KProcessInfo::operator=(const KProcessInfo &other)
{
    d_ptr = other.d_ptr;
    return *this;
}

bool KProcessInfo::isValid() const
{
    return d_ptr->valid;
}

qint64 KProcessInfo::pid() const
{
    return d_ptr->pid;
}

QString KProcessInfo::name() const
{
    return d_ptr->name;
}

QString KProcessInfo::command() const
{
    return d_ptr->command;
}

QString KProcessInfo::user() const
{
    return d_ptr->user;
}
