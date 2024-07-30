/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 1999 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "openurlevent.h"

#include <QUrl>

using namespace KParts;

class KParts::OpenUrlEventPrivate
{
public:
    OpenUrlEventPrivate(ReadOnlyPart *part, const QUrl &url, const OpenUrlArguments &args)
        : m_part(part)
        , m_url(url)
        , m_args(args)
    {
    }
    ReadOnlyPart *const m_part;
    const QUrl m_url;
    const OpenUrlArguments m_args;
};

const QEvent::Type openUrlEventType = (QEvent::Type)8958;
OpenUrlEvent::OpenUrlEvent(ReadOnlyPart *part, const QUrl &url, const OpenUrlArguments &args)
    : QEvent(openUrlEventType)
    , d(new OpenUrlEventPrivate(part, url, args))
{
}

OpenUrlEvent::~OpenUrlEvent() = default;

ReadOnlyPart *OpenUrlEvent::part() const
{
    return d->m_part;
}

QUrl OpenUrlEvent::url() const
{
    return d->m_url;
}

OpenUrlArguments OpenUrlEvent::arguments() const
{
    return d->m_args;
}

bool OpenUrlEvent::test(const QEvent *event)
{
    return event->type() == openUrlEventType;
}
