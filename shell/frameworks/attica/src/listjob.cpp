/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2009 Eckhart Wörner <ewoerner@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "listjob.h"

#include <QXmlStreamReader>
#include <attica_debug.h>

using namespace Attica;

template<class T>
ListJob<T>::ListJob(PlatformDependent *internals, const QNetworkRequest &request)
    : GetJob(internals, request)
{
    qCDebug(ATTICA) << "creating list job:" << request.url();
}

template<class T>
typename T::List ListJob<T>::itemList() const
{
    return m_itemList;
}

template<class T>
void ListJob<T>::parse(const QString &xml)
{
    typename T::Parser parser;
    m_itemList = parser.parseList(xml);
    setMetadata(parser.metadata());
    qCDebug(ATTICA) << "received categories:" << m_itemList.size();
}
