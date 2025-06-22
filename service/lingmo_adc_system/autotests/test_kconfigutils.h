/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010 Canonical Ltd
    SPDX-FileContributor: Aurélien Gâteau <aurelien.gateau@canonical.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef TEST_KCONFIGUTILS_H
#define TEST_KCONFIGUTILS_H

#include <QObject>

class TestKConfigUtils : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testParseGroupString_data();
    void testParseGroupString();
    void testUnescapeString_data();
    void testUnescapeString();
};

#endif /* TEST_KCONFIGUTILS_H */
