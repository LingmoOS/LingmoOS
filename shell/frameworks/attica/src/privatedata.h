/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Martin Sandsmark <martin.sandsmark@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef ATTICA_PRIVATEDATA_H
#define ATTICA_PRIVATEDATA_H

#include "provider.h"

#include <QDateTime>
#include <QList>
#include <QMap>
#include <QSharedDataPointer>
#include <QUrl>

#include "attica_export.h"

namespace Attica
{

/**
 * @class PrivateData privatedata.h <Attica/PrivateData>
 *
 * Represents private data.
 */
class ATTICA_EXPORT PrivateData
{
public:
    class Parser;

    typedef QList<PrivateData> List; // nonsense

    PrivateData();
    PrivateData(const PrivateData &other);
    PrivateData &operator=(const PrivateData &other);
    ~PrivateData();

    /**
     * Sets an attribute referenced by \key to \value.
     */
    void setAttribute(const QString &key, const QString &value);

    /**
     * Returns an attribute referenced by \key.
     */
    QString attribute(const QString &key) const;

    /**
     * Sets when an attribute last was changed (mostly for internal use).
     */
    void setTimestamp(const QString &key, const QDateTime &when);

    /**
     * Returns the date and time an attribute last was changed.
     */
    QDateTime timestamp(const QString &key) const;

    /**
     * Returns a list of fetched keys.
     */
    QStringList keys() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

#endif // ATTICA_ATTRIBUTES_H
