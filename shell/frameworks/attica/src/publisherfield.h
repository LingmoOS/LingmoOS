/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2011 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef PUBLISHERFIELD_H
#define PUBLISHERFIELD_H

#include "attica_export.h"
#include "buildservice.h"

namespace Attica
{

/**
 * @class PublisherField publisherfield.h <Attica/PublisherField>
 *
 * Represents a publisher field.
 */
class ATTICA_EXPORT PublisherField
{
public:
    typedef QList<PublisherField> List;
    class Parser;

    PublisherField();
    PublisherField(const PublisherField &other);
    PublisherField &operator=(const PublisherField &other);
    ~PublisherField();

    void setName(const QString &value);
    QString name() const;

    void setType(const QString &value);
    QString type() const;

    void setData(const QString &value);
    QString data() const;

    bool isValid() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

} // namespace Attica

#endif // PUBLISHERFIELD_H
