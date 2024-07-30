/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2015-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef RESOURCELOCATORURL_H
#define RESOURCELOCATORURL_H

#include "kcontacts_export.h"

#include <QMap>
#include <QMetaType>
#include <QSharedDataPointer>
#include <QString>
#include <QUrl>

class ResourceLocatorUrlTest;

namespace KContacts
{
class ParameterMap;

/** @short Class that holds a Resource Locator
 *  @since 5.0
 */
class KCONTACTS_EXPORT ResourceLocatorUrl
{
    friend KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &, const ResourceLocatorUrl &);
    friend KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &, ResourceLocatorUrl &);
    friend class VCardTool;
    friend class ::ResourceLocatorUrlTest;

    Q_GADGET
    Q_PROPERTY(QUrl url READ url WRITE setUrl)
    Q_PROPERTY(bool isValid READ isValid)
    Q_PROPERTY(Type type READ type WRITE setType)
    Q_PROPERTY(bool isPreferred READ isPreferred WRITE setPreferred)

public:
    ResourceLocatorUrl();
    ResourceLocatorUrl(const ResourceLocatorUrl &other);

    ~ResourceLocatorUrl();

    typedef QList<ResourceLocatorUrl> List;

    /** URL types.
     *  @since 5.12
     *  @see Type
     */
    enum TypeFlag {
        Unknown = 0, /**< No or unknown URL type is set. */
        Home = 1, /**< Personal website. */
        Work = 2, /**< Work website. */
        Profile = 4, /**< Profile website. */
        Ftp = 8, /**< Ftp website. @since 6.0 */
        Reservation = 16, /**< Reservation website. @since 6.0 */
        AppInstallPage = 32, /**< Application installation website. @sine 6.0 */
        Other = 64, /**< Other websie. */
    };

    /**
     * Stores a combination of #TypeFlag values.
     */
    Q_DECLARE_FLAGS(Type, TypeFlag)
    Q_FLAG(Type)

    Q_REQUIRED_RESULT bool isValid() const;

    void setUrl(const QUrl &url);
    Q_REQUIRED_RESULT QUrl url() const;

    /**
     * Returns the type of the URL.
     * @since 5.12
     */
    Type type() const;
    /**
     * Sets the URL type.
     * @since 5.12
     */
    void setType(Type type);

    /**
     * Returns whether this is the preferred website.
     * @since 5.12
     */
    bool isPreferred() const;
    /**
     * Sets that this is the preferred website.
     * @since 5.12
     */
    void setPreferred(bool preferred);

    Q_REQUIRED_RESULT bool operator==(const ResourceLocatorUrl &other) const;
    Q_REQUIRED_RESULT bool operator!=(const ResourceLocatorUrl &other) const;

    ResourceLocatorUrl &operator=(const ResourceLocatorUrl &other);

    Q_REQUIRED_RESULT QString toString() const;

private:
    // exported for ResourceLocatorUrlTest
    void setParams(const ParameterMap &params);
    Q_REQUIRED_RESULT ParameterMap params() const;

    class Private;
    QSharedDataPointer<Private> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ResourceLocatorUrl::Type)

KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &stream, const ResourceLocatorUrl &object);

KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &stream, ResourceLocatorUrl &object);
}

Q_DECLARE_TYPEINFO(KContacts::ResourceLocatorUrl, Q_RELOCATABLE_TYPE);

#endif // RESOURCELOCATORURL_H
