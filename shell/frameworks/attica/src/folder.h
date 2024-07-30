/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2008 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_FOLDER_H
#define ATTICA_FOLDER_H

#include "attica_export.h"
#include <QList>
#include <QSharedDataPointer>
#include <QString>

namespace Attica
{
/**
 * @class Folder folder.h <Attica/Folder>
 *
 * Represents a single mail folder
 */
class ATTICA_EXPORT Folder
{
public:
    typedef QList<Folder> List;
    class Parser;

    /**
     * Creates an empty Folder
     */
    Folder();

    /**
     * Copy constructor.
     * @param other the Folder to copy from
     */
    Folder(const Folder &other);

    /**
     * Assignment operator.
     * @param other the Folder to assign from
     * @return pointer to this Folder
     */
    Folder &operator=(const Folder &other);

    /**
     * Destructor.
     */
    ~Folder();

    /**
     * Sets the id of the Folder.
     * The id uniquely identifies a Folder with the OCS API.
     * @param id the new id
     */
    void setId(const QString &id);

    /**
     * Gets the id of the Folder.
     * The id uniquely identifies a Folder with the OCS API.
     * @return the id
     */
    QString id() const;

    /**
     * Sets the name of the Folder.
     * @param name the new name
     */
    void setName(const QString &name);

    /**
     * Gets the name of the Folder.
     * @return the name
     */
    QString name() const;

    /**
     * Sets the number of messages in the Folder.
     * @param messageCount the new number of messages
     */
    void setMessageCount(int messageCount);

    /**
     * Gets the number of messages in the Folder.
     * @return the number of messages
     */
    int messageCount() const;

    /**
     * Sets the type of the folder
     * @param type the new type
     */
    void setType(const QString &type);

    /**
     * Gets the type of the Folder.
     * @return the type
     */
    QString type() const;

    /**
     * Checks whether this Folder has an id
     * @return @c true if an id has been set, @c false otherwise
     */
    bool isValid() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

#endif
