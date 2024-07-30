/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "servicetypebrowser.h"
#include <QStringList>

namespace KDNSSD
{
class ServiceTypeBrowserPrivate
{
};

ServiceTypeBrowser::ServiceTypeBrowser(const QString &, QObject *parent)
    : QObject(parent)
    , d(nullptr)
{
}

ServiceTypeBrowser::~ServiceTypeBrowser()
{
}

void ServiceTypeBrowser::startBrowse()
{
}

QStringList ServiceTypeBrowser::serviceTypes() const
{
    return QStringList();
}

}
#include "moc_servicetypebrowser.cpp"
