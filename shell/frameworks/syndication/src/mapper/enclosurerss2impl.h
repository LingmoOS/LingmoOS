/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_MAPPER_ENCLOSURERSS2IMPL_H
#define SYNDICATION_MAPPER_ENCLOSURERSS2IMPL_H

#include <enclosure.h>
#include <rss2/enclosure.h>
#include <rss2/item.h>

namespace Syndication
{
class EnclosureRSS2Impl;
typedef QSharedPointer<EnclosureRSS2Impl> EnclosureRSS2ImplPtr;

/**
 *
 * @internal
 * @author Frank Osterfeld
 */
class EnclosureRSS2Impl : public Syndication::Enclosure
{
public:
    explicit EnclosureRSS2Impl(const Syndication::RSS2::Item &item, const Syndication::RSS2::Enclosure &enc);

    Q_REQUIRED_RESULT bool isNull() const override;

    Q_REQUIRED_RESULT QString url() const override;

    Q_REQUIRED_RESULT QString title() const override;

    Q_REQUIRED_RESULT QString type() const override;

    Q_REQUIRED_RESULT uint length() const override;

    Q_REQUIRED_RESULT uint duration() const override;

private:
    Syndication::RSS2::Item m_item;
    Syndication::RSS2::Enclosure m_enclosure;
};

} // namespace Syndication

#endif // SYNDICATION_MAPPER_ENCLOSURERSS2IMPL_H
