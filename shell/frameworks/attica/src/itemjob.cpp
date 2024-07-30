/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>
    SPDX-FileCopyrightText: 2011 Laszlo Papp <djszapi@archlinux.us>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "itemjob.h"

using namespace Attica;

template<class T>
ItemJob<T>::ItemJob(PlatformDependent *internals, const QNetworkRequest &request)
    : GetJob(internals, request)
{
}

template<class T>
void ItemJob<T>::parse(const QString &xml)
{
    typename T::Parser p;
    m_item = p.parse(xml);
    setMetadata(p.metadata());
}

template<class T>
T ItemJob<T>::result() const
{
    return m_item;
}

template<class T>
ItemDeleteJob<T>::ItemDeleteJob(PlatformDependent *internals, const QNetworkRequest &request)
    : DeleteJob(internals, request)
{
}

template<class T>
void ItemDeleteJob<T>::parse(const QString &xml)
{
    typename T::Parser p;
    m_item = p.parse(xml);
    setMetadata(p.metadata());
}

template<class T>
T ItemDeleteJob<T>::result() const
{
    return m_item;
}

template<class T>
ItemPostJob<T>::ItemPostJob(PlatformDependent *internals, const QNetworkRequest &request, QIODevice *data)
    : PostJob(internals, request, data)
{
}

template<class T>
ItemPostJob<T>::ItemPostJob(PlatformDependent *internals, const QNetworkRequest &request, const StringMap &parameters)
    : PostJob(internals, request, parameters)
{
}

template<class T>
void ItemPostJob<T>::parse(const QString &xml)
{
    typename T::Parser p;
    m_item = p.parse(xml);
    setMetadata(p.metadata());
}

template<class T>
T ItemPostJob<T>::result() const
{
    return m_item;
}

template<class T>
ItemPutJob<T>::ItemPutJob(PlatformDependent *internals, const QNetworkRequest &request, QIODevice *data)
    : PutJob(internals, request, data)
{
}

template<class T>
ItemPutJob<T>::ItemPutJob(PlatformDependent *internals, const QNetworkRequest &request, const StringMap &parameters)
    : PutJob(internals, request, parameters)
{
}

template<class T>
void ItemPutJob<T>::parse(const QString &xml)
{
    typename T::Parser p;
    m_item = p.parse(xml);
    setMetadata(p.metadata());
}

template<class T>
T ItemPutJob<T>::result() const
{
    return m_item;
}
