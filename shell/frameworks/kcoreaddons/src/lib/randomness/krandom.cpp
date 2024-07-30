/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 1999 Matthias Kalle Dalheimer <kalle@kde.org>
    SPDX-FileCopyrightText: 2000 Charles Samuels <charles@kde.org>
    SPDX-FileCopyrightText: 2005 Joseph Wenninger <kde@jowenn.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "krandom.h"

#include <stdlib.h>
#ifdef Q_OS_WIN
#include <process.h>
#else // Q_OS_WIN
#include <unistd.h>
#endif // Q_OS_WIN
#include <stdio.h>
#include <time.h>
#ifndef Q_OS_WIN
#include <sys/time.h>
#endif //  Q_OS_WIN
#include <fcntl.h>

#include <QFile>
#include <QThread>
#include <QThreadStorage>

QString KRandom::randomString(int length)
{
    if (length <= 0) {
        return QString();
    }

    QString str;
    str.resize(length);
    int i = 0;
    while (length--) {
        int r = QRandomGenerator::global()->bounded(62);
        r += 48;
        if (r > 57) {
            r += 7;
        }
        if (r > 90) {
            r += 6;
        }
        str[i++] = QLatin1Char(char(r));
        // so what if I work backwards?
    }
    return str;
}
