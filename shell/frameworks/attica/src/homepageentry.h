/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Intel Corporation
    SPDX-FileContributor: Mateu Batle Sastre <mbatle@collabora.co.uk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_HOMEPAGEENTRY_H
#define ATTICA_HOMEPAGEENTRY_H

#include <QSharedDataPointer>
#include <QUrl>

#include "attica_export.h"

namespace Attica
{
/**
    @class HomePageEntry homepageentry.h <Attica/HomePageEntry>

    The HomePageEntry class contains information about one home page entry.
    It consists of a type and a home page url.
 */
class ATTICA_EXPORT HomePageEntry
{
public:
    typedef QList<HomePageEntry> List;

    /**
     * Creates an empty HomePageEntry
     */
    HomePageEntry();

    /**
     * Copy constructor.
     * @param other the HomePageEntry to copy from
     */
    HomePageEntry(const HomePageEntry &other);

    /**
     * Assignment operator.
     * @param other the HomePageEntry to assign from
     * @return pointer to this HomePageEntry
     */
    HomePageEntry &operator=(const HomePageEntry &other);

    /**
     * Destructor.
     */
    ~HomePageEntry();

    QString type() const;
    void setType(const QString &type);

    QUrl url() const;
    void setUrl(const QUrl &url);

private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

#endif
