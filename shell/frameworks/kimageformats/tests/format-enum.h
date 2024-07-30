/*
    SPDX-FileCopyrightText: 2014 Alex Merry <alex.merry@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef FORMAT_ENUM_H
#define FORMAT_ENUM_H

#include <QImage>
#include <QMetaEnum>

QImage::Format formatFromString(const QString &str)
{
    const QMetaEnum metaEnum = QMetaEnum::fromType<QImage::Format>();
    const QString enumString = QStringLiteral("Format_") + str;

    bool ok;
    const int res = metaEnum.keyToValue(enumString.toLatin1().constData(), &ok);

    return ok ? static_cast<QImage::Format>(res) : QImage::Format_Invalid;
}

QString formatToString(QImage::Format format)
{
    const QMetaEnum metaEnum = QMetaEnum::fromType<QImage::Format>();
    return QString::fromLatin1(metaEnum.valueToKey(format)).remove(QStringLiteral("Format_"));
}

#endif
