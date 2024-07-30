/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_ATOM_PERSON_H
#define SYNDICATION_ATOM_PERSON_H

#include <syndication/elementwrapper.h>

class QDomElement;
class QString;

namespace Syndication
{
namespace Atom
{
/**
 * describes a person, with name and optional URI and e-mail address.
 * Used to describe authors and contributors of feeds/entries
 *
 *  @author Frank Osterfeld
 */
class SYNDICATION_EXPORT Person : public ElementWrapper
{
public:
    /**
     * creates a null person object
     */
    Person();

    /**
     * creates a Person object wrapping an Atom Person Construct (atom:author,
     * atom:contributor tags)
     * @param element a DOM element, should be a Atom Person Construct
     * (although not enforced), otherwise this object will not parse
     * anything useful
     */
    explicit Person(const QDomElement &element);

    /**
     * a human-readable name for the person. (required)
     * The name is a required  attribute of person constructs.
     *
     * @return a human-readable name of the person
     */
    Q_REQUIRED_RESULT QString name() const;

    /**
     * A URI associated with the person (optional). Usually the homepage.
     *
     * @return the URI of the person, or a null string if not specified
     */
    Q_REQUIRED_RESULT QString uri() const;

    /**
     * returns an e-mail address associated with the person. (optional)
     *
     * @return an e-mail address, or a null string if not specified
     */
    Q_REQUIRED_RESULT QString email() const;

    /**
     * description for debugging purposes
     *
     * @return debug string
     */
    Q_REQUIRED_RESULT QString debugInfo() const;
};

} // namespace Atom
} // namespace Syndication

#endif // SYNDICATION_ATOM_PERSON_H
