/*
    SPDX-FileCopyrightText: 2011 John Layt <john@layt.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTIMECOMBOBOXTEST_H
#define KTIMECOMBOBOXTEST_H

#include <QWidget>

class KTimeComboBox;

class KTimeComboBoxTest : public QWidget
{
    Q_OBJECT

public:
    KTimeComboBoxTest();

private Q_SLOTS:
    void testDefaults();
    void testValidNull();
    void testTimeRange();
    void testTimeListInterval();
    void testTimeList();
    void testOptions();
    void testDisplayFormat();
    void testMask();
    void testEdit_data();
    void testEdit();

private:
    KTimeComboBox *m_combo;
};

#endif
