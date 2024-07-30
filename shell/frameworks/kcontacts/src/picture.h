/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2002 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONTACTS_PICTURE_H
#define KCONTACTS_PICTURE_H

#include "kcontacts_export.h"

#include <QDataStream>
#include <QImage>
#include <QSharedDataPointer>
#include <QString>

namespace KContacts
{
class PicturePrivate;

/**
  A class to store a picture of an addressee. It can store the data directly or
  an url reference to a picture.
*/
class KCONTACTS_EXPORT Picture
{
    friend KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &, const Picture &);
    friend KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &, Picture &);

    Q_GADGET
    Q_PROPERTY(QImage data READ data WRITE setData)
    Q_PROPERTY(QString url READ url WRITE setUrl)
    Q_PROPERTY(bool isIntern READ isIntern)
    Q_PROPERTY(bool isEmpty READ isEmpty)

public:
    /**
     * Creates an empty picture.
     */
    Picture();

    /**
     * Creates a picture which points to the given url.
     *
     * @param url A URL that describes the location of the picture file.
     */
    Picture(const QString &url);

    /**
     * Creates a picture with the given data.
     *
     * @param data The raw data of the picture.
     */
    Picture(const QImage &data);

    /**
     * Copy constructor.
     *
     * Fast operation, Picture's data is implicitly shared.
     *
     * @param picture The Picture instance to copy from
     */
    Picture(const Picture &picture);

    /**
     * Destructor.
     */
    ~Picture();

    typedef QList<Picture> List;
    /**
     * Assignment operator
     *
     * Fast operation, Picture's data is implicitly shared.
     *
     * @param other The Picture instance to assign to @c this
     */
    Picture &operator=(const Picture &other);

    /**
     * Equality operator.
     */
    Q_REQUIRED_RESULT bool operator==(const Picture &other) const;

    /**
     * Not-Equal operator.
     */
    Q_REQUIRED_RESULT bool operator!=(const Picture &other) const;

    /**
     * Returns true, if the picture is empty.
     */
    Q_REQUIRED_RESULT bool isEmpty() const;

    /**
     * Sets a URL for the location of the picture file. When using this
     * function, isIntern() will return 'false' until you use
     * setData().
     * This also clears the type, as it is unknown.
     *
     * @param url  The location URL of the picture file.
     */
    void setUrl(const QString &url);

    /**
     * Sets a URL for the location of the picture file. When using this
     * function, isIntern() will return 'false' until you use
     * setData().
     *
     * @param url  The location URL of the picture file.
     * @param type  The encoding format of the image, e.g. jpeg or png
     * @since 4.10
     */
    void setUrl(const QString &url, const QString &type);

    /**
     * Sets the image data of the picture. When using this function,
     * isIntern() will return 'true' until you use setUrl().
     * This also sets type to "png" or "jpeg" depending
     * on whether the image has an alpha channel or not.
     *
     * @param data  The image data of the picture.
     */
    void setData(const QImage &data);

    /**
     * Sets the raw data of the picture. When using this function,
     * isIntern() will return 'true' until you use setUrl().
     *
     * @param rawData  The raw data of the picture.
     * @param type  The encoding format of the image, e.g. jpeg or png
     * @since 4.10
     */
    void setRawData(const QByteArray &rawData, const QString &type);

    /**
     * Returns whether the picture is described by a URL (extern) or
     * by the raw data (intern).
     * When this method returns 'true' you can use data() to
     * get the raw data. Otherwise you can request the URL of this
     * picture by url() and load the raw data from that location.
     */
    Q_REQUIRED_RESULT bool isIntern() const;

    /**
     * Returns the location URL of this picture.
     */
    Q_REQUIRED_RESULT QString url() const;

    /**
     * Returns the image data of this picture.
     */
    Q_REQUIRED_RESULT QImage data() const;

    /**
     * Returns the raw data of this picture.
     *
     * @since 4.10
     */
    Q_REQUIRED_RESULT QByteArray rawData() const;

    /**
     * Returns the type of this picture.
     */
    Q_REQUIRED_RESULT QString type() const;

    /**
     * Returns string representation of the picture.
     */
    Q_REQUIRED_RESULT QString toString() const;

private:
    QSharedDataPointer<PicturePrivate> d;
};

/**
 * Serializes the @p picture object into the @p stream.
 */
KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &stream, const Picture &picture);

/**
 * Initializes the @p picture object from the @p stream.
 */
KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &stream, Picture &picture);
}
Q_DECLARE_TYPEINFO(KContacts::Picture, Q_RELOCATABLE_TYPE);
#endif
