/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 1999 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include <QList>
#include <QString>

#include "krandom.h"
#include "krandomsequence.h"

#include <stdio.h>

int main(/*int argc, char *argv[]*/)
{
    long seed;
    KRandomSequence seq;

    seed = 2;
    seq.setSeed(seed);
    printf("Seed = %4ld :", seed);
    for (int i = 0; i < 20; i++) {
        printf("%3ld ", seq.getLong(100));
    }
    printf("\n");

    seed = 0;
    seq.setSeed(seed);
    printf("Seed = %4ld :", seed);
    for (int i = 0; i < 20; i++) {
        printf("%3ld ", seq.getLong(100));
    }
    printf("\n");

    seed = 0;
    seq.setSeed(seed);
    printf("Seed = %4ld :", seed);
    for (int i = 0; i < 20; i++) {
        printf("%3ld ", seq.getLong(100));
    }
    printf("\n");

    seed = 2;
    seq.setSeed(seed);
    printf("Seed = %4ld :", seed);
    for (int i = 0; i < 20; i++) {
        printf("%3ld ", seq.getLong(100));
    }
    printf("\n");

    seq.setSeed(KRandom::random());

    QStringList list{
        QLatin1String("A"),
        QLatin1String("B"),
        QLatin1String("C"),
        QLatin1String("D"),
        QLatin1String("E"),
        QLatin1String("F"),
        QLatin1String("G"),
    };

    auto printList = [&list]() {
        for (const QString &str : std::as_const(list)) {
            printf("%s", str.toLatin1().data());
        }
        printf("\n");
    };

    printList();

    seq.randomize(list);
    printList();

    seq.randomize(list);
    printList();

    seq.randomize(list);
    printList();
}
