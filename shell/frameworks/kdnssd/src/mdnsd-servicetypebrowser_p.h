/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MDNSD_SERVICETYPEBROWSER_P_H
#define MDNSD_SERVICETYPEBROWSER_P_H

#include <QStringList>

#include "servicebrowser.h"
#include "servicetypebrowser.h"

namespace KDNSSD
{
class ServiceTypeBrowserPrivate : public QObject
{
    Q_OBJECT
public:
    explicit ServiceTypeBrowserPrivate(ServiceTypeBrowser *parent)
        : m_parent(parent)
    {
    }
    ServiceTypeBrowser *m_parent = nullptr;
    ServiceBrowser *m_browser = nullptr;
    QStringList m_servicetypes;
public Q_SLOTS:
    void newService(KDNSSD::RemoteService::Ptr);
    void removeService(KDNSSD::RemoteService::Ptr);
};

}
#endif
