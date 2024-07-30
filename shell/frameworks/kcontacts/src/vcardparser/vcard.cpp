/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2003 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "vcard_p.h"

using namespace KContacts;

VCard::VCard()
{
}

VCard::VCard(const VCard &vcard)
    : mLineMap(vcard.mLineMap)
{
}

VCard::~VCard()
{
}

VCard &VCard::operator=(const VCard &vcard)
{
    if (&vcard == this) {
        return *this;
    }

    mLineMap = vcard.mLineMap;

    return *this;
}

void VCard::clear()
{
    mLineMap.clear();
}

QStringList VCard::identifiers() const
{
    QStringList list;
    list.reserve(mLineMap.size());
    for (const auto &[lineId, l] : mLineMap) {
        list.append(lineId);
    }
    return list;
}

void VCard::addLine(const VCardLine &line)
{
    auto it = findByLineId(line.identifier());
    if (it != mLineMap.end()) {
        it->list.append(line);
    } else {
        const LineData newdata{line.identifier(), {line}};
        auto beforeIt = std::lower_bound(mLineMap.begin(), mLineMap.end(), newdata);
        mLineMap.insert(beforeIt, newdata);
    }
}

VCardLine::List VCard::lines(const QString &identifier) const
{
    auto it = findByLineId(identifier);
    return it != mLineMap.cend() ? it->list : VCardLine::List{};
}

VCardLine VCard::line(const QString &identifier) const
{
    auto it = findByLineId(identifier);
    return it != mLineMap.cend() && !it->list.isEmpty() ? it->list.at(0) : VCardLine{};
}

static const char s_verStr[] = "VERSION";

void VCard::setVersion(Version version)
{

    VCardLine line;
    line.setIdentifier(QLatin1String(s_verStr));
    if (version == v2_1) {
        line.setIdentifier(QStringLiteral("2.1"));
    } else if (version == v3_0) {
        line.setIdentifier(QStringLiteral("3.0"));
    } else if (version == v4_0) {
        line.setIdentifier(QStringLiteral("4.0"));
    }

    auto it = findByLineId(QLatin1String(s_verStr));
    if (it != mLineMap.end()) {
        it->list.append(line);
    } else {
        const LineData newData{QLatin1String(s_verStr), {line}};
        auto beforeIt = std::lower_bound(mLineMap.begin(), mLineMap.end(), newData);
        mLineMap.insert(beforeIt, newData);
    }
}

VCard::Version VCard::version() const
{
    auto versionEntryIt = findByLineId(QLatin1String(s_verStr));
    if (versionEntryIt == mLineMap.cend()) {
        return v3_0;
    }

    const VCardLine line = versionEntryIt->list.at(0);
    if (line.value() == QLatin1String("2.1")) {
        return v2_1;
    }
    if (line.value() == QLatin1String("3.0")) {
        return v3_0;
    }

    return v4_0;
}
