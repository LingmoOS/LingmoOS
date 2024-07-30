/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "metadata.h"

#include <QSharedData>

using namespace Attica;

class Q_DECL_HIDDEN Metadata::Private : public QSharedData
{
public:
    Error error;

    /// The status of the job, for example "Ok"
    QString statusString;
    /// The status as int, for easier interpretation.
    /// 100 means "Ok", for other codes refer to http://www.freedesktop.org/wiki/Specifications/open-collaboration-services
    int statusCode;

    /// An optional additional message from the server
    QString message;

    /// The number of items returned by this job (only relevant for list jobs)
    int totalItems;
    /// The number of items per page the server was asked for
    int itemsPerPage;

    QString resultingId;

    /// The http headers for the most recent network action in the case of a network error
    QList<QNetworkReply::RawHeaderPair> headers;

    Private()
        // values that make sense for single item jobs
        : error(NoError)
        , statusCode(0)
        , totalItems(1)
        , itemsPerPage(1)
    {
    }
};

Metadata::Metadata()
    : d(new Private)
{
}

Metadata::~Metadata()
{
}

Metadata::Metadata(const Attica::Metadata &other)
    : d(other.d)
{
}

Metadata &Metadata::operator=(const Attica::Metadata &other)
{
    d = other.d;
    return *this;
}

Metadata::Error Metadata::error() const
{
    return d->error;
}

void Metadata::setError(Metadata::Error error)
{
    d->error = error;
}

QString Metadata::message()
{
    return d->message;
}

void Metadata::setMessage(const QString &message)
{
    d->message = message;
}

QString Metadata::resultingId()
{
    return d->resultingId;
}

void Metadata::setResultingId(const QString &id)
{
    d->resultingId = id;
}

int Metadata::statusCode() const
{
    return d->statusCode;
}

void Metadata::setStatusCode(int code)
{
    d->statusCode = code;
}

QString Metadata::statusString() const
{
    return d->statusString;
}

void Metadata::setStatusString(const QString &status)
{
    d->statusString = status;
}

int Metadata::totalItems()
{
    return d->totalItems;
}

void Metadata::setTotalItems(int items)
{
    d->totalItems = items;
}

int Metadata::itemsPerPage()
{
    return d->itemsPerPage;
}

void Metadata::setItemsPerPage(int itemsPerPage)
{
    d->itemsPerPage = itemsPerPage;
}

QList<QNetworkReply::RawHeaderPair> Metadata::headers() const
{
    return d->headers;
}

void Metadata::setHeaders(const QList<QNetworkReply::RawHeaderPair> &headers)
{
    d->headers = headers;
}
