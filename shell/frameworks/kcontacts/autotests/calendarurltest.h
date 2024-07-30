/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2015-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CALENDARURLTEST_H
#define CALENDARURLTEST_H

#include <QObject>

class CalendarUrlTest : public QObject
{
    Q_OBJECT
public:
    explicit CalendarUrlTest(QObject *parent = nullptr);
    ~CalendarUrlTest() override;

private Q_SLOTS:
    void shouldHaveDefaultValue();
    void shouldAssignValue();
    void shouldAssignExternal();
    void shouldSerialized();
    void shouldEqualCalendarUrl();
    void shouldParseCalendarUrl();
    void shouldGenerateVCard_data();
    void shouldGenerateVCard();
    void shouldGenerateVCardWithParameter();
    void shouldNotGeneratedAttributeForVcard3();
    void shouldGenerateMultiCalendarUrl();
};

#endif // CALENDARURLTEST_H
