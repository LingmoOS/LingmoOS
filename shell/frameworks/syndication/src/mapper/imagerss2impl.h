/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_MAPPER_IMAGERSS2IMPL_H
#define SYNDICATION_MAPPER_IMAGERSS2IMPL_H

#include <image.h>
#include <rss2/image.h>

namespace Syndication
{
class ImageRSS2Impl;
typedef QSharedPointer<ImageRSS2Impl> ImageRSS2ImplPtr;

/**
 *
 * @internal
 * @author Frank Osterfeld
 */
class ImageRSS2Impl : public Syndication::Image
{
public:
    explicit ImageRSS2Impl(const Syndication::RSS2::Image &image);

    bool isNull() const override;

    QString url() const override;

    QString title() const override;

    QString link() const override;

    QString description() const override;

    uint width() const override;

    uint height() const override;

private:
    Syndication::RSS2::Image m_image;
};

} // namespace Syndication

#endif // SYNDICATION_MAPPER_IMAGERSS2IMPL_H
