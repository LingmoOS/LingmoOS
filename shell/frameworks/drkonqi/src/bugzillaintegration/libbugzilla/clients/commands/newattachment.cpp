/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "newattachment.h"

#include <QVariantHash>

namespace Bugzilla
{
QVariantHash NewAttachment::toVariantHash() const
{
    auto hash = JsonCommand::toVariantHash();

    QVariantList idsVariant;
    for (int id : ids) {
        idsVariant << QVariant::fromValue(id);
    }
    hash[QStringLiteral("ids")] = idsVariant;
    hash[QStringLiteral("data")] = data.toUtf8().toBase64();

    return hash;
}

} // namespace Bugzilla

#include "moc_newattachment.cpp"
