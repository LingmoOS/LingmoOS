/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2003-2007 Josef Spillner <spillner@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_AUTHOR_P_H
#define KNEWSTUFF3_AUTHOR_P_H

#include <QSharedData>
#include <QString>
#include <QUrl>

#include "knewstuffcore_export.h"

namespace KNSCore
{
class AuthorPrivate;

/**
 * @short KNewStuff author information.
 *
 * This class provides accessor methods to the author data
 * as used by KNewStuff.
 * It should probably not be used directly by the application.
 *
 * @author Josef Spillner (spillner@kde.org)
 */
class KNEWSTUFFCORE_EXPORT Author
{
    Q_GADGET
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString email READ email)

public:
    explicit Author();
    Author(const Author &other);
    Author &operator=(const Author &other);
    ~Author();

    /**
     * Sets the user ID of the author.
     */
    void setId(const QString &id);

    /**
     * Retrieve the author's user ID
     * @return the author's user ID
     */
    QString id() const;

    /**
     * Sets the full name of the author.
     */
    void setName(const QString &name);

    /**
     * Retrieve the author's name.
     *
     * @return author name
     */
    QString name() const;

    /**
     * Sets the email address of the author.
     */
    void setEmail(const QString &email);

    /**
     * Retrieve the author's email address.
     *
     * @return author email address
     */
    QString email() const;

    /**
     * Sets the jabber address of the author.
     */
    void setJabber(const QString &jabber);

    /**
     * Retrieve the author's jabber address.
     *
     * @return author jabber address
     */
    QString jabber() const;

    /**
     * Sets the homepage of the author.
     */
    void setHomepage(const QString &homepage);

    /**
     * Retrieve the author's homepage.
     *
     * @return author homepage
     */
    QString homepage() const;

    /**
     * Sets the profile page of the author, usually located on the server hosting the content.
     */
    void setProfilepage(const QString &profilepage);

    /**
     * Retrieve the author's profile page.
     *
     * @return author profile page
     */
    QString profilepage() const;

    /**
     * Sets the url for the user's avatar image
     */
    void setAvatarUrl(const QUrl &avatarUrl);

    /**
     * Retrieve the url of the user's avatar image
     *
     * @return a url for the user's avatar (may be empty)
     */
    QUrl avatarUrl() const;

    /**
     * Retrieve the user's description text
     *
     * @return A long(ish)-form text describing this user, usually self-entered
     */
    QString description() const;
    /**
     * Set the user's description
     */
    void setDescription(const QString &description);

private:
    QSharedDataPointer<AuthorPrivate> d;
};

}

#endif
