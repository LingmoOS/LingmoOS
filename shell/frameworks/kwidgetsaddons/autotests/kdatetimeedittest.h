/*
    SPDX-FileCopyrightText: 2011 John Layt <john@layt.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDATETIMEEDITTEST_H
#define KDATETIMEEDITTEST_H

#include <QWidget>

class KDateTimeEdit;

class KDateTimeEditTest : public QWidget
{
    Q_OBJECT

private Q_SLOTS:
    void testDefaults();
    void testValidNull();
    void testDateTimeRange();
    void testOptions();
    void testDateDisplayFormat();
    void testDateList();
    void testTimeListInterval();
    void testTimeList();
    void testTimeDisplayFormat();
    void testCalendarSystem();
    void testTimeSpec();
    void testDateMenu();

private:
    KDateTimeEdit *m_edit;
};

#endif // KDATECOMBOBOXTEST_H
