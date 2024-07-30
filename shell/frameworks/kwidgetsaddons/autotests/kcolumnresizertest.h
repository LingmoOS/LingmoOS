/*
    This file is part of the KDE frameworks
    SPDX-FileCopyrightText: 2014 Aurélien Gâteau <agateau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/
#ifndef KCOLUMNRESIZERTEST_H
#define KCOLUMNRESIZERTEST_H

#include <QObject>

class KColumnResizerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void test_data();
    void test();
};

#endif /* KCOLUMNRESIZERTEST_H */
