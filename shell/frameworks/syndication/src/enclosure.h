/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_ENCLOSURE_H
#define SYNDICATION_ENCLOSURE_H

#include "syndication_export.h"

#include <QSharedPointer>
#include <QString>

namespace Syndication
{
class Enclosure;
//@cond PRIVATE
typedef QSharedPointer<Enclosure> EnclosurePtr;
//@endcond

/**
 * An enclosure describes a (media) file available on the net.
 *
 * Most of the time, enclosures are used for "podcasts", i.e. audio
 * files announced and distributed via syndication.
 *
 * @author Frank Osterfeld
 */
class SYNDICATION_EXPORT Enclosure
{
public:
    /**
     * destructor
     */
    virtual ~Enclosure();

    /**
     * returns whether this enclosure is a null object.
     */
    virtual bool isNull() const = 0;

    /**
     * The URL of the linked resource (required).
     */
    virtual QString url() const = 0;

    /**
     * title of the enclosure. This is a human-readable description of the
     * linked file. If available, the title should be used in user interfaces
     * instead of the URL. If no title is set (e.g., RSS2 enclosures don't
     * have titles), use url() as fallback.
     *
     * @return title describing the enclosure, or a null string if not
     * specified.
     */
    virtual QString title() const = 0;

    /**
     * mimetype of the enclosure.
     * TODO: link mimetype specs
     *
     * Examples are @c "audio/mpeg" for MP3, or @c "application/pdf" for
     * PDF.
     *
     * @return the mimetype of the file, or a null string if not
     * specified
     */
    virtual QString type() const = 0;

    /**
     * returns the length of the linked file in bytes
     *
     * @return the length of the file in bytes, 0 if not specified
     */
    virtual uint length() const = 0;

    /**
     * for audio/video files, the duration of the file in seconds
     *
     * @return the duration of the file in seconds, or 0 if not specified
     */
    virtual uint duration() const = 0;

    /**
     * description of this enclosure for debugging purposes
     *
     * @return debug string
     */
    virtual QString debugInfo() const;
};

} // namespace Syndication

#endif // SYNDICATION_ENCLOSURE_H
