/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2012 Dawit Alemayehu <adawit@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "listingnotificationextension.h"

#include "readonlypart.h"

class KParts::ListingNotificationExtensionPrivate
{
};

KParts::ListingNotificationExtension::ListingNotificationExtension(KParts::ReadOnlyPart *parent)
    : QObject(parent)
    , d(nullptr)
{
}

KParts::ListingNotificationExtension::~ListingNotificationExtension()
{
}

KParts::ListingNotificationExtension *KParts::ListingNotificationExtension::childObject(QObject *obj)
{
    return obj->findChild<KParts::ListingNotificationExtension *>(QString(), Qt::FindDirectChildrenOnly);
}

KParts::ListingNotificationExtension::NotificationEventTypes KParts::ListingNotificationExtension::supportedNotificationEventTypes() const
{
    return None;
}

#include "moc_listingnotificationextension.cpp"
