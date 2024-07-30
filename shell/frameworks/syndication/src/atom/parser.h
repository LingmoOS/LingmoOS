/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_ATOM_PARSER_H
#define SYNDICATION_ATOM_PARSER_H

#include <syndication/abstractparser.h>

#include <memory>

class QString;
template<class T, class U>
class QHash;

namespace Syndication
{
class SpecificDocument;
class DocumentSource;

namespace Atom
{
/**
 * parser implementation for Atom 1.0 and 0.3.
 *
 * @author Frank Osterfeld
 */
class SYNDICATION_EXPORT Parser : public Syndication::AbstractParser
{
public:
    /** default constructor */
    Parser();

    /** destructor */
    ~Parser() override;

    /**
     * returns whether the source looks like an Atom 1.0 or 0.3
     * document, by checking the root element.
     * @param source document source to check
     */
    bool accept(const Syndication::DocumentSource &source) const override;

    /**
     * parses either an EntryDocument or a FeedDocument from a
     * document source. If the source is not an atom document,
     * an invalid FeedDocument is returned.
     * @see SpecificDocument::isValid()
     * @param source the document source to parse
     */
    Q_REQUIRED_RESULT Syndication::SpecificDocumentPtr parse(const Syndication::DocumentSource &source) const override;

    /**
     * returns the format string for this parser implementation, which is
     * @c "atom"
     * @return @c "atom"
     */
    Q_REQUIRED_RESULT QString format() const override;

private:
    Parser(const Parser &other);
    Parser &operator=(const Parser &other);
    class ParserPrivate;
    std::unique_ptr<ParserPrivate> const d;
};

} // namespace Atom
} // namespace Syndication

#endif // SYNDICATION_ATOM_PARSER_H
