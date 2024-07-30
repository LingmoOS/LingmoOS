/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BALOO_QUERY_H
#define BALOO_QUERY_H

#include "core_export.h"
#include "resultiterator.h"

#include <QUrl>

#include <memory>

namespace Baloo {

/**
 * @class Query query.h <Baloo/Query>
 *
 * The Query class is the central class to query to search for files from the Index.
 *
 * This class has an inbuilt parser which recognizes words along with AND / OR and parenthesis
 * and specific properties. This can be used with the setSearchString method
 *
 * @example -
 * "Fire" -> Looks for all files which contain the word "Fire"
 *
 * @example -
 * "Fire OR water" -> Looks for files which contain either "Fire" or "Water". The capitalization
 * of the words doesn't matter as that will be ignored internally. However, OR and AND have to
 * be in upper case.
 *
 * @example -
 * "artist:Coldplay" -> Look for any files with the artist "Coldplay"
 *
 * @example -
 * "artist:(Coldplay OR Maroon5) power" -> Look for files with the artist Coldplay or Maroon5 and
 * the word "power"
 *
 * @example -
 * "artist:'Noah and the Whale'" -> Look for files with the artist "Noah and the Whale"
 *
 * @example -
 * "type:Audio title:Fix" -> Look for Audio files which contains the title "Fix" in its title.
 *
 * The Query Parser recognizes a large number of properties. These property names can be looked
 * up in KFileMetaData::Property::Property. The type of the file can mentioned with the property
 * 'type' or 'kind'.
 */
class BALOO_CORE_EXPORT Query
{
public:
    Query();
    Query(const Query& rhs);
    ~Query();

    /**
     * Add a type to the results of the query.
     *
     * Every file has a higher level type such as "Audio", "Video", "Image", "Document", etc.
     *
     * Please note that the types are ANDed together. So searching for "Image"
     * and "Video" will probably never return any results. Have a look at
     * KFileMetaData::TypeInfo for a list of type names.
     */
    void addType(const QString& type);
    void addTypes(const QStringList& typeList);
    void setType(const QString& type);
    void setTypes(const QStringList& types);

    QStringList types() const;

    /**
     * Set some text which should be used to search for Items. This
     * contain a single word or an entire sentence.
     */
    void setSearchString(const QString& str);
    QString searchString() const;

    /**
     * Only a maximum of \p limit results will be returned.
     * By default the value is -1
     */
    void setLimit(uint limit);
    uint limit() const;

    void setOffset(uint offset);
    uint offset() const;

    /**
     * Filter the results in the specified date range.
     *
     * The year/month/day may be set to 0 in order to ignore it.
     */
    void setDateFilter(int year, int month = 0, int day = 0);

    int yearFilter() const;
    int monthFilter() const;
    int dayFilter() const;

    enum SortingOption {
        /**
         * The results are returned in the most efficient order. They can
         * be returned in any order.
         */
        SortNone,

        /**
         * The results are returned in the order Baloo decides
         * should be ideal. This criteria is based on the mtime of the
         * file.
         *
         * This is the default sorting mechanism.
         */
        SortAuto,
    };

    void setSortingOption(SortingOption option);
    SortingOption sortingOption() const;

    /**
     * Only files in this folder will be returned
     */
    void setIncludeFolder(const QString& folder);
    QString includeFolder() const;

    ResultIterator exec();

    QByteArray toJSON();
    static Query fromJSON(const QByteArray& arr);

    QUrl toSearchUrl(const QString& title = QString());
    static Query fromSearchUrl(const QUrl& url);
    static QString titleFromQueryUrl(const QUrl& url);

    bool operator == (const Query& rhs) const;
    bool operator != (const Query& rhs) const;

    Query& operator=(const Query& rhs);

private:
    class Private;
    std::unique_ptr<Private> const d;
};

}
#endif // BALOO_QUERY_H
