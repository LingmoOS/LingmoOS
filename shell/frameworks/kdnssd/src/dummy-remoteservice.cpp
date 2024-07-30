/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004, 2005 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "remoteservice.h"
#include <QDataStream>

namespace KDNSSD
{
RemoteService::RemoteService(const QString &name, const QString &type, const QString &domain)
    : ServiceBase(name, type, domain)
{
}

RemoteService::~RemoteService()
{
}

bool RemoteService::resolve()
{
    return false;
}

void RemoteService::resolveAsync()
{
    Q_EMIT resolved(false);
}

bool RemoteService::isResolved() const
{
    return false;
}

void RemoteService::virtual_hook(int, void *)
{
    // BASE::virtual_hook(int, void*);
}

}

#include "moc_remoteservice.cpp"
