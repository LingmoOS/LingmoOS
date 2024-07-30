/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 1999 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "partactivateevent.h"

using namespace KParts;

class KParts::PartActivateEventPrivate
{
public:
    PartActivateEventPrivate(bool activated, Part *part, QWidget *widget)
        : m_bActivated(activated)
        , m_part(part)
        , m_widget(widget)
    {
    }
    const bool m_bActivated;
    Part *const m_part;
    QWidget *const m_widget;
};

const QEvent::Type partActivateEvent = (QEvent::Type)11769;

PartActivateEvent::PartActivateEvent(bool activated, Part *part, QWidget *widget)
    : QEvent(partActivateEvent)
    , d(new PartActivateEventPrivate(activated, part, widget))
{
}

PartActivateEvent::~PartActivateEvent() = default;

bool PartActivateEvent::activated() const
{
    return d->m_bActivated;
}

Part *PartActivateEvent::part() const
{
    return d->m_part;
}

QWidget *PartActivateEvent::widget() const
{
    return d->m_widget;
}

bool PartActivateEvent::test(const QEvent *event)
{
    return event->type() == partActivateEvent;
}
