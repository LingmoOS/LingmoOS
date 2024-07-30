/*
    SPDX-FileCopyrightText: 2011 Romain Perier <bambi@kubuntu.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KCHARSETSTEST_H
#define KCHARSETSTEST_H

#include <QObject>

class KCharsetsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testSingleton();
    void testFromEntity();
    void testToEntity();
    void testResolveEntities();
    void testEncodingNames();
};

#endif /* KCHARSETSTEST_H */
