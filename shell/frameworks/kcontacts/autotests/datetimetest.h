/*
    This file is part of kcontacts.
    SPDX-FileCopyrightText: 2016 Benjamin Löwe <benni@mytum.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef DATETIMETEST_H
#define DATETIMETEST_H

#include <QObject>

class DateTimeTest : public QObject
{
    Q_OBJECT
public:
    explicit DateTimeTest(QObject *parent = nullptr);
    ~DateTimeTest() override;

private Q_SLOTS:
    void shouldParseDateTime();
    void shouldCreateDateTime();
    void shouldCreateDate();
};

#endif // DATETIMETEST_H
