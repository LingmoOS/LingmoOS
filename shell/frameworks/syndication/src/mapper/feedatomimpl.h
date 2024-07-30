/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_MAPPER_FEEDATOMIMPL_H
#define SYNDICATION_MAPPER_FEEDATOMIMPL_H

#include <atom/document.h>
#include <feed.h>

namespace Syndication
{
class FeedAtomImpl;
typedef QSharedPointer<FeedAtomImpl> FeedAtomImplPtr;
class Image;
typedef QSharedPointer<Image> ImagePtr;

/**
 * @internal
 */
class FeedAtomImpl : public Syndication::Feed
{
public:
    explicit FeedAtomImpl(Syndication::Atom::FeedDocumentPtr doc);

    Syndication::SpecificDocumentPtr specificDocument() const override;

    QList<Syndication::ItemPtr> items() const override;

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
    Syndication::Atom::FeedDocumentPtr m_doc;
};

} // namespace Syndication

#endif // SYNDICATION_MAPPER_FEEDATOMIMPL_H
