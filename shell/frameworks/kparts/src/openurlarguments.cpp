/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 1999-2005 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "openurlarguments.h"

class KParts::OpenUrlArgumentsPrivate : public QSharedData
{
public:
    bool reload = false;
    bool actionRequestedByUser = true;
    int xOffset = 0;
    int yOffset = 0;
    QString mimeType;
    QMap<QString, QString> metaData;
};

KParts::OpenUrlArguments::OpenUrlArguments()
    : d(new OpenUrlArgumentsPrivate)
{
}

KParts::OpenUrlArguments::OpenUrlArguments(const OpenUrlArguments &other)
    : d(other.d)
{
}

KParts::OpenUrlArguments &KParts::OpenUrlArguments::operator=(const OpenUrlArguments &other)
{
    d = other.d;
    return *this;
}

KParts::OpenUrlArguments::~OpenUrlArguments()
{
}

bool KParts::OpenUrlArguments::reload() const
{
    return d->reload;
}

void KParts::OpenUrlArguments::setReload(bool b)
{
    d->reload = b;
}

int KParts::OpenUrlArguments::xOffset() const
{
    return d->xOffset;
}

void KParts::OpenUrlArguments::setXOffset(int x)
{
    d->xOffset = x;
}

int KParts::OpenUrlArguments::yOffset() const
{
    return d->yOffset;
}

void KParts::OpenUrlArguments::setYOffset(int y)
{
    d->yOffset = y;
}

QString KParts::OpenUrlArguments::mimeType() const
{
    return d->mimeType;
}

void KParts::OpenUrlArguments::setMimeType(const QString &mime)
{
    d->mimeType = mime;
}

QMap<QString, QString> &KParts::OpenUrlArguments::metaData()
{
    return d->metaData;
}

const QMap<QString, QString> &KParts::OpenUrlArguments::metaData() const
{
    return d->metaData;
}

bool KParts::OpenUrlArguments::actionRequestedByUser() const
{
    return d->actionRequestedByUser;
}

void KParts::OpenUrlArguments::setActionRequestedByUser(bool userRequested)
{
    d->actionRequestedByUser = userRequested;
}
