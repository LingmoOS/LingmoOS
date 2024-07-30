/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>
    SPDX-FileCopyrightText: 2011 Laszlo Papp <djszapi@archlinux.us>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef ATTICA_ITEMJOB_H
#define ATTICA_ITEMJOB_H

#include "attica_export.h"
#include "deletejob.h"
#include "getjob.h"
#include "postjob.h"
#include "putjob.h"

namespace Attica
{
class Provider;

/**
 * @class ItemJob itemjob.h <Attica/ItemJob>
 *
 * Represents an item get job.
 */
template<class T>
class ATTICA_EXPORT ItemJob : public GetJob
{
public:
    T result() const;

private:
    ItemJob(PlatformDependent *, const QNetworkRequest &request);
    void parse(const QString &xml) override;
    T m_item;
    friend class Attica::Provider;
};

/**
 * @class ItemDeleteJob itemjob.h <Attica/ItemJob>
 *
 * Represents an item delete job.
 */
template<class T>
class ATTICA_EXPORT ItemDeleteJob : public DeleteJob
{
public:
    T result() const;

private:
    ItemDeleteJob(PlatformDependent *, const QNetworkRequest &request);
    void parse(const QString &xml) override;
    T m_item;
    friend class Attica::Provider;
};

/**
 * @class ItemPostJob itemjob.h <Attica/ItemJob>
 *
 * Represents an item post job.
 */
template<class T>
class ATTICA_EXPORT ItemPostJob : public PostJob
{
public:
    T result() const;

private:
    ItemPostJob(PlatformDependent *internals, const QNetworkRequest &request, QIODevice *data);
    ItemPostJob(PlatformDependent *internals, const QNetworkRequest &request, const StringMap &parameters = StringMap());

    void parse(const QString &xml) override;
    T m_item;
    friend class Attica::Provider;
};

/**
 * @class ItemPutJob itemjob.h <Attica/ItemJob>
 *
 * Represents an item put job.
 */
template<class T>
class ATTICA_EXPORT ItemPutJob : public PutJob
{
public:
    T result() const;

private:
    ItemPutJob(PlatformDependent *internals, const QNetworkRequest &request, QIODevice *data);
    ItemPutJob(PlatformDependent *internals, const QNetworkRequest &request, const StringMap &parameters = StringMap());

    void parse(const QString &xml) override;
    T m_item;
    friend class Attica::Provider;
};

}

#endif
