/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006 Jacob R Rideout <kde@jacobrideout.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TEXTBREAKS_H
#define TEXTBREAKS_H

class QString;

#include "sonnetcore_export.h"

#include <memory>

namespace Sonnet
{
class TextBreaksPrivate;

/**
 * @short TextBreaks determines the barriers between linguistic structures in any given text.
 *
 * TextBreaks is a class that determines the boundaries between graphemes
 * (characters as per the unicode definition,) words and sentences. The
 * default implementation conforms to Unicode Standard Annex #29 https://unicode.org/reports/tr29/.
 * You can subclass TextBreaks to create the correct behaviour for languages that require it.
 *
 * @author Jacob Rideout <kde@jacobrideout.net>
 * @since 4.3
 */
class SONNETCORE_EXPORT TextBreaks
{
public:
    struct Position {
        int start, length;
    };

    /**
     * This structure abstracts the positions of breaks in the test. As per the
     * unicode annex, both the start and end of the text are returned.
     */
    typedef QList<Position> Positions;

    /** Constructor
     * Creates a new TextBreaks instance. If @p text is specified,
     * it sets the text to be checked.
     * @param text the text that is to be checked
     */
    explicit TextBreaks(const QString &text = QString());

    /** Virtual Destructor
     */
    virtual ~TextBreaks();

    /**
     * Returns the text to be checked
     * @return text
     */
    QString text() const;

    /**
     * Sets the text to @p text
     * @param text to be set
     * @return true if the word is misspelled. false otherwise
     */
    void setText(const QString &text);

    /**
     * Return the Positions of each word for the given  @p text.
     * @param text to be checked
     * @return positions of breaks
     */
    static Positions wordBreaks(const QString &text);

    /**
     * Return the Positions of each sentence for the given  @p text.
     * @param text to be checked
     * @return positions of breaks
     */
    static Positions sentenceBreaks(const QString &text);

    /**
     * Return the Positions of each word for the text previously set.
     * @return positions of breaks
     */
    virtual Positions wordBreaks() const;

    /**
     * Return the Positions of each sentence for the text previously set.
     * @return positions of breaks
     */
    virtual Positions sentenceBreaks() const;

private:
    std::unique_ptr<TextBreaksPrivate> const d;
};
}

Q_DECLARE_TYPEINFO(Sonnet::TextBreaks::Position, Q_PRIMITIVE_TYPE);

#endif
