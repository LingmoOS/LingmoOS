/*
    SPDX-FileCopyrightText: 2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_LICENSE_H
#define ATTICA_LICENSE_H

#include <QSharedDataPointer>
#include <QUrl>

#include "attica_export.h"

namespace Attica
{
/**
    @class License license.h <Attica/License>

    The License class contains information about one license that the server offers.
    It consists of an integer id, a name and a link to a webpage describing the license.
 */
class ATTICA_EXPORT License
{
public:
    typedef QList<License> List;
    class Parser;

    /**
     * Creates an empty License
     */
    License();

    /**
     * Copy constructor.
     * @param other the License to copy from
     */
    License(const License &other);

    /**
     * Assignment operator.
     * @param other the License to assign from
     * @return pointer to this License
     */
    License &operator=(const License &other);

    /**
     * Destructor.
     */
    ~License();

    /*
    <id>3</id>
    <name>Artistic 2.0</name>
    <link>http://dev.perl.org/perl6/rfc/346.html</link>
    */

    uint id() const;
    void setId(uint id);

    QString name() const;
    void setName(const QString &name);

    QUrl url() const;
    void setUrl(const QUrl &url);

private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

#endif
