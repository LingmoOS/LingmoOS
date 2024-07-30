/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2017 Forrest Smith <forrestthewoods@gmail.com>
    SPDX-FileCopyrightText: 2021 Waqar Ahmed   <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "kfuzzymatcher.h"

#include <QList>
#include <QString>
#include <QStringView>

/**
 * Custom toLower function which is much faster than
 * c.toLower() directly
 */
static inline constexpr QChar toLower(QChar c)
{
    return c.isLower() ? c : c.toLower();
}

// internal
// clang-format off
static bool match_recursive(QStringView::const_iterator pattern,
                            QStringView::const_iterator str,
                            int &outScore,
                            const QStringView::const_iterator strBegin,
                            const QStringView::const_iterator strEnd,
                            const QStringView::const_iterator patternEnd,
                            const uint8_t *srcMatches,
                            uint8_t *matches,
                            int nextMatch,
                            int &totalMatches,
                            int &recursionCount)
{
    static constexpr int recursionLimit = 10;
    // max number of matches allowed, this should be enough
    static constexpr int maxMatches = 256;

    // Count recursions
    ++recursionCount;
    if (recursionCount >= recursionLimit) {
        return false;
    }

    // Detect end of strings
    if (pattern == patternEnd || str == strEnd) {
        return false;
    }

    // Recursion params
    bool recursiveMatch = false;
    uint8_t bestRecursiveMatches[maxMatches];
    int bestRecursiveScore = 0;

    // Loop through pattern and str looking for a match
    bool firstMatch = true;
    QChar currentPatternChar = toLower(*pattern);
    // Are we matching in sequence start from start?
    bool matchingInSequence = true;
    while (pattern != patternEnd && str != strEnd) {
        // Found match
        if (currentPatternChar == toLower(*str)) {
            // Supplied matches buffer was too short
            if (nextMatch >= maxMatches) {
                return false;
            }

            // "Copy-on-Write" srcMatches into matches
            if (firstMatch && srcMatches) {
                memcpy(matches, srcMatches, nextMatch);
                firstMatch = false;
            }

            // Recursive call that "skips" this match
            uint8_t recursiveMatches[maxMatches];
            int recursiveScore = 0;
            const auto strNextChar = std::next(str);
            if (!matchingInSequence && match_recursive(pattern, strNextChar, recursiveScore, strBegin,
                                strEnd, patternEnd, matches, recursiveMatches,
                                nextMatch, totalMatches, recursionCount)) {
                // Pick best recursive score
                if (!recursiveMatch || recursiveScore > bestRecursiveScore) {
                    memcpy(bestRecursiveMatches, recursiveMatches, maxMatches);
                    bestRecursiveScore = recursiveScore;
                }
                recursiveMatch = true;
            }

            // Advance
            matches[nextMatch++] = (uint8_t)(std::distance(strBegin, str));
            ++pattern;
            currentPatternChar = toLower(*pattern);
        } else {
            matchingInSequence = false;
        }
        ++str;
    }

    // Determine if full pattern was matched
    const bool matched = pattern == patternEnd;

    // Calculate score
    if (matched) {
        static constexpr int sequentialBonus = 25;
        static constexpr int separatorBonus = 25; // bonus if match occurs after a separator
        static constexpr int camelBonus = 25; // bonus if match is uppercase and prev is lower
        static constexpr int firstLetterBonus = 15; // bonus if the first letter is matched

        static constexpr int leadingLetterPenalty = -5; // penalty applied for every letter in str before the first match
        static constexpr int maxLeadingLetterPenalty = -15; // maximum penalty for leading letters
        static constexpr int unmatchedLetterPenalty = -1; // penalty for every letter that doesn't matter

        static constexpr int nonBeginSequenceBonus = 10;

        // Initialize score
        outScore = 100;

        // Apply leading letter penalty
        const int penalty = std::max(leadingLetterPenalty * matches[0], maxLeadingLetterPenalty);

        outScore += penalty;

        // Apply unmatched penalty
        const int unmatched = (int)(std::distance(strBegin, strEnd)) - nextMatch;
        outScore += unmatchedLetterPenalty * unmatched;

        uint8_t seqs[maxMatches] = {0};

        // Apply ordering bonuses
        int j = 0;
        for (int i = 0; i < nextMatch; ++i) {
            const uint8_t currIdx = matches[i];

            if (i > 0) {
                const uint8_t prevIdx = matches[i - 1];

                // Sequential
                if (currIdx == (prevIdx + 1)) {
                    if (j > 0 && seqs[j - 1] == i - 1){
                        outScore += sequentialBonus;
                        seqs[j++] = i;
                    } else {
                        // In sequence, but from first char
                        outScore += nonBeginSequenceBonus;
                    }
                }
            }

            // Check for bonuses based on neighbor character value
            if (currIdx > 0) {
                // Camel case
                const QChar neighbor = *(strBegin + currIdx - 1);
                const QChar curr = *(strBegin + currIdx);
                if (neighbor.isLower() && curr.isUpper()) {
                    outScore += camelBonus;
                }

                // Separator
                const bool neighborSeparator = neighbor == QLatin1Char('_') || neighbor == QLatin1Char(' ');
                if (neighborSeparator) {
                    outScore += separatorBonus;
                }
            } else {
                // First letter
                outScore += firstLetterBonus;
                seqs[j++] = i;
            }
        }
    }

    totalMatches = nextMatch;

    // Return best result
    if (recursiveMatch && (!matched || bestRecursiveScore > outScore)) {
        // Recursive score is better than "this"
        memcpy(matches, bestRecursiveMatches, maxMatches);
        outScore = bestRecursiveScore;
        return true;
    } else if (matched) {
        // "this" score is better than recursive
        return true;
    } else {
        // no match
        return false;
    }
}
// clang-format on

static bool match_internal(QStringView pattern, QStringView str, int &outScore, unsigned char *matches)
{
    if (pattern.isEmpty()) {
        return true;
    }

    int recursionCount = 0;

    auto strIt = str.cbegin();
    auto patternIt = pattern.cbegin();
    const auto patternEnd = pattern.cend();
    const auto strEnd = str.cend();

    int total = 0;
    return match_recursive(patternIt, strIt, outScore, strIt, strEnd, patternEnd, nullptr, matches, 0, total, recursionCount);
}

/**************************************************************/

bool KFuzzyMatcher::matchSimple(QStringView pattern, QStringView str)
{
    auto patternIt = pattern.cbegin();
    /**
     * Instead of doing
     *
     *      strIt.toLower() == patternIt.toLower()
     *
     * we convert patternIt to Upper / Lower as needed and compare with strIt. This
     * saves us from calling toLower() on both strings, making things a little bit faster
     */
    bool lower = patternIt->isLower();
    QChar cUp = lower ? patternIt->toUpper() : *patternIt;
    QChar cLow = lower ? *patternIt : patternIt->toLower();
    for (auto strIt = str.cbegin(); strIt != str.cend() && patternIt != pattern.cend(); ++strIt) {
        if (*strIt == cLow || *strIt == cUp) {
            ++patternIt;
            lower = patternIt->isLower();
            cUp = lower ? patternIt->toUpper() : *patternIt;
            cLow = lower ? *patternIt : patternIt->toLower();
        }
    }

    return patternIt == pattern.cend();
}

KFuzzyMatcher::Result KFuzzyMatcher::match(QStringView pattern, QStringView str)
{
    /**
     * Simple substring matching to flush out non-matching strings
     */
    const bool simpleMatch = matchSimple(pattern, str);

    KFuzzyMatcher::Result result;
    result.matched = false;
    result.score = 0;

    if (!simpleMatch) {
        return result;
    }

    // actual algorithm
    int score = 0;
    uint8_t matches[256];
    const bool matched = match_internal(pattern, str, score, matches);
    result.matched = matched;
    result.score = score;
    return result;
}

QList<KFuzzyMatcher::Range> KFuzzyMatcher::matchedRanges(QStringView pattern, QStringView str, RangeType type)
{
    QList<KFuzzyMatcher::Range> ranges;
    if (pattern.isEmpty()) {
        return ranges;
    }

    int totalMatches = 0;
    int score = 0;
    int recursionCount = 0;

    auto strIt = str.cbegin();
    auto patternIt = pattern.cbegin();
    const auto patternEnd = pattern.cend();
    const auto strEnd = str.cend();

    uint8_t matches[256];
    auto res = match_recursive(patternIt, strIt, score, strIt, strEnd, patternEnd, nullptr, matches, 0, totalMatches, recursionCount);
    // didn't match? => We don't care about results
    if (!res && type == RangeType::FullyMatched) {
        return {};
    }

    int previousMatch = 0;
    for (int i = 0; i < totalMatches; ++i) {
        auto matchPos = matches[i];
        /**
         * Check if this match is part of the previous
         * match. If it is, we increase the length of
         * the last range.
         */
        if (!ranges.isEmpty() && matchPos == previousMatch + 1) {
            ranges.last().length++;
        } else {
            /**
             * This is a new match inside the string
             */
            ranges.push_back({/* start: */ matchPos, /* length: */ 1});
        }
        previousMatch = matchPos;
    }

    return ranges;
}
