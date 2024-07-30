/*
    SPDX-FileCopyrightText: 2018 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNSCORE_TAGSFILTERCHECKER_H
#define KNSCORE_TAGSFILTERCHECKER_H

#include "knewstuffcore_export.h"
#include <QStringList>

#include <memory>

namespace KNSCore
{
class TagsFilterCheckerPrivate;
/**
 * @brief Apply simple filtering logic to a list of tags
 *
 * == Examples of specifying tag filters ==
 * Value for tag "tagname" must be exactly "tagdata":
 * tagname==tagdata
 *
 * Value for tag "tagname" must be different from "tagdata":
 * tagname!=tagdata
 *
 * == Tag filter list ==
 * A tag filter list is a string list of filters as shown above, and a combination
 * of which might look like:
 *
 * - ghns_excluded!=1
 * - data##mimetype==application/cbr+zip
 * - data##mimetype==application/cbr+rar
 *
 * which would filter out anything which has ghns_excluded set to 1, and
 * anything where the value of data##mimetype does not equal either
 * "application/cbr+zip" or "application/cbr+rar".
 * Notice in particular the two data##mimetype entries. Use this
 * for when a tag may have multiple values.
 *
 * The value does not current support wildcards. The list should be considered
 * a binary AND operation (that is, all filter entries must match for the data
 * entry to be included in the return data)
 * @since 5.51
 */
class KNEWSTUFFCORE_EXPORT TagsFilterChecker
{
public:
    /**
     * Constructs an instance of the tags filter checker, prepopulated
     * with the list of tag filters in the tagFilter parameter.
     *
     * @param tagFilter The list of tag filters
     * @since 5.51
     */
    explicit TagsFilterChecker(const QStringList &tagFilter);
    ~TagsFilterChecker();

    TagsFilterChecker(const TagsFilterChecker &) = delete;
    TagsFilterChecker &operator=(const TagsFilterChecker &) = delete;

    /**
     * Check whether the filter list accepts the passed list of tags
     *
     * @param tags A list of tags in the form of key=value strings
     * @return True if the filter accepts the list, false if not
     * @since 5.51
     */
    bool filterAccepts(const QStringList &tags);

private:
    const std::unique_ptr<TagsFilterCheckerPrivate> d;
};

}

#endif // KNSCORE_TAGSFILTERCHECKER_H
