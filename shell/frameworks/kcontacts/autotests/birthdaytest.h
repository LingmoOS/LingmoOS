/*
    This file is part of kcontacts.
    SPDX-FileCopyrightText: 2016-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef BIRTHDAYTEST_H
#define BIRTHDAYTEST_H

#include <QObject>

class BirthDayTest : public QObject
{
    Q_OBJECT
public:
    explicit BirthDayTest(QObject *parent = nullptr);
    ~BirthDayTest() override;

private Q_SLOTS:
    void shouldExportVcard4();
    void shouldExportVcard4WithoutTime();
    void shouldExportVcard3();
    void shouldExportVcard3WithoutTime();
    void shouldParseBirthDay();
    void shouldParseBirthDayWithoutTime();
    void shouldParseBirthDayWithoutTimeAndYear();
    void shouldExportVcard4WithoutTimeAndWithoutYear();
    void shouldExportVcard3WithoutTimeAndWithoutYear();
};

#endif // BIRTHDAYTEST_H
