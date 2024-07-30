/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_MAPPER_IMAGEATOMIMPL_H
#define SYNDICATION_MAPPER_IMAGEATOMIMPL_H

#include <image.h>

#include <QString>

namespace Syndication
{
class ImageAtomImpl;
typedef QSharedPointer<ImageAtomImpl> ImageAtomImplPtr;

/**
 *
 * @internal
 * @author Frank Osterfeld
 */
class ImageAtomImpl : public Syndication::Image
{
public:
    explicit ImageAtomImpl(const QString &logoURI)
        : m_logoURI(logoURI)
    {
    }

    bool isNull() const override
    {
        return m_logoURI.isEmpty();
    }

    QString url() const override
    {
        return m_logoURI;
    }

    QString title() const override
    {
        return QString();
    }

    QString link() const override
    {
        return QString();
    }

    QString description() const override
    {
        return QString();
    }

    uint width() const override
    {
        return 0;
    }

    uint height() const override
    {
        return 0;
    }

private:
    QString m_logoURI;
};

} // namespace Syndication

#endif // SYNDICATION_MAPPER_IMAGEATOMIMPL_H
