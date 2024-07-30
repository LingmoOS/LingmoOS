/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_MAPPER_FEEDRSS2IMPL_H
#define SYNDICATION_MAPPER_FEEDRSS2IMPL_H

#include <feed.h>
#include <rss2/document.h>

namespace Syndication
{
class FeedRSS2Impl;
typedef QSharedPointer<FeedRSS2Impl> FeedRSS2ImplPtr;
class Image;
typedef QSharedPointer<Image> ImagePtr;

/**
 * @internal
 */
class FeedRSS2Impl : public Syndication::Feed
{
public:
    explicit FeedRSS2Impl(Syndication::RSS2::DocumentPtr doc);

    Syndication::SpecificDocumentPtr specificDocument() const override;

    QList<ItemPtr> items() const override;

    QList<CategoryPtr> categories() const override;

    QString title() const override;

    QString link() const override;

    QString description() const override;

    QList<PersonPtr> authors() const override;

    QString language() const override;

    QString copyright() const override;

    ImagePtr image() const override;

    ImagePtr icon() const override;

    QMultiMap<QString, QDomElement> additionalProperties() const override;

private:
    Syndication::RSS2::DocumentPtr m_doc;
};

} // namespace Syndication

#endif // SYNDICATION_MAPPER_FEEDRSS2IMPL_H
