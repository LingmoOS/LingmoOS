/*
    SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#pragma once

#include <QQuickImageProvider>

/**
 * @brief An image provider for displaying a contact's photo
 *
 * It is automatically available when importing the KPeople QML module.
 *
 * Image URIs for it can be retrieved from `KPeople.PersonData`'s `photoImageProviderUri` property,
 * or from `KPeople.PersonsModel`'s `photoImageProviderUri` role.
 *
 * Example:
 * \code
 * Kirigami.Avatar {
 *     source: person.photoImageProviderUri
 * }
 * \endcode
 *
 * @since 5.93.0
 */
class AvatarImageProvider : public QQuickImageProvider
{
public:
    explicit AvatarImageProvider();

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;
};
