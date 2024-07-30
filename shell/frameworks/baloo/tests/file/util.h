/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BALOO_FILE_TEST_UTIL_H
#define BALOO_FILE_TEST_UTIL_H

#include <QDebug>
#include <QFile>

inline void printIOUsage()
{
    // Print the io usage
    QFile file(QStringLiteral("/proc/self/io"));
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream fs(&file);
    QString fileContents = fs.readAll();

    qDebug() << "------- IO ---------";
    QTextStream stream(&fileContents);
    while (!stream.atEnd()) {
        const QString line = stream.readLine();

        const QStringView str(line);

        const QString rchar(QStringLiteral("rchar: "));
        if (str.startsWith(rchar)) {
            const ulong amount = str.mid(rchar.size()).toULong();
            qDebug() << "Read:" << amount / 1024  << "kb";
        }

        const QString wchar(QStringLiteral("wchar: "));
        if (str.startsWith(wchar)) {
            const ulong amount = str.mid(wchar.size()).toULong();
            qDebug() << "Write:" << amount / 1024  << "kb";
        }

        const QString read(QStringLiteral("read_bytes: "));
        if (str.startsWith(read)) {
            const ulong amount = str.mid(read.size()).toULong();
            qDebug() << "Actual Reads:" << amount / 1024  << "kb";
        }

        const QString write(QStringLiteral("write_bytes: "));
        if (str.startsWith(write)) {
            const ulong amount = str.mid(write.size()).toULong();
            qDebug() << "Actual Writes:" << amount / 1024  << "kb";
        }
    }
    qDebug() << "\nThe actual read/writes may be 0 because of an existing"
             << "cache and /tmp being memory mapped";
}

#endif
