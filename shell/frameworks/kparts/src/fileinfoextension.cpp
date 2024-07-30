/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2010 Dawit Alemayehu <adawit@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "fileinfoextension.h"

#include "readonlypart.h"

class KParts::FileInfoExtensionPrivate
{
};

KParts::FileInfoExtension::FileInfoExtension(KParts::ReadOnlyPart *parent)
    : QObject(parent)
    , d(nullptr)

{
}

KParts::FileInfoExtension::~FileInfoExtension()
{
}

KParts::FileInfoExtension *KParts::FileInfoExtension::childObject(QObject *obj)
{
    return obj->findChild<KParts::FileInfoExtension *>(QString(), Qt::FindDirectChildrenOnly);
}

KParts::FileInfoExtension::QueryModes KParts::FileInfoExtension::supportedQueryModes() const
{
    return None;
}

bool KParts::FileInfoExtension::hasSelection() const
{
    return false;
}

#include "moc_fileinfoextension.cpp"
