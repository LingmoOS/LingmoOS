/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "domainbrowser.h"
#include <QStringList>

namespace KDNSSD
{
class DomainBrowserPrivate
{
};

DomainBrowser::DomainBrowser(DomainType, QObject *parent)
    : QObject(parent)
    , d(nullptr)
{
}

DomainBrowser::~DomainBrowser()
{
}

void DomainBrowser::startBrowse()
{
}

QStringList DomainBrowser::domains() const
{
    return QStringList();
}

bool DomainBrowser::isRunning() const
{
    return false;
}

}
#include "moc_domainbrowser.cpp"
