/*
    SPDX-FileCopyrightText: 2011 John Layt <john@layt.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDATECOMBOBOXTEST_H
#define KDATECOMBOBOXTEST_H

#include <QWidget>

class KDateComboBox;

class KDateComboBoxTest : public QWidget
{
    Q_OBJECT

private Q_SLOTS:
    void testDefaults();
    void testValidNull();
    void testDateRange();
    void testDateList();
    void testOptions();
    void testDisplayFormat();
    void testSignals();
    void testDatePickerIntegration();
};

#endif // KDATECOMBOBOXTEST_H
