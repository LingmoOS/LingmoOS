/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 1999 David Faure <faure@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef __kparts_openurlevent_h__
#define __kparts_openurlevent_h__

#include <QEvent>
#include <kparts/openurlarguments.h>
#include <memory>

class QUrl;

namespace KParts
{
class ReadOnlyPart;
class OpenUrlEventPrivate;

/**
 * @class OpenUrlEvent openurlevent.h <KParts/OpenUrlEvent>
 *
 * @short The KParts::OpenUrlEvent event informs that a given part has opened a given URL.
 * Applications can use this event to send this information to interested plugins.
 *
 * The event should be sent before opening the URL in the part, so that the plugins
 * can use part()->url() to get the old URL.
 */
class KPARTS_EXPORT OpenUrlEvent : public QEvent
{
public:
    OpenUrlEvent(ReadOnlyPart *part, const QUrl &url, const OpenUrlArguments &args = OpenUrlArguments());
    ~OpenUrlEvent() override;

    ReadOnlyPart *part() const;
    QUrl url() const;
    OpenUrlArguments arguments() const;

    static bool test(const QEvent *event);

private:
    const std::unique_ptr<OpenUrlEventPrivate> d;
};

}

#endif
