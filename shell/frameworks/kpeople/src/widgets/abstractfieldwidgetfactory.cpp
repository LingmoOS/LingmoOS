/*
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "abstractfieldwidgetfactory.h"

using namespace KPeople;

AbstractFieldWidgetFactory::AbstractFieldWidgetFactory(QObject *parent)
    : QObject(parent)
{
}

AbstractFieldWidgetFactory::~AbstractFieldWidgetFactory()
{
}

#include "moc_abstractfieldwidgetfactory.cpp"
