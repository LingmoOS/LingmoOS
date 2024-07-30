/*
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "abstractpersonaction.h"

using namespace KPeople;

AbstractPersonAction::AbstractPersonAction(QObject *parent)
    : QObject(parent)
{
}

#include "moc_abstractpersonaction.cpp"
