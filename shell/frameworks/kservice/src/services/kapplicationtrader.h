/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 Torben Weis <weis@kde.org>
    SPDX-FileCopyrightText: 2006-2020 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KAPPLICATIONTRADER_H
#define KAPPLICATIONTRADER_H

#include <functional>
#include <kservice.h>

/**
 * @namespace KApplicationTrader
 *
 * The application trader is a convenient way to find installed applications
 * based on specific criteria (association with a MIME type, name contains Foo, etc.)
 *
 * Example: say that you want to get the list of all applications that can handle PNG images.
 * The code would look like:
 * \code
 * KService::List lst = KApplicationTrader::queryByMimeType("image/png");
 * \endcode
 *
 * If you want to get the preferred application for image/png you would use:
 * @code
 * KService::Ptr service = KApplicationTrader::preferredService("image/png");
 * @endcode
 *
 * @see KService
 */
namespace KApplicationTrader
{
/**
 * Filter function, used for filtering results of query and queryByMimeType.
 */
using FilterFunc = std::function<bool(const KService::Ptr &)>;

/**
 * This method returns a list of services (applications) that match a given filter.
 *
 * @param filter a callback function that returns @c true if the application
 * should be selected and @c false if it should be skipped.
 *
 * @return A list of services that satisfy the query
 * @since 5.68
 */
KSERVICE_EXPORT KService::List query(FilterFunc filterFunc);

/**
 * This method returns a list of services (applications) which are associated with a given MIME type.
 *
 * @param mimeType a MIME type like 'text/plain' or 'text/html'
 * @param filter a callback function that returns @c true if the application
 * should be selected and @c false if it should be skipped. Do not return
 * true for all services, this would return the complete list of all
 * installed applications (slow).
 *
 * @return A list of services that satisfy the query, sorted by preference
 * (preferred service first)
 * @since 5.68
 */
KSERVICE_EXPORT KService::List queryByMimeType(const QString &mimeType, FilterFunc filterFunc = {});

/**
 * Returns the preferred service for @p mimeType
 *
 * This a convenience method for queryByMimeType(mimeType).at(0), with a check for empty.
 *
 * @param mimeType the MIME type (see query())
 * @return the preferred service, or @c nullptr if no service is available
 * @since 5.68
 */
KSERVICE_EXPORT KService::Ptr preferredService(const QString &mimeType);

/**
 * Changes the preferred service for @p mimeType to @p service
 *
 * You may need to rebuild KSyCoca for the change to be reflected
 *
 * @param mimeType the MIME type
 * @param service the service to set as the preferred one
 * @since 5.101
 */
KSERVICE_EXPORT void setPreferredService(const QString &mimeType, const KService::Ptr service);

/**
 * Returns true if @p pattern matches a subsequence of the string @p text.
 * For instance the pattern "libremath" matches the text "LibreOffice Math", assuming
 * @p cs is Qt::CaseInsensitive.
 *
 * This can be useful from your filter function, e.g. with @p text being service->name().
 * @since 5.68
 */
KSERVICE_EXPORT bool isSubsequence(const QString &pattern, const QString &text, Qt::CaseSensitivity cs = Qt::CaseSensitive);
}

#endif
