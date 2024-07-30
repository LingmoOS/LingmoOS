/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MDNSD_DOMAINBROWSER_P_H
#define MDNSD_DOMAINBROWSER_P_H

#include "domainbrowser.h"
#include "mdnsd-responder.h"
#include <QStringList>

namespace KDNSSD
{
class DomainBrowserPrivate : public Responder
{
    Q_OBJECT
public:
    DomainBrowserPrivate(DomainBrowser::DomainType type, DomainBrowser *parent)
        : Responder()
        , m_type(type)
        , m_parent(parent)
    {
    }
    DomainBrowser::DomainType m_type;
    DomainBrowser *m_parent = nullptr;
    QStringList m_domains;
    virtual void customEvent(QEvent *event);
};

}
#endif
