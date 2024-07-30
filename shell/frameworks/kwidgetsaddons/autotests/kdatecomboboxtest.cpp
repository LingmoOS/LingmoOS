/*
    SPDX-FileCopyrightText: 2011 John Layt <john@layt.net>
    SPDX-FileCopyrightText: 2022 g10 Code GmbH
    SPDX-FileContributor: Ingo Klöcker <dev@ingo-kloecker.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kdatecomboboxtest.h"

#include <QDate>

#include "kdatecombobox.h"
#include "kdatepickerpopup.h"

#include <QLineEdit>
#include <QSignalSpy>
#include <QTest>

QTEST_MAIN(KDateComboBoxTest)

#ifndef Q_OS_WIN
void initLocale()
{
    setenv("LC_ALL", "en_US.utf-8", 1);
}
Q_CONSTRUCTOR_FUNCTION(initLocale)
#endif

void KDateComboBoxTest::testDefaults()
{
    KDateComboBox combo;
    QCOMPARE(combo.date(), QDate::currentDate());
    // Missing support in QLocale;
    // QCOMPARE(m_combo.minimumDate(), KLocale::global()->calendar()->earliestValidDate());
    // QCOMPARE(m_combo.maximumDate(), KLocale::global()->calendar()->latestValidDate());
    QCOMPARE(combo.isValid(), true);
    QCOMPARE(combo.isNull(), false);
    QCOMPARE(combo.options(), KDateComboBox::EditDate | KDateComboBox::SelectDate | KDateComboBox::DatePicker | KDateComboBox::DateKeywords);
    QCOMPARE(combo.displayFormat(), QLocale::ShortFormat);
}

void KDateComboBoxTest::testValidNull()
{
    KDateComboBox combo;
    QCOMPARE(combo.isValid(), true);
    QCOMPARE(combo.isNull(), false);
    combo.setDate(QDate());
    QCOMPARE(combo.isValid(), false);
    QCOMPARE(combo.isNull(), true);
    combo.setDate(QDate(2000, 1, 1));
    combo.lineEdit()->setText(QStringLiteral("invalid"));
    QCOMPARE(combo.isValid(), false);
    QCOMPARE(combo.isNull(), false);
}

void KDateComboBoxTest::testDateRange()
{
    KDateComboBox combo;
    combo.setDate(QDate(2000, 1, 1));
    // Missing support in QLocale;
    // QCOMPARE(m_combo.minimumDate(), KLocale::global()->calendar()->earliestValidDate());
    // QCOMPARE(m_combo.maximumDate(), KLocale::global()->calendar()->latestValidDate());
    QCOMPARE(combo.isValid(), true);

    combo.setDateRange(QDate(2001, 1, 1), QDate(2002, 1, 1));
    QCOMPARE(combo.minimumDate(), QDate(2001, 1, 1));
    QCOMPARE(combo.maximumDate(), QDate(2002, 1, 1));
    QCOMPARE(combo.isValid(), false);

    combo.setDate(QDate(2003, 1, 1));
    QCOMPARE(combo.isValid(), false);
    combo.setDate(QDate(2001, 6, 1));
    QCOMPARE(combo.isValid(), true);
    combo.setDate(QDate(2001, 1, 1));
    QCOMPARE(combo.isValid(), true);
    combo.setDate(QDate(2002, 1, 1));
    QCOMPARE(combo.isValid(), true);
    combo.setDate(QDate(2000, 12, 31));
    QCOMPARE(combo.isValid(), false);
    combo.setDate(QDate(2002, 1, 2));
    QCOMPARE(combo.isValid(), false);

    combo.setDateRange(QDate(1995, 1, 1), QDate(1990, 1, 1));
    QCOMPARE(combo.minimumDate(), QDate(2001, 1, 1));
    QCOMPARE(combo.maximumDate(), QDate(2002, 1, 1));

    combo.setMinimumDate(QDate(2000, 1, 1));
    QCOMPARE(combo.minimumDate(), QDate(2000, 1, 1));
    QCOMPARE(combo.maximumDate(), QDate(2002, 1, 1));

    combo.setMaximumDate(QDate(2003, 1, 1));
    QCOMPARE(combo.minimumDate(), QDate(2000, 1, 1));
    QCOMPARE(combo.maximumDate(), QDate(2003, 1, 1));

    combo.resetDateRange();
    QVERIFY(!combo.minimumDate().isValid());
    QVERIFY(!combo.maximumDate().isValid());

    // Check functioning when the minimum or maximum date is not already set

    combo.setMinimumDate(QDate(2000, 1, 1));
    QCOMPARE(combo.minimumDate(), QDate(2000, 1, 1));
    QVERIFY(!combo.maximumDate().isValid());

    combo.resetMinimumDate();
    QVERIFY(!combo.minimumDate().isValid());
    QVERIFY(!combo.maximumDate().isValid());

    combo.setMaximumDate(QDate(2003, 1, 1));
    QVERIFY(!combo.minimumDate().isValid());
    QCOMPARE(combo.maximumDate(), QDate(2003, 1, 1));

    combo.resetMaximumDate();
    QVERIFY(!combo.minimumDate().isValid());
    QVERIFY(!combo.maximumDate().isValid());
}

void KDateComboBoxTest::testDateList()
{
    KDateComboBox combo;
    QMap<QDate, QString> map;

    // Test default map
    QCOMPARE(combo.dateMap(), map);

    // Test basic map
    map.clear();
    map.insert(QDate(2000, 1, 1), QStringLiteral("New Years Day"));
    map.insert(QDate(2000, 1, 2), QString());
    map.insert(QDate(2000, 1, 3), QStringLiteral("separator"));
    map.insert(QDate(), QStringLiteral("No Date"));
    combo.setDateMap(map);
    QCOMPARE(combo.dateMap(), map);
}

void KDateComboBoxTest::testOptions()
{
    KDateComboBox combo;
    auto datePicker = combo.findChild<KDatePickerPopup *>();
    QVERIFY(datePicker);
    KDateComboBox::Options options = KDateComboBox::EditDate | KDateComboBox::SelectDate | KDateComboBox::DatePicker | KDateComboBox::DateKeywords;
    QCOMPARE(combo.options(), options);
    QCOMPARE(datePicker->modes(), KDatePickerPopup::DatePicker | KDatePickerPopup::Words);
    options = KDateComboBox::EditDate | KDateComboBox::WarnOnInvalid;
    combo.setOptions(options);
    QCOMPARE(combo.options(), options);
    QCOMPARE(datePicker->modes(), 0);
}

void KDateComboBoxTest::testDisplayFormat()
{
    KDateComboBox combo;
    QLocale::FormatType format = QLocale::ShortFormat;
    QCOMPARE(combo.displayFormat(), format);
    format = QLocale::NarrowFormat;
    combo.setDisplayFormat(format);
    QCOMPARE(combo.displayFormat(), format);
}

void KDateComboBoxTest::testSignals()
{
    // GIVEN
    KDateComboBox combo;
    QSignalSpy spyDateEntered(&combo, &KDateComboBox::dateEntered);
    QSignalSpy spyDateEdited(&combo, &KDateComboBox::dateEdited);
    QSignalSpy spyDateChanged(&combo, &KDateComboBox::dateChanged);
    auto clearSpies = [&]() {
        spyDateEntered.clear();
        spyDateEdited.clear();
        spyDateChanged.clear();
    };

    // WHEN setting a date programmatically
    combo.setDate(QDate(2016, 05, 30));

    // THEN
    QCOMPARE(spyDateEntered.count(), 0);
    QCOMPARE(spyDateEdited.count(), 0);
    QCOMPARE(spyDateChanged.count(), 1);
    clearSpies();

    combo.show();
    combo.activateWindow();
    QVERIFY(QTest::qWaitForWindowActive(&combo));
    QCOMPARE(combo.currentText(), QStringLiteral("5/30/2016"));
    QCOMPARE(combo.date(), QDate(2016, 5, 30));

    // WHEN typing a new day (Home, Del, '4' -> 30 April 2016)
    QTest::keyClick(&combo, Qt::Key_Home);
    QTest::keyClick(&combo, Qt::Key_Delete);
    QTest::keyClick(&combo, Qt::Key_4);

    // THEN
    QCOMPARE(combo.currentText(), QStringLiteral("4/30/2016"));
    QCOMPARE(combo.date(), QDate(2016, 4, 30));

    // and losing focus
    combo.clearFocus();

    // THEN
    QCOMPARE(spyDateEdited.count(), 2);
    QCOMPARE(spyDateEntered.count(), 0);
    QCOMPARE(spyDateChanged.count(), 1);
    clearSpies();

    // WHEN typing Key_Up
    QTest::keyClick(&combo, Qt::Key_Up);

    // THEN
    QCOMPARE(combo.currentText(), QStringLiteral("5/1/2016"));
    QCOMPARE(combo.date(), QDate(2016, 5, 1));
    QCOMPARE(spyDateEdited.count(), 0);
    QCOMPARE(spyDateEntered.count(), 1);
    QCOMPARE(spyDateChanged.count(), 1);
    clearSpies();

    // WHEN typing a digit ('6' -> 1 june 2016) then Return
    QTest::keyClick(&combo, Qt::Key_Home);
    QTest::keyClick(&combo, Qt::Key_Delete);
    QTest::keyClick(&combo, Qt::Key_6);
    QTest::keyClick(&combo, Qt::Key_Return);

    // THEN
    QCOMPARE(combo.currentText(), QStringLiteral("6/1/2016"));
    QCOMPARE(combo.date(), QDate(2016, 6, 1));
    QCOMPARE(spyDateEdited.count(), 2);
    QCOMPARE(spyDateEntered.count(), 1);
    QCOMPARE(spyDateChanged.count(), 1);
    clearSpies();
}

void KDateComboBoxTest::testDatePickerIntegration()
{
    // GIVEN
    KDateComboBox combo;
    const QDate minimumDate{2001, 1, 1};
    const QDate maximumDate{2106, 1, 1};
    combo.setDateRange(minimumDate, maximumDate);
    const QDate originalDate{2021, 6, 23};
    combo.setDate(originalDate);

    // WHEN the date picker emits dateChanged signal with an invalid date
    auto datePicker = combo.findChild<KDatePickerPopup *>();
    QVERIFY(datePicker);
    const QDate dateOutsideTheAllowedRange{1921, 6, 23};
    Q_EMIT datePicker->dateChanged(dateOutsideTheAllowedRange);

    // THEN the combo still has the original date
    QCOMPARE(combo.date(), originalDate);
}

#include "moc_kdatecomboboxtest.cpp"
