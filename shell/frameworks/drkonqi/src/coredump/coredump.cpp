/*
    SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
    SPDX-FileCopyrightText: 2019-2022 Harald Sitter <sitter@kde.org>
*/

#include "coredump.h"

using namespace Qt::StringLiterals;

Coredump::Coredump(QByteArray cursor, EntriesHash data)
    : m_cursor(std::move(cursor))
    , m_rawData(std::move(data))
    , uid(m_rawData[QByteArrayLiteral("COREDUMP_UID")].toInt())
    , pid(m_rawData[QByteArrayLiteral("COREDUMP_PID")].toInt())
    , exe(QString::fromLocal8Bit(m_rawData[QByteArrayLiteral("COREDUMP_EXE")]))
    , filename(QString::fromLocal8Bit(m_rawData[keyFilename()]))
    , systemd_unit(QString::fromLocal8Bit(m_rawData[QByteArrayLiteral("_SYSTEMD_UNIT")]))
    , bootId(QString::fromUtf8(m_rawData["_BOOT_ID"_ba]))
    , timestamp(QString::fromUtf8(m_rawData["COREDUMP_TIMESTAMP"_ba]))
{
    if (!m_rawData.contains(keyCursor())) {
        m_rawData[keyCursor()] = m_cursor; // so we can easily access it in launcher & drkonqi
    }
}

Coredump::Coredump(const QJsonDocument &document)
    : Coredump(QByteArray() /* not from journal, has no cursor */, documentToHash(document))
{
}

QByteArray Coredump::keyFilename()
{
    return QByteArrayLiteral("COREDUMP_FILENAME");
}

Coredump::EntriesHash Coredump::documentToHash(const QJsonDocument &document)
{
    const QVariantMap variantMap = document.toVariant().toMap();
    EntriesHash hash;
    for (auto it = variantMap.cbegin(); it != variantMap.cend(); ++it) {
        hash.insert(it.key().toUtf8(), it->value<QByteArray>());
    }
    return hash;
}

QByteArray Coredump::keyPickup()
{
    return "_DRKONQI_PICKUP"_ba;
}

QByteArray Coredump::keyCursor()
{
    return "_DRKONQI_SD_CURSOR"_ba;
}
