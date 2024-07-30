/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 1999 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "guiactivateevent.h"

using namespace KParts;

class KParts::GUIActivateEventPrivate
{
public:
    GUIActivateEventPrivate(bool activated)
        : m_bActivated(activated)
    {
    }
    const bool m_bActivated;
};

const QEvent::Type GUIActivateEventType = (QEvent::Type)1970;

GUIActivateEvent::GUIActivateEvent(bool activated)
    : QEvent(GUIActivateEventType)
    , d(new GUIActivateEventPrivate(activated))
{
}

GUIActivateEvent::~GUIActivateEvent() = default;

bool GUIActivateEvent::activated() const
{
    return d->m_bActivated;
}

bool GUIActivateEvent::test(const QEvent *event)
{
    return event->type() == GUIActivateEventType;
}
