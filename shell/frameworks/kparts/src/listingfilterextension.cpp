/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2012 Dawit Alemayehu <adawit@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "listingfilterextension.h"

#include "readonlypart.h"

class KParts::ListingFilterExtensionPrivate
{
};

KParts::ListingFilterExtension::ListingFilterExtension(KParts::ReadOnlyPart *parent)
    : QObject(parent)
    , d(nullptr)
{
}

KParts::ListingFilterExtension::~ListingFilterExtension()
{
}

KParts::ListingFilterExtension *KParts::ListingFilterExtension::childObject(QObject *obj)
{
    return obj->findChild<KParts::ListingFilterExtension *>(QString(), Qt::FindDirectChildrenOnly);
}

KParts::ListingFilterExtension::FilterModes KParts::ListingFilterExtension::supportedFilterModes() const
{
    return None;
}

bool KParts::ListingFilterExtension::supportsMultipleFilters(KParts::ListingFilterExtension::FilterMode) const
{
    return false;
}

#include "moc_listingfilterextension.cpp"
