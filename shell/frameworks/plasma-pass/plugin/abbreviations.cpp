// SPDX-FileCopyrightText: 2014 Sven Brauch <svenbrauch@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "abbreviations.h"

#include <QVarLengthArray>

namespace
{
constexpr const std::size_t offsetsSize = 32;
constexpr const int maxDepth = 128;

// Taken and adapted for kdevelop from katecompletionmodel.cpp
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
bool matchesAbbreviationHelper(const QStringRef &word,
                               const QStringRef &typed,
                               const QVarLengthArray<int, offsetsSize> &offsets,
                               int &depth,
                               int atWord = -1,
                               int i = 0)
#else
bool matchesAbbreviationHelper(const QStringView &word,
                               const QStringView &typed,
                               const QVarLengthArray<int, offsetsSize> &offsets,
                               int &depth,
                               int atWord = -1,
                               int i = 0)

#endif
{
    int atLetter = 1;
    for (; i < typed.size(); i++) {
        const QChar c = typed.at(i).toLower();
        bool haveNextWord = offsets.size() > atWord + 1;
        bool canCompare = atWord != -1 && word.size() > offsets.at(atWord) + atLetter;
        if (canCompare && c == word.at(offsets.at(atWord) + atLetter).toLower()) {
            // the typed letter matches a letter after the current word beginning
            if (!haveNextWord || c != word.at(offsets.at(atWord + 1)).toLower()) {
                // good, simple case, no conflict
                atLetter += 1;
                continue;
            }
            // For maliciously crafted data, the code used here theoretically can have very high
            // complexity. Thus ensure we don't run into this case, by limiting the amount of branches
            // we walk through to 128.
            depth++;
            if (depth > maxDepth) {
                return false;
            }
            // the letter matches both the next word beginning and the next character in the word
            if (haveNextWord && matchesAbbreviationHelper(word, typed, offsets, depth, atWord + 1, i + 1)) {
                // resolving the conflict by taking the next word's first character worked, fine
                return true;
            }
            // otherwise, continue by taking the next letter in the current word.
            atLetter += 1;
            continue;
        }

        if (haveNextWord && c == word.at(offsets.at(atWord + 1)).toLower()) {
            // the typed letter matches the next word beginning
            atWord++;
            atLetter = 1;
            continue;
        }

        // no match
        return false;
    }
    // all characters of the typed word were matched
    return true;
}

}
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
bool PlasmaPass::matchesAbbreviation(const QStringRef &word, const QStringRef &typed)
#else
bool PlasmaPass::matchesAbbreviation(const QStringView &word, const QStringView &typed)
#endif
{
    // A mismatch is very likely for random even for the first letter,
    // thus this optimization makes sense.
    if (word.at(0).toLower() != typed.at(0).toLower()) {
        return false;
    }

    // First, check if all letters are contained in the word in the right order.
    int atLetter = 0;
    for (const auto c : typed) {
        while (c.toLower() != word.at(atLetter).toLower()) {
            atLetter += 1;
            if (atLetter >= word.size()) {
                return false;
            }
        }
    }

    bool haveUnderscore = true;
    QVarLengthArray<int, offsetsSize> offsets;
    // We want to make "KComplM" match "KateCompletionModel"; this means we need
    // to allow parts of the typed text to be not part of the actual abbreviation,
    // which consists only of the uppercased / underscored letters (so "KCM" in this case).
    // However it might be ambigous whether a letter is part of such a word or part of
    // the following abbreviation, so we need to find all possible word offsets first,
    // then compare.
    for (int i = 0; i < word.size(); ++i) {
        const QChar c = word.at(i);
        if (c == QLatin1Char('_') || c == QLatin1Char('-')) {
            haveUnderscore = true;
        } else if (haveUnderscore || c.isUpper()) {
            offsets.append(i);
            haveUnderscore = false;
        }
    }
    int depth = 0;
    return matchesAbbreviationHelper(word, typed, offsets, depth);
}
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
bool PlasmaPass::matchesPath(const QStringRef &path, const QStringRef &typed)
#else
bool PlasmaPass::matchesPath(const QStringView &path, const QStringView &typed)
#endif
{
    int consumed = 0;
    int pos = 0;
    // try to find all the characters in typed in the right order in the path;
    // jumps are allowed everywhere
    while (consumed < typed.size() && pos < path.size()) {
        if (typed.at(consumed).toLower() == path.at(pos).toLower()) {
            consumed++;
        }
        pos++;
    }
    return consumed == typed.size();
}
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
int PlasmaPass::matchPathFilter(const QVector<QStringRef> &toFilter, const QVector<QStringRef> &text)
#else
int PlasmaPass::matchPathFilter(const QVector<QStringView> &toFilter, const QVector<QStringView> &text)
#endif
{
    enum PathFilterMatchQuality {
        NoMatch = -1,
        ExactMatch = 0,
        StartMatch = 1,
        OtherMatch = 2, // and anything higher than that
    };
    const auto &segments = toFilter;

    if (text.count() > segments.count()) {
        // number of segments mismatches, thus item cannot match
        return NoMatch;
    }

    bool allMatched = true;
    int searchIndex = text.size() - 1;
    int pathIndex = segments.size() - 1;
    int lastMatchIndex = -1;
    // stop early if more search fragments remain than available after path index
    while (pathIndex >= 0 && searchIndex >= 0 && (pathIndex + text.size() - searchIndex - 1) < segments.size()) {
        const auto &segment = segments.at(pathIndex);
        const auto &typedSegment = text.at(searchIndex);
        const int matchIndex = segment.indexOf(typedSegment, 0, Qt::CaseInsensitive);
        const bool isLastPathSegment = pathIndex == segments.size() - 1;
        const bool isLastSearchSegment = searchIndex == text.size() - 1;

        // check for exact matches
        allMatched &= matchIndex == 0 && segment.size() == typedSegment.size();

        // check for fuzzy matches
        bool isMatch = matchIndex != -1;
        // do fuzzy path matching on the last segment
        if (!isMatch && isLastPathSegment && isLastSearchSegment) {
            isMatch = matchesPath(segment, typedSegment);
        } else if (!isMatch) { // check other segments for abbreviations
            isMatch = matchesAbbreviation(segment.mid(0), typedSegment);
        }

        if (!isMatch) {
            // no match, try with next path segment
            --pathIndex;
            continue;
        }
        // else we matched
        if (isLastPathSegment) {
            lastMatchIndex = matchIndex;
        }
        --searchIndex;
        --pathIndex;
    }

    if (searchIndex != -1) {
        return NoMatch;
    }

    const int segmentMatchDistance = segments.size() - (pathIndex + 1);

    if (allMatched) {
        return ExactMatch;
    }

    if (lastMatchIndex == 0) {
        // prefer matches whose last element starts with the filter
        return StartMatch;
    }

    // prefer matches closer to the end of the path
    return OtherMatch + segmentMatchDistance;
}
