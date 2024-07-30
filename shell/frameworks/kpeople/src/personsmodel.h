/*
    Persons Model
    SPDX-FileCopyrightText: 2012 Martin Klapetek <martin.klapetek@gmail.com>
    SPDX-FileCopyrightText: 2012 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef PERSONS_MODEL_H
#define PERSONS_MODEL_H

#include <kpeople/kpeople_export.h>

#include "global.h"

#include <QAbstractItemModel>

namespace KPeople
{
class ContactItem;
class PersonItem;
class MetaContact;
class PersonsModelPrivate;

/**
 * This class creates a model of all known contacts from all sources
 * Contacts are represented as a tree where the top level represents a "person" which is an
 * amalgamation of all the sub-contacts
 *
 * @since 5.8
 */
class KPEOPLE_EXPORT PersonsModel : public QAbstractItemModel
{
    Q_OBJECT
    /** specifies whether the model has already been initialized */
    Q_PROPERTY(bool isInitialized READ isInitialized NOTIFY modelInitialized)
public:
    enum Role {
        FormattedNameRole = Qt::DisplayRole, // QString best name for this person
        PhotoRole = Qt::DecorationRole, // QPixmap best photo for this person
        PersonUriRole = Qt::UserRole, // QString ID of this person
        PersonVCardRole, // AbstractContact::Ptr
        ContactsVCardRole, // AbstractContact::List (FIXME or map?)

        GroupsRole, /// groups QStringList
        PhoneNumberRole,
        PhotoImageProviderUri, ///< Provide a URL to use with QtQuick's Image.source, similar to the Photo Role. @since 5.93

        UserRole = Qt::UserRole + 0x1000, ///< in case it's needed to extend, use this one to start from
    };
    Q_ENUM(Role)

    explicit PersonsModel(QObject *parent = nullptr);

    ~PersonsModel() override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    /** @returns the index for a given @p personUri */
    QModelIndex indexForPersonUri(const QString &personUri) const;

    /** Returns if all the backends have been initialized yet. */
    bool isInitialized() const;

    /** Helper class to ease model access through QML */
    Q_SCRIPTABLE QVariant get(int row, int role);

    /**
     * Makes it possible to access custom properties that are not available to the model
     *
     * @returns the property for the contact at @p index defined by the @p key
     */
    QVariant contactCustomProperty(const QModelIndex &index, const QString &key) const;

Q_SIGNALS:
    /** Will emit when the model is finally initialized. @p success will specify if it succeeded */
    void modelInitialized(bool success);

private:
    Q_DISABLE_COPY(PersonsModel)

    QScopedPointer<PersonsModelPrivate> const d_ptr;
    Q_DECLARE_PRIVATE(PersonsModel)
};
}

#endif // PERSONS_MODEL_H
