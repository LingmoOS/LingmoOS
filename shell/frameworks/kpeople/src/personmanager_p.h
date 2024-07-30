/*
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2013 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef PERSONMANAGER_H
#define PERSONMANAGER_H

#include <QMultiHash>
#include <QObject>
#include <QStringList>

#include <QSqlDatabase>

#include <kpeople/kpeople_export.h>

/**
 * This is a private internal class that manages all the internal mapping of contacts <---> persons
 * It stores the connection to the database as well as signals communicating with other clients
 *
 * It is a singleton.
 *
 */

class KPEOPLE_EXPORT PersonManager : public QObject
{
    Q_OBJECT

public:
    /**
     * Create or return a singleton instance of the PersonManager
     *
     * @databasePath the path to the database to be used.
     * If null the correct database is used.
     * This is for testing purposes only.
     */
    static PersonManager *instance(const QString &databasePath = QString());

    // DATA RETRIEVAL------------

    /** Returns a list of all known personIDs in the database*/
    QMultiHash<QString /*PersonUri*/, QString /*ContactUri*/> allPersons() const;

    /**
     * Returns the ID of a person associated with a given contact
     * If no person for that contact exists, an empty string is returned
     */
    QString personUriForContact(const QString &contactUri) const;

    /**
     * Returns a list of contactUris associated with a given person
     */
    QStringList contactsForPersonUri(const QString &personUri) const;

    /**
     * Creates a contact with the specified @p properties
     *
     * @since 5.62
     */

    bool addContact(const QVariantMap &properties);

public Q_SLOTS:
    // merge all ids (person IDs and contactUris into a single person)
    // returns the ID that will be created
    // users should KPeople::mergeContacts from global.h
    QString mergeContacts(const QStringList &ids);

    // unmerge a contact. Either remove a contact from a given person or remove a person
    // users should KPeople::unmergeContact from global.h
    bool unmergeContact(const QString &id);

Q_SIGNALS:
    void contactRemovedFromPerson(const QString &contactUri);
    void contactAddedToPerson(const QString &contactUri, const QString &newPersonUri);

protected:
    explicit PersonManager(const QString &databasePath, QObject *parent = nullptr);
    ~PersonManager() override;

private:
    QSqlDatabase m_db;
};

#endif // PERSONMANAGER_H
