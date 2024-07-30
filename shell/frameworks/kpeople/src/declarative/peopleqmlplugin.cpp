/*
    Persons Model
    SPDX-FileCopyrightText: 2012 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "peopleqmlplugin.h"

#include <QQmlEngine>

#include "avatarimageprovider.h"

void PeopleQMLPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_ASSERT(uri == QByteArrayLiteral("org.kde.people"));
    engine->addImageProvider(QStringLiteral("kpeople-avatar"), new AvatarImageProvider());
}

void PeopleQMLPlugin::registerTypes(const char * /*uri*/)
{
}

#include "moc_peopleqmlplugin.cpp"
#include "peopleqmlplugin.moc"
