/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_ATOM_GENERATOR_H
#define SYNDICATION_ATOM_GENERATOR_H

#include <syndication/elementwrapper.h>

class QDomElement;
class QString;

namespace Syndication
{
namespace Atom
{
/**
 * Description of the agent used to generate the feed.
 *
 * @author Frank Osterfeld
 */
class SYNDICATION_EXPORT Generator : public ElementWrapper
{
public:
    /**
     * default constructor, creates a null generator
     */
    Generator();

    /**
     * creates a Generator wrapping an atom:generator element.
     * @param element a DOM element, should be a atom:generator element
     * (although not enforced), otherwise this object will not parse
     * anything useful
     */
    explicit Generator(const QDomElement &element);

    /**
     * A URI for the generator (e.g. its homepage) (optional)
     */
    Q_REQUIRED_RESULT QString uri() const;

    /**
     * version of the agent (optional)
     */
    Q_REQUIRED_RESULT QString version() const;

    /**
     * human-readable name of the generator. (optional)
     *
     * @return generator name as plain text
     */
    Q_REQUIRED_RESULT QString name() const;

    /**
     * a description of this generator for debugging purposes.
     *
     * @return debug info
     */
    Q_REQUIRED_RESULT QString debugInfo() const;
};

} // namespace Atom
} // namespace Syndication

#endif // SYNDICATION_ATOM_GENERATOR_H
