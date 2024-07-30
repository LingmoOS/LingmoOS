/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2017 Forrest Smith <forrestthewoods@gmail.com>
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KFUZZYMATCHER_H
#define KFUZZYMATCHER_H

#include <kcoreaddons_export.h>

#include <QtContainerFwd>
#include <QtGlobal>

class QString;
class QStringView;

/**
 * This namespace contains functions for fuzzy matching a list of strings
 * against a pattern.
 *
 * This code is ported to Qt from lib_fts:
 * https://github.com/forrestthewoods/lib_fts
 * which tries to replicate SublimeText like fuzzy matching.
 *
 * @note
 * All character matches will happen sequentially. That means that this function is not
 * typo tolerant i.e., "gti" will not match "git", but "gt" will. All methods in here are
 * stateless i.e., the input string will not be modified. Also note that strings in all the
 * functions in this namespace will be matched case-insensitively.
 *
 * Limitations:
 * - Currently this will match only strings with length < 256 correctly. This is because we
 * intend on matching a pattern against words / short strings and not paragraphs.
 * - No more than 256 matches will happen.
 *
 * If you are using this with @c QSortFilterProxyModel, you need to override both
 * @c QSortFilterProxyModel::lessThan and @c QSortFilterProxyModel::filterAcceptsRow.
 * A simple example:
 *
 * \code
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override
    {
        int score = 0;
        const auto idx = sourceModel()->index(sourceRow, 0, sourceParent);
        const auto actionName = idx.data().toString().splitRef(QLatin1Char(':')).at(1);
        const bool res = kfts::fuzzy_match_sequential(m_pattern, actionName, score);
        // store the score in the source model
        sourceModel()->setData(idx, score, ScoreRole);
        return res;
    }

    bool lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const override
    {
        // use the score here to sort
        const int l = sourceLeft.data(ScoreRole).toInt();
        const int r = sourceRight.data(ScoreRole).toInt();
        return l < r;
    }
 * \endcode
 *
 * Additionally you must not use @c invalidateFilter() if you go with the above approach. Instead
 * use @c beginResetModel()/@c endResetModel():
 *
 * \code
 *  Q_SLOT void setFilterString(const QString &string)
    {
        beginResetModel();
        m_pattern = string;
        endResetModel();
    }
 * \endcode
 *
 * @short Namespace for fuzzy matching of strings
 * @author Waqar Ahmed <waqar.17a@gmail.com>
 */
namespace KFuzzyMatcher
{
/**
 * @brief The result of a fuzzy match
 */
struct KCOREADDONS_EXPORT Result {
    /**
     * Score of this match. This can be negative.if matched is @c false
     * then the score will be zero.
     */
    int score = 0;
    /** @c true if match was successful */
    bool matched = false;
};

/**
 * @brief A range representing a matched sequence in a string
 *
 * @since 5.84
 */
struct KCOREADDONS_EXPORT Range {
    int start;
    int length;
};

/**
 * @brief The type of matches to consider when requesting for ranges.
 * @see matchedRanges
 *
 * @since 5.84
 */
enum class RangeType : unsigned char {
    /**
     * We want ranges only where the pattern fully matches the user
     * supplied string
     */
    FullyMatched,
    /**
     * We want ranges for all matches, even if the pattern partially
     * matched the user supplied string
     */
    All
};

/**
 * @brief Simple fuzzy matching of chars in @p pattern with chars in @p str
 * sequentially. If there is a match, it will return true and false otherwise.
 * There is no scoring. You should use this if score is not important for you
 * and only matching is important.
 *
 * If @p pattern is empty, the function will return @c true
 *
 * @param pattern to search for. For e.g., text entered by a user to filter a
 * list
 * @param str the current string from your list of strings
 * @return @c true on sucessful match
 *
 * @since 5.79
 */
KCOREADDONS_EXPORT bool matchSimple(QStringView pattern, QStringView str);

/**
 * @brief This is the main function which does scored fuzzy matching.
 *
 * The return value of this function contains Result#score which should be used to
 * sort the results. Without sorting of the results this function won't very effective.
 *
 * If @p pattern is empty, the function will return @c true
 *
 * @param pattern to search for. For e.g., text entered by a user to filter a
 * list or model
 * @param str the current string from your list of strings
 * @return A Result type with score of this match and whether the match was
 * successful. If there is no match, score is zero. If the match is successful,
 * score must be used to sort the results.
 *
 * @since 5.79
 */
KCOREADDONS_EXPORT Result match(QStringView pattern, QStringView str);

/**
 * @brief A function which returns the positions + lengths where the @p pattern matched
 * inside the @p str. The resulting ranges can then be utilized to show the user where
 * the matches occurred. Example:
 *
 * @code
 * String: hello
 * Pattern: Hlo
 *
 * Output: [Range{0, 1}, Range{3, 2}]
 * @endcode
 *
 * In the above example @c "Hlo" matched inside the string @c "hello" in two places i.e.,
 * position 0 and position 3. At position 0 it matched 'h', and at position 3 it
 * matched 'lo'.
 *
 * The ranges themeselves can't do much so you will have to make the result useful
 * in your own way. Some possible uses can be:
 * - Transform the result into a vector of @c QTextLayout::FormatRange and then paint
 *   them in the view
 * - Use the result to transform the string into html, for example conver the string from
 *   above example to "\<b\>H\</b\>el\<b\>lo\</b\>, and then use @c QTextDocument
 *   to paint it into your view.
 *
 * Example with the first method:
 * @code
 *       auto ranges = KFuzzyMatcher::matchedRanges(pattern, string);
 *       QList<QTextLayout::FormatRange> out;
 *       std::transform(ranges.begin(), ranges.end(), std::back_inserter(out), [](const KFuzzyMatcher::Range &fr){
 *          return QTextLayout::FormatRange{fr.start, fr.length, QTextCharFormat()};
 *       });
 *
 *       QTextLayout layout(text, font);
 *       layout.beginLayout();
 *       QTextLine line = layout.createLine();
 *       //...
 *       layout.endLayout();
 *
 *       layout.setFormats(layout.formats() + out);
 *       layout.draw(painter, position);
 * @endcode
 *
 * If @p pattern is empty, the function will return an empty vector
 *
 * if @p type is @c RangeType::All, the function will try to get ranges even if
 * the pattern didn't fully match. For example:
 * @code
 * pattern: "git"
 * string: "gti"
 * RangeType: All
 *
 * Output: [Range{0, 1}, Range{2, 1}]
 * @endcode
 *
 * @param pattern to search for. For e.g., text entered by a user to filter a
 * list or model
 * @param str the current string from your list of strings
 * @param type whether to consider ranges from full matches only or all matches including partial matches
 * @return A vector of ranges containing positions and lengths where the pattern
 * matched. If there was no match, the vector will be empty
 *
 * @since 5.84
 */
KCOREADDONS_EXPORT QList<KFuzzyMatcher::Range> matchedRanges(QStringView pattern, QStringView str, RangeType type = RangeType::FullyMatched);

} // namespace KFuzzyMatcher

Q_DECLARE_TYPEINFO(KFuzzyMatcher::Range, Q_PRIMITIVE_TYPE);

#endif // KFUZZYMATCHER_H
