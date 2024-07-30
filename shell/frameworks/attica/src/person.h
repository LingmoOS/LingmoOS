/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2008 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_PERSON_H
#define ATTICA_PERSON_H

#include <QDate>
#include <QList>
#include <QMap>
#include <QSharedDataPointer>
#include <QUrl>

#include "attica_export.h"

namespace Attica
{

/**
 * @class Person person.h <Attica/Person>
 *
 * Represents a person.
 */
class ATTICA_EXPORT Person
{
public:
    typedef QList<Person> List;
    class Parser;

    Person();
    Person(const Person &other);
    Person &operator=(const Person &other);
    ~Person();

    void setId(const QString &);
    QString id() const;

    void setFirstName(const QString &);
    QString firstName() const;

    void setLastName(const QString &);
    QString lastName() const;

    void setBirthday(const QDate &);
    QDate birthday() const;

    void setCountry(const QString &);
    QString country() const;

    void setLatitude(qreal);
    qreal latitude() const;

    void setLongitude(qreal);
    qreal longitude() const;

    void setAvatarUrl(const QUrl &);
    QUrl avatarUrl() const;

    void setHomepage(const QString &);
    QString homepage() const;

    void setCity(const QString &);
    QString city() const;

    void addExtendedAttribute(const QString &key, const QString &value);
    QString extendedAttribute(const QString &key) const;

    QMap<QString, QString> extendedAttributes() const;

    bool isValid() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

#endif
