/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "jsoncommand.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaProperty>

namespace Bugzilla
{
QByteArray JsonCommand::toJson() const
{
    QJsonDocument doc;
    doc.setObject(QJsonObject::fromVariantHash(toVariantHash()));
    return doc.toJson();
}

QVariantHash JsonCommand::toVariantHash() const
{
    QVariantHash hash;

    const auto propertyCount = metaObject()->propertyCount();
    for (int i = 0; i < propertyCount; ++i) {
        const auto property = metaObject()->property(i);
        const auto name = QString::fromLatin1(property.name());
        const auto value = property.read(this);

        if (name == QLatin1String("objectName")) { // Builtin property.
            continue;
        }

        if (value.isNull()) {
            continue;
        }

        // If this is a nested representation, serialize it and glue it in.
        if (value.canConvert<JsonCommand *>()) {
            auto *repValue = value.value<JsonCommand *>();
            hash.insert(name, repValue->toVariantHash());
            continue;
        }

        hash.insert(name, value);
    }

    return hash;
}

} // namespace Bugzilla

#include "moc_jsoncommand.cpp"
