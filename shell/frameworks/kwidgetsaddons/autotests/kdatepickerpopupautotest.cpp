/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KDatePickerPopup>

#include <QSignalSpy>
#include <QTest>

class KDatePickerPopupTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testWordActions()
    {
        KDatePickerPopup p(KDatePickerPopup::DatePicker | KDatePickerPopup::Words | KDatePickerPopup::NoDate);
        // picker, 9 words, no date, 2 separators
        p.popup(QPoint());
        QCOMPARE(p.actions().count(), 13);
        p.hide();
        p.popup(QPoint());
        QCOMPARE(p.actions().count(), 13);

        QCOMPARE(p.actions()[2]->data().toDate(), QDate::currentDate().addYears(1));
        QCOMPARE(p.actions()[6]->data().toDate(), QDate::currentDate());
        QCOMPARE(p.actions()[12]->data().toDate(), QDate());
    }

    void testModes()
    {
        KDatePickerPopup p;
        QCOMPARE(p.modes(), KDatePickerPopup::DatePicker);
        p.popup(QPoint());
        QCOMPARE(p.actions().count(), 1); // only the date picker
        p.hide();

        p.setModes(KDatePickerPopup::NoDate);
        p.popup(QPoint());
        QCOMPARE(p.actions().count(), 1);
        p.hide();

        p.setModes(KDatePickerPopup::Words);
        p.popup(QPoint());
        QCOMPARE(p.actions().count(), 9);
        p.hide();

        p.setModes(KDatePickerPopup::DatePicker | KDatePickerPopup::NoDate);
        p.popup(QPoint());
        QCOMPARE(p.actions().count(), 3);
        p.hide();

        p.setModes(KDatePickerPopup::Words | KDatePickerPopup::NoDate);
        p.popup(QPoint());
        QCOMPARE(p.actions().count(), 11);
        p.hide();
    }

    void testDateRange()
    {
        KDatePickerPopup p(KDatePickerPopup::Words);
        const auto today = QDate::currentDate();

        p.setDateRange(today.addDays(-1), today.addDays(1));
        p.popup(QPoint());
        QCOMPARE(p.actions().count(), 3);
        QCOMPARE(p.actions()[1]->data().toDate(), today);
        p.hide();

        p.setDateRange(QDate(), today.addDays(1));
        p.popup(QPoint());
        QCOMPARE(p.actions().count(), 6);
        QCOMPARE(p.actions()[1]->data().toDate(), today);
        p.hide();

        p.setDateRange(today.addDays(-2), {});
        p.popup(QPoint());
        QCOMPARE(p.actions().count(), 6);
        QCOMPARE(p.actions()[4]->data().toDate(), today);
        p.hide();

        p.setDateRange({}, {});
        p.popup(QPoint());
        QCOMPARE(p.actions().count(), 9);
        p.hide();
    }

    void testDateMap()
    {
        KDatePickerPopup p(KDatePickerPopup::Words);

        QMap<QDate, QString> m;
        m.insert(QDate(1996, 10, 14), QStringLiteral("KDE's birthday"));
        m.insert(QDate(1997, 1, 1), QStringLiteral("separator"));
        m.insert(QDate(2017, 9, 10), {});

        p.setDateMap(m);
        p.popup(QPoint());
        QCOMPARE(p.actions().count(), 3);
        QCOMPARE(p.actions()[0]->data().toDate(), QDate(1996, 10, 14));
        QCOMPARE(p.actions()[0]->text(), QStringLiteral("KDE's birthday"));
        QCOMPARE(p.actions()[1]->data().toDate(), QDate());
        QCOMPARE(p.actions()[1]->isSeparator(), true);
        QCOMPARE(p.actions()[2]->data().toDate(), QDate(2017, 9, 10));
        QCOMPARE(p.actions()[2]->text().isEmpty(), false);
        p.hide();

        p.setDateMap({});
        p.popup(QPoint());
        QCOMPARE(p.actions().count(), 9);
        p.hide();
    }
};

QTEST_MAIN(KDatePickerPopupTest)

#include "kdatepickerpopupautotest.moc"
