/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_PERSON_H
#define SYNDICATION_PERSON_H

#include <QSharedPointer>
#include <QString>

#include "syndication_export.h"

namespace Syndication
{
class Person;

//@cond PRIVATE
typedef QSharedPointer<Person> PersonPtr;
//@endcond

/**
 * Person objects hold information about a person, such as the author of
 * the content syndicated in the feed. Depending on the feed format, different
 * information is available.
 * While according to the RSS2 spec, RSS2 author elements must contain only an
 * e-mail address, Atom requires the person's name and the e-mail address is
 * optional. Also, in reality, feeds often contain other information than what
 * is specified in the specs. Syndication tries to find out what author
 * information is contained and maps it to this representation.
 *
 * @author Frank Osterfeld
 */
class SYNDICATION_EXPORT Person
{
public:
    /**
     * destructor
     */
    virtual ~Person();

    /**
     * returns whether this object is a null person
     */
    virtual bool isNull() const = 0;

    /**
     * the name of the person (optional)
     *
     * @return the name of the person as plain text,
     * or a null string if not specified
     */
    virtual QString name() const = 0;

    /**
     * a URI associated with the person. (optional)
     * This is usually the URL of the
     * person's homepage.
     *
     * @return URI of the person, or a null string if not specified
     */
    virtual QString uri() const = 0;

    /**
     * e-mail address of the person (optional)
     *
     * @return email address, or a null string if not specified
     */
    virtual QString email() const = 0;

    /**
     * description of the person for debugging purposes.
     *
     * @return debug string
     */
    virtual QString debugInfo() const;

    /**
     * compares two person instances. Persons are equal if and only if
     * their respective name(), uri() and email() values are equal.
     * @param other another person instance
     */
    virtual bool operator==(const Person &other) const;
};

} // namespace Syndication

#endif // SYNDICATION_PERSON_H
