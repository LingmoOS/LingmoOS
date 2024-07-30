/*
    SPDX-FileCopyrightText: 2022 Waqar Ahmed <waqar.17a@gmail.com>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef INDENT_DETECT_TEST_H
#define INDENT_DETECT_TEST_H

#include <QObject>

class IndentDetectTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void test_data();
    void test();
    void bench();
};

#endif
