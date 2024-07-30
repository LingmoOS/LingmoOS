/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2008 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef ATTICA_CATEGORY_H
#define ATTICA_CATEGORY_H

#include <QList>
#include <QSharedDataPointer>

#include "attica_export.h"

namespace Attica
{
/**
 * @class Category category.h <Attica/Category>
 *
 * Represents a single content category
 */
class ATTICA_EXPORT Category
{
public:
    typedef QList<Category> List;
    class Parser;

    /**
     * Creates an empty Category
     */
    Category();

    /**
     * Copy constructor.
     * @param other the Category to copy from
     */
    Category(const Category &other);

    /**
     * Assignment operator.
     * @param other the Category to assign from
     * @return pointer to this Category
     */
    Category &operator=(const Category &other);

    /**
     * Destructor.
     */
    ~Category();

    /**
     * Sets the id of the Category.
     * The id uniquely identifies a Category with the OCS API.
     * @param id the new id
     */
    void setId(const QString &);

    /**
     * Gets the id of the Category.
     * The id uniquely identifies a Category with the OCS API.
     * @return the id
     */
    QString id() const;

    /**
     * Sets the name of the Category.
     * @param name the new name
     */
    void setName(const QString &name);

    /**
     * Gets the name of the Category.
     * @return the name
     */
    QString name() const;

    /**
     * Checks whether this Category has an id
     * @return @c true if an id has been set, @c false otherwise
     */
    bool isValid() const;

    /**
     * Sets the display name of the Category.
     * This name is guaranteed to be user friendly, while name may be
     * internal for the server
     * @param name the new name
     * @since 5.31
     */
    void setDisplayName(const QString &name);

    /**
     * Gets the display name of the Category.
     * This name is guaranteed to be user friendly, while name may be
     * internal for the server
     * @return the name
     * @since 5.31
     */
    QString displayName() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

#endif
