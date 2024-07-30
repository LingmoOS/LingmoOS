/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006 Jacob R Rideout <kde@jacobrideout.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef GUESSLANGUAGE_H
#define GUESSLANGUAGE_H

#include <QString>
#include <QStringList>

#include "sonnetcore_export.h"

#include <memory>

namespace Sonnet
{
// Amount of trigrams in each file
static const int MAXGRAMS = 300;

class GuessLanguagePrivate;

/**
 * @class Sonnet::GuessLanguage guesslanguage.h <Sonnet/GuessLanguage>
 *
 * @short GuessLanguage determines the language of a given text.
 *
 * GuessLanguage can determine the difference between ~75 languages for a given string. It is
 * based off a Perl script originally written by Maciej Ceglowski <maciej@ceglowski.com>
 * called Languid. His script used a 2 part heuristic to determine language. First the text
 * is checked for the scripts it contains, then for each set of languages using those
 * scripts a n-gram frequency model of a given language is compared to a model of the text.
 * The most similar language model is assumed to be the language. If no language is found
 * an empty string is returned.
 *
 *
 * @author Jacob Rideout <kde@jacobrideout.net>
 * @since 4.3
 */
class SONNETCORE_EXPORT GuessLanguage
{
public:
    /** Constructor
     * Creates a new GuessLanguage instance. If @p text is specified,
     * it sets the text to be checked.
     * @param text the text that is to be checked
     */
    GuessLanguage();

    /** Destructor
     */
    ~GuessLanguage();

    GuessLanguage(const GuessLanguage &) = delete;
    GuessLanguage &operator=(const GuessLanguage &) = delete;

    /**
     * Sets limits to number of languages returned by identify(). The confidence for each language is computed
     * as difference between this and next language on the list normalized to 0-1 range. Reasonable value to get
     * fairly sure result is 0.1 . Default is returning best guess without caring about confidence - exactly
     * as after call to setLimits(1,0).
     * @param maxItems The list returned by identify() will never have more than maxItems item
     * @param minConfidence The list will have only enough items for their summary confidence equal
     * or exceed minConfidence.
     */
    void setLimits(int maxItems, double minConfidence);

    /**
     * Returns the 2 digit ISO 639-1 code for the language of the currently
     * set text and. Three digits are returned only in the case where a 2 digit
     * code does not exist. If @p text isn't empty, set the text to checked.
     * @param text to be identified
     * @return list of the presumed languages of the text, sorted by decreasing confidence. Empty list means
     * it is impossible to determine language with confidence required by setLimits
     */
    QString identify(const QString &text, const QStringList &suggestions = QStringList()) const;

private:
    std::unique_ptr<GuessLanguagePrivate> const d;
};
}

#endif
