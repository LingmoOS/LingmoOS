/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Intel Corporation
    SPDX-FileContributor: Mateu Batle Sastre <mbatle@collabora.co.uk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_DISTRIBUTION_H
#define ATTICA_DISTRIBUTION_H

#include <QSharedDataPointer>
#include <QUrl>

#include "attica_export.h"

namespace Attica
{
/**
    @class Distribution distribution.h <Attica/Distribution>

    The Distribution class contains information about one distribution that the server offers.
    It consists of an integer id and a distribution name.
 */
class ATTICA_EXPORT Distribution
{
public:
    typedef QList<Distribution> List;
    class Parser;

    /**
     * Creates an empty Distribution
     */
    Distribution();

    /**
     * Copy constructor.
     * @param other the Distribution to copy from
     */
    Distribution(const Distribution &other);

    /**
     * Assignment operator.
     * @param other the Distribution to assign from
     * @return pointer to this Distribution
     */
    Distribution &operator=(const Distribution &other);

    /**
     * Destructor.
     */
    ~Distribution();

    /*
     <id>2000</id>
     <name>Ark</name>
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
