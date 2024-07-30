/*
    This file is part of the KContacts framework.

    SPDX-FileCopyrightText: 2002 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "vcarddrag.h"

#include "converter/vcardconverter.h"

#include <QMimeDatabase>

using namespace KContacts;

static QString findCompatibleMimeType(const QMimeData *md)
{
    if (!md) {
        return {};
    }
    // check the canonical MIME type first
    if (md->hasFormat(KContacts::Addressee::mimeType())) {
        return KContacts::Addressee::mimeType();
    }

    const QStringList mimeTypeOffers = md->formats();
    const QMimeDatabase db;
    for (const QString &mimeType : mimeTypeOffers) {
        const QMimeType mimeTypePtr = db.mimeTypeForName(mimeType);
        if (mimeTypePtr.isValid() && mimeTypePtr.inherits(KContacts::Addressee::mimeType())) {
            return mimeType;
        }
    }

    return QString();
}

bool VCardDrag::populateMimeData(QMimeData *md, const QByteArray &content)
{
    md->setData(KContacts::Addressee::mimeType(), content);
    return true;
}

bool VCardDrag::populateMimeData(QMimeData *md, const KContacts::Addressee::List &addressees)
{
    KContacts::VCardConverter converter;
    const QByteArray vcards = converter.createVCards(addressees);
    if (!vcards.isEmpty()) {
        return populateMimeData(md, vcards);
    } else {
        return false;
    }
}

bool VCardDrag::canDecode(const QMimeData *md)
{
    return !findCompatibleMimeType(md).isEmpty();
}

bool VCardDrag::fromMimeData(const QMimeData *md, QByteArray &content)
{
    const QString mimeOffer = findCompatibleMimeType(md);
    if (mimeOffer.isEmpty()) {
        return false;
    }
    content = md->data(mimeOffer);
    return !content.isEmpty();
}

bool VCardDrag::fromMimeData(const QMimeData *md, KContacts::Addressee::List &addressees)
{
    const QString mimeOffer = findCompatibleMimeType(md);
    if (mimeOffer.isEmpty()) {
        return false;
    }
    addressees = KContacts::VCardConverter().parseVCards(md->data(mimeOffer));
    return !addressees.isEmpty();
}
