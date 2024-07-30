/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_MAPPER_FEEDRDFIMPL_H
#define SYNDICATION_MAPPER_FEEDRDFIMPL_H

#include <feed.h>
#include <rdf/document.h>

namespace Syndication
{
class FeedRDFImpl;
typedef QSharedPointer<FeedRDFImpl> FeedRDFImplPtr;
class Image;
typedef QSharedPointer<Image> ImagePtr;

/**
 * @internal
 */
class FeedRDFImpl : public Syndication::Feed
{
public:
    explicit FeedRDFImpl(Syndication::RDF::DocumentPtr doc);

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

    QMultiMap<QString, QDomElement> additionalProperties() const override;

    ImagePtr icon() const override;

private:
    Syndication::RDF::DocumentPtr m_doc;
};

} // namespace Syndication

#endif // SYNDICATION_MAPPER_FEEDRDFIMPL_H
