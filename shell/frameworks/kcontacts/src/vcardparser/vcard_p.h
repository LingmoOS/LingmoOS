/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2003 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef VCARDPARSER_VCARD_H
#define VCARDPARSER_VCARD_H

#include "vcardline_p.h"
#include <QList>
#include <QMap>
#include <QStringList>

#include <vector>

namespace KContacts
{
class VCard
{
public:
    typedef QList<VCard> List;

    struct LineData {
        QString identifier;
        VCardLine::List list;
    };
    using LineMap = std::vector<LineData>;

    inline LineMap::iterator findByLineId(const QString &identifier)
    {
        return std::find_if(mLineMap.begin(), mLineMap.end(), [&identifier](const LineData &data) {
            return data.identifier == identifier;
        });
    }

    inline LineMap::const_iterator findByLineId(const QString &identifier) const
    {
        return std::find_if(mLineMap.cbegin(), mLineMap.cend(), [&identifier](const LineData &data) {
            return data.identifier == identifier;
        });
    }

    enum Version {
        v2_1,
        v3_0,
        v4_0,
    };

    VCard();
    VCard(const VCard &card);

    ~VCard();

    VCard &operator=(const VCard &card);

    /**
     * Removes all lines from the vCard.
     */
    void clear();

    /**
     * Returns a list of all identifiers that exists in the vCard.
     */
    Q_REQUIRED_RESULT QStringList identifiers() const;

    /**
     * Adds a VCardLine to the VCard
     */
    void addLine(const VCardLine &line);

    /**
     * Returns all lines of the vcard with a special identifier.
     */
    Q_REQUIRED_RESULT VCardLine::List lines(const QString &identifier) const;

    /**
     * Returns only the first line of the vcard with a special identifier.
     */
    Q_REQUIRED_RESULT VCardLine line(const QString &identifier) const;

    /**
     * Set the version of the vCard.
     */
    void setVersion(Version version);

    /**
     * Returns the version of this vCard.
     */
    Q_REQUIRED_RESULT Version version() const;

private:
    /**
     * A container of LineData, sorted by identifier.
     */
    LineMap mLineMap;
};

inline bool operator<(const VCard::LineData &a, const VCard::LineData &b)
{
    return a.identifier < b.identifier;
}
}

#endif
