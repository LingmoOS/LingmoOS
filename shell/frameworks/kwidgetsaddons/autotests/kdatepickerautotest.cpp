/*
    SPDX-FileCopyrightText: 2022 g10 Code GmbH
    SPDX-FileContributor: Ingo Klöcker <dev@ingo-kloecker.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KDatePicker>

#include <QLineEdit>
#include <QSignalSpy>
#include <QTest>

#ifndef Q_OS_WIN
void initLocale()
{
    setenv("LC_ALL", "en_US.utf-8", 1);
}
Q_CONSTRUCTOR_FUNCTION(initLocale)
#endif

class KDatePickerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testSubmittingDateInLineEditEmitsOriginalDate()
    {
        const QDate originalDate{2022, 5, 15};
        KDatePicker p{originalDate};
        QSignalSpy spyDateEntered(&p, &KDatePicker::dateEntered);
        auto lineEdit = p.findChild<QLineEdit *>();
        QVERIFY(lineEdit);

        QTest::keyClick(lineEdit, Qt::Key_Return);
        QCOMPARE(spyDateEntered.count(), 1);
        QCOMPARE(spyDateEntered.first().at(0).toDate(), originalDate);
    }

    void testLineEditShowsDateWith4DigitYear()
    {
        KDatePicker p;
        auto lineEdit = p.findChild<QLineEdit *>();
        QVERIFY(lineEdit);

        p.setDate(QDate{2022, 5, 15});
        QCOMPARE(lineEdit->text(), QStringLiteral("5/15/2022"));
    }

    void testPickerAcceptsDatesEnteredInMultipleFormats()
    {
        KDatePicker p;
        QSignalSpy spyDateEntered(&p, &KDatePicker::dateEntered);
        auto lineEdit = p.findChild<QLineEdit *>();
        QVERIFY(lineEdit);

#ifndef Q_OS_WIN
        {
            // short date format
            lineEdit->setText(QStringLiteral("5/15/22"));
            QTest::keyClick(lineEdit, Qt::Key_Return);
            QCOMPARE(spyDateEntered.count(), 1);
            const auto emittedDate = spyDateEntered.first().at(0).toDate();
            const QDate expectedDate{2022, 5, 15};
            QCOMPARE(emittedDate.day(), expectedDate.day());
            QCOMPARE(emittedDate.month(), expectedDate.month());
            // we don't expect Qt to guess the correct century of a two digit year
            QCOMPARE(emittedDate.year() % 100, expectedDate.year() % 100);
            spyDateEntered.clear();
        }
#endif
        {
            // long date format
            lineEdit->setText(QStringLiteral("Sunday, May 15, 2022"));
            QTest::keyClick(lineEdit, Qt::Key_Return);
            QCOMPARE(spyDateEntered.count(), 1);
            const auto emittedDate = spyDateEntered.first().at(0).toDate();
            const QDate expectedDate{2022, 5, 15};
            QCOMPARE(emittedDate, expectedDate);
            spyDateEntered.clear();
        }
        {
            // short date format with 4-digit year
            lineEdit->setText(QStringLiteral("5/15/2022"));
            QTest::keyClick(lineEdit, Qt::Key_Return);
            QCOMPARE(spyDateEntered.count(), 1);
            const auto emittedDate = spyDateEntered.first().at(0).toDate();
            const QDate expectedDate{2022, 5, 15};
            QCOMPARE(emittedDate, expectedDate);
            spyDateEntered.clear();
        }
        {
            // ISO date format
            lineEdit->setText(QStringLiteral("2022-05-15"));
            QTest::keyClick(lineEdit, Qt::Key_Return);
            QCOMPARE(spyDateEntered.count(), 1);
            const auto emittedDate = spyDateEntered.first().at(0).toDate();
            const QDate expectedDate{2022, 5, 15};
            QCOMPARE(emittedDate, expectedDate);
            spyDateEntered.clear();
        }
    }
};

QTEST_MAIN(KDatePickerTest)

#include "kdatepickerautotest.moc"
