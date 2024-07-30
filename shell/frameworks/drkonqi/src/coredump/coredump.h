/*
    SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
    SPDX-FileCopyrightText: 2019-2022 Harald Sitter <sitter@kde.org>
*/

#pragma once

#include <QByteArray>
#include <QHash>
#include <QJsonDocument>
#include <QString>

#include "memory.h"

class Coredump
{
public:
    using EntriesHash = QHash<QByteArray, QByteArray>;

    Coredump(QByteArray cursor, EntriesHash data);
    explicit Coredump(const QJsonDocument &document);

    ~Coredump() = default;

    // In a function cause it is used in more than one location.
    static QByteArray keyFilename();
    static QByteArray keyPickup();
    static QByteArray keyCursor();

    // Other bits and bobs
    QByteArray m_cursor;
    EntriesHash m_rawData;

    // Journal Entry values
    uid_t uid = 0;
    pid_t pid = -1;
    QString exe;
    QString filename; // core dump file if available (may be /dev/null if the core is stored in journal directly - only in older systemds)
    QString systemd_unit;
    QString bootId;
    QString timestamp;

private:
    static EntriesHash documentToHash(const QJsonDocument &document);
    Q_DISABLE_COPY_MOVE(Coredump)
};
