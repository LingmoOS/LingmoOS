/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Intel Corporation
    SPDX-FileContributor: Mateu Batle Sastre <mbatle@collabora.co.uk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_HOMEPAGETYPE_H
#define ATTICA_HOMEPAGETYPE_H

#include <QSharedDataPointer>
#include <QUrl>

#include "attica_export.h"

namespace Attica
{
/**
    @class HomePageType homepagetype.h <Attica/HomePageType>

    The HomePageType class contains information about one home page type.
    It consists of an integer id and a home page type name.
 */
class ATTICA_EXPORT HomePageType
{
public:
    typedef QList<HomePageType> List;
    class Parser;

    /**
     * Creates an empty HomePageType
     */
    HomePageType();

    /**
     * Copy constructor.
     * @param other the HomePageType to copy from
     */
    HomePageType(const HomePageType &other);

    /**
     * Assignment operator.
     * @param other the HomePageType to assign from
     * @return pointer to this HomePageType
     */
    HomePageType &operator=(const HomePageType &other);

    /**
     * Destructor.
     */
    ~HomePageType();

    /*
    <id>10</id>
    <name>Blog</name>
    */

    uint id() const;
    void setId(uint id);

    QString name() const;
    void setName(const QString &name);

private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

#endif
