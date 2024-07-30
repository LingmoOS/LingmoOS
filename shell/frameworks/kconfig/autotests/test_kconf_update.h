/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010 Canonical Ltd
    SPDX-FileContributor: Aurélien Gâteau <aurelien.gateau@canonical.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef TEST_KCONF_UPDATE_H
#define TEST_KCONF_UPDATE_H

#include <QObject>

class TestKConfUpdate : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testScript_data();
    void testScript();
};

#endif /* TEST_KCONF_UPDATE_H */
