/*
    This file is part of the KDE frameworks
    SPDX-FileCopyrightText: 2016 Ragnar Thomsen <rthomsen6@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KCOLLAPSIBLEGROUPBOXTEST_H
#define KCOLLAPSIBLEGROUPBOXTEST_H

#include <QObject>

class KCollapsibleGroupBoxTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testDestructorCrash();
    void testOverrideFocus();
    void childShouldGetFocus();
};

#endif /* KCOLLAPSIBLEGROUPBOXTEST_H */
