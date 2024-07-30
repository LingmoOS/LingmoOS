/*
    SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "avatarimageprovider.h"

#include <abstractcontact.h>
#include <persondata.h>

#include "kpeopledeclarative_debug.h"

AvatarImageProvider::AvatarImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Pixmap)
{
}

QPixmap AvatarImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    const auto base64encoded = QStringView(id).split(u'#').constFirst();
    const auto decoded = QByteArray::fromBase64(base64encoded.toUtf8(), QByteArray::AbortOnBase64DecodingErrors);
    if (decoded.isEmpty()) {
        qCDebug(KPEOPLE_DECLARATIVE_LOG) << "AvatarImageProvider:" << id << "could not be decoded as a person uri";
        return {};
    }

    auto personUri = QString::fromUtf8(decoded);
    if (personUri.isEmpty()) {
        qCDebug(KPEOPLE_DECLARATIVE_LOG()) << "AvatarImageProvider:"
                                           << "passed person uri" << personUri << "was not valid utf8";
        return {};
    }

    KPeople::PersonData person(personUri);
    if (!person.isValid()) {
        qCDebug(KPEOPLE_DECLARATIVE_LOG()) << "AvatarImageProvider:"
                                           << "No contact found with person uri" << personUri;
        return {};
    }

    const auto avatar = [&person]() -> QPixmap {
        QVariant pic = person.contactCustomProperty(KPeople::AbstractContact::PictureProperty);
        if (pic.canConvert<QImage>()) {
            return QPixmap::fromImage(pic.value<QImage>());
        } else if (pic.canConvert<QUrl>()) {
            return QPixmap(pic.toUrl().toLocalFile());
        }
        return {};
    }();

    if (avatar.isNull()) {
        return {};
    }

    if (size) {
        *size = requestedSize;
    }

    return avatar.scaled(requestedSize);
}
