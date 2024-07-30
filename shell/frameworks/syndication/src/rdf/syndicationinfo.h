/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_RDF_SYNDICATIONINFO_H
#define SYNDICATION_RDF_SYNDICATIONINFO_H

#include <syndication/rdf/resourcewrapper.h>

#include <ctime>

class QString;

namespace Syndication
{
namespace RDF
{
/**
 * Wrapper to access syndication information for a feed.
 * The RSS 1.0 syndication module provides syndication hints to
 * aggregators regarding how often it is updated.
 *
 * The specification can be found at
 * http://web.resource.org/rss/1.0/modules/syndication/
 *
 * @author Frank Osterfeld
 */
class SyndicationInfo : public ResourceWrapper
{
public:
    /**
     * update period enum as used by updatePeriod().
     */
    enum Period {
        Hourly, /**< the feed is updated hourly */
        Daily, /**< the feed is updated daily */
        Weekly, /**< the feed is updated weekly */
        Monthly, /**< the feed is updated monthly */
        Yearly, /**< the feed is updated yearly */
    };

    /**
     * creates a wrapper wrapping a null resource.
     * isNull() will be true.
     */
    SyndicationInfo();

    /**
     * creates a wrapper from a resource
     * @param resource the feed resource to read syndication
     * information from
     */
    explicit SyndicationInfo(ResourcePtr resource);

    /**
     * virtual destructor
     */
    ~SyndicationInfo() override;

    /**
     * Describes the period over which the channel format is updated.
     * Acceptable values are: hourly, daily, weekly, monthly, yearly.
     * If omitted, daily is assumed.
     *
     * @return update period, daily is default
     */
    Period updatePeriod() const;

    /** Used to describe the frequency of updates in relation to the
     * update period. A positive integer indicates how many times in
     * that period the channel is updated. For example, an
     * updatePeriod of daily, and an updateFrequency of 2 indicates
     * the channel format is updated twice daily. If omitted a value
     * of 1 is assumed.
     *
     * @return update frequency, default is 1
     */
    int updateFrequency() const;

    /**
     * Defines a base date to be used in concert with updatePeriod
     * and updateFrequency to calculate the publishing schedule.
     *
     * @return the base date in seconds since epoch. Default value is
     * 0 (epoch).
     */
    time_t updateBase() const;

    /**
     * description of the syndication information
     * for debugging purposes
     *
     * @return debug string
     */
    QString debugInfo() const;

protected:
    /**
     * returns Period value as string.
     * @param period period enum to convert to a string
     * @return the enum name in lower case, "daily", "hourly", etc.
     */
    static QString periodToString(Period period);

    /**
     * parses a Period value from a string.
     *
     * @param str a period string as defined in the syndication module
     * @return the parsed period, Daily (the default) if the parsed
     * string is empty or invalid
     */
    static Period stringToPeriod(const QString &str);
};

} // namespace RDF
} // namespace Syndication

#endif // SYNDICATION_RDF_SYNDICATIONINFO_H
