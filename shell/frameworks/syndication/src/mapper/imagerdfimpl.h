/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_MAPPER_IMAGERDFIMPL_H
#define SYNDICATION_MAPPER_IMAGERDFIMPL_H

#include <QString>
#include <image.h>
#include <rdf/image.h>

namespace Syndication
{
class ImageRDFImpl;
typedef QSharedPointer<ImageRDFImpl> ImageRDFImplPtr;

/**
 *
 * @internal
 * @author Frank Osterfeld
 */
class ImageRDFImpl : public Syndication::Image
{
public:
    explicit ImageRDFImpl(const Syndication::RDF::Image &image);

    bool isNull() const override;

    QString url() const override;

    QString title() const override;

    QString link() const override;

    QString description() const override;

    uint width() const override;

    uint height() const override;

private:
    Syndication::RDF::Image m_image;
};

} // namespace Syndication

#endif // SYNDICATION_MAPPER_IMAGERDFIMPL_H
