/*
    SPDX-FileCopyrightText: 2011 John Layt <john@layt.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ktimecomboboxtest.h"

#include <QTime>

#include <QLineEdit>
#include <QTest>
#include <ktimecombobox.h>

QTEST_MAIN(KTimeComboBoxTest)

KTimeComboBoxTest::KTimeComboBoxTest()
{
    QLocale::setDefault(QLocale(QLocale::C));
}

void KTimeComboBoxTest::testDefaults()
{
    m_combo = new KTimeComboBox();
    QCOMPARE(m_combo->time(), QTime(0, 0, 0, 0));
    QCOMPARE(m_combo->minimumTime(), QTime(0, 0, 0, 0));
    QCOMPARE(m_combo->maximumTime(), QTime(23, 59, 59, 999));
    QCOMPARE(m_combo->isValid(), true);
    QCOMPARE(m_combo->options(), KTimeComboBox::EditTime | KTimeComboBox::SelectTime);
    QCOMPARE(m_combo->timeListInterval(), 15);
    QCOMPARE(m_combo->displayFormat(), QLocale::ShortFormat);
    delete m_combo;
}

void KTimeComboBoxTest::testValidNull()
{
    m_combo = new KTimeComboBox(nullptr);
    QCOMPARE(m_combo->isValid(), true);
    QCOMPARE(m_combo->isNull(), false);
    m_combo->setTime(QTime());
    QCOMPARE(m_combo->isValid(), false);
    QCOMPARE(m_combo->isNull(), true);
    m_combo->setTime(QTime(0, 0, 0));
    m_combo->lineEdit()->setText(QStringLiteral("99:99"));
    QCOMPARE(m_combo->isValid(), false);
    QCOMPARE(m_combo->isNull(), false);
    delete m_combo;
}

void KTimeComboBoxTest::testTimeRange()
{
    m_combo = new KTimeComboBox();
    m_combo->setTime(QTime(2, 0, 0, 0));
    QCOMPARE(m_combo->minimumTime(), QTime(0, 0, 0, 0));
    QCOMPARE(m_combo->maximumTime(), QTime(23, 59, 59, 999));
    QCOMPARE(m_combo->isValid(), true);

    m_combo->setTimeRange(QTime(3, 0, 0, 0), QTime(22, 0, 0, 0));
    QCOMPARE(m_combo->minimumTime(), QTime(3, 0, 0, 0));
    QCOMPARE(m_combo->maximumTime(), QTime(22, 0, 0, 0));
    QCOMPARE(m_combo->isValid(), false);

    m_combo->setTime(QTime(23, 0, 0, 0));
    QCOMPARE(m_combo->isValid(), false);
    m_combo->setTime(QTime(12, 0, 0, 0));
    QCOMPARE(m_combo->isValid(), true);
    m_combo->setTime(QTime(3, 0, 0, 0));
    QCOMPARE(m_combo->isValid(), true);
    m_combo->setTime(QTime(22, 0, 0, 0));
    QCOMPARE(m_combo->isValid(), true);
    m_combo->setTime(QTime(2, 59, 59, 999));
    QCOMPARE(m_combo->isValid(), false);
    m_combo->setTime(QTime(22, 1, 0, 0));
    QCOMPARE(m_combo->isValid(), false);

    m_combo->setTimeRange(QTime(15, 0, 0, 0), QTime(5, 0, 0, 0));
    QCOMPARE(m_combo->minimumTime(), QTime(3, 0, 0, 0));
    QCOMPARE(m_combo->maximumTime(), QTime(22, 0, 0, 0));

    m_combo->setMinimumTime(QTime(2, 0, 0, 0));
    QCOMPARE(m_combo->minimumTime(), QTime(2, 0, 0, 0));
    QCOMPARE(m_combo->maximumTime(), QTime(22, 0, 0, 0));

    m_combo->setMaximumTime(QTime(21, 0, 0, 0));
    QCOMPARE(m_combo->minimumTime(), QTime(2, 0, 0, 0));
    QCOMPARE(m_combo->maximumTime(), QTime(21, 0, 0, 0));

    delete m_combo;
}

void KTimeComboBoxTest::testTimeListInterval()
{
    m_combo = new KTimeComboBox();
    QCOMPARE(m_combo->timeListInterval(), 15);
    m_combo->setTimeListInterval(60);
    QCOMPARE(m_combo->timeListInterval(), 60);
    m_combo->setTimeListInterval(7);
    QCOMPARE(m_combo->timeListInterval(), 60);
    m_combo->setTimeListInterval(720);
    QCOMPARE(m_combo->timeListInterval(), 720);
    QList<QTime> list;
    list << QTime(0, 0, 0) << QTime(12, 0, 0) << QTime(23, 59, 59, 999);
    QCOMPARE(m_combo->timeList(), list);
    m_combo->setTimeRange(QTime(4, 0, 0, 0), QTime(5, 0, 0, 0));
    m_combo->setTimeListInterval(30);
    list.clear();
    list << QTime(4, 0, 0) << QTime(4, 30, 0) << QTime(5, 0, 0, 0);
    QCOMPARE(m_combo->timeList(), list);
    m_combo->setTimeRange(QTime(4, 0, 0, 0), QTime(4, 59, 0, 0));
    m_combo->setTimeListInterval(30);
    list.clear();
    list << QTime(4, 0, 0) << QTime(4, 30, 0) << QTime(4, 59, 0, 0);
    QCOMPARE(m_combo->timeList(), list);
    delete m_combo;
}

void KTimeComboBoxTest::testTimeList()
{
    m_combo = new KTimeComboBox();
    QList<QTime> list;

    // Test default list
    QTime thisTime = QTime(0, 0, 0);
    for (int i = 0; i < 1440; i = i + 15) {
        list << thisTime.addSecs(i * 60);
    }
    list << QTime(23, 59, 59, 999);
    QCOMPARE(m_combo->timeList(), list);

    // Test basic list
    list.clear();
    list << QTime(3, 0, 0) << QTime(15, 16, 17);
    m_combo->setTimeList(list);
    QCOMPARE(m_combo->timeList(), list);
    QCOMPARE(m_combo->minimumTime(), QTime(3, 0, 0));
    QCOMPARE(m_combo->maximumTime(), QTime(15, 16, 17));

    // Test sort input times
    list.clear();
    list << QTime(17, 16, 15) << QTime(4, 0, 0);
    m_combo->setTimeList(list);
    std::sort(list.begin(), list.end());
    QCOMPARE(m_combo->timeList(), list);
    QCOMPARE(m_combo->minimumTime(), QTime(4, 0, 0));
    QCOMPARE(m_combo->maximumTime(), QTime(17, 16, 15));

    // Test ignore null QTime
    list.clear();
    list << QTime(3, 0, 0) << QTime(15, 16, 17) << QTime();
    m_combo->setTimeList(list);
    list.takeLast();
    QCOMPARE(m_combo->timeList(), list);
    QCOMPARE(m_combo->minimumTime(), QTime(3, 0, 0));
    QCOMPARE(m_combo->maximumTime(), QTime(15, 16, 17));
    delete m_combo;
}

void KTimeComboBoxTest::testOptions()
{
    m_combo = new KTimeComboBox();
    KTimeComboBox::Options options = KTimeComboBox::EditTime | KTimeComboBox::SelectTime;
    QCOMPARE(m_combo->options(), options);
    options = KTimeComboBox::EditTime | KTimeComboBox::WarnOnInvalid;
    m_combo->setOptions(options);
    QCOMPARE(m_combo->options(), options);
    delete m_combo;
}

void KTimeComboBoxTest::testDisplayFormat()
{
    m_combo = new KTimeComboBox();
    QLocale::FormatType format = QLocale::ShortFormat;
    QCOMPARE(m_combo->displayFormat(), format);
    format = QLocale::NarrowFormat;
    m_combo->setDisplayFormat(format);
    QCOMPARE(m_combo->displayFormat(), format);
    delete m_combo;
}

void KTimeComboBoxTest::testMask()
{
    // Store the current locale, and set to one which reproduces the bug.
    QLocale currentLocale;

    // Test that the line edit input mask AM/PM portion gets correctly
    // replaced with xx.
    QLocale::setDefault(QLocale(QLocale::English, QLocale::Australia));
    m_combo = new KTimeComboBox(nullptr);
    QString mask = m_combo->lineEdit()->inputMask();
    QVERIFY(mask.contains(QLatin1String("xx")));
    delete m_combo;

    // For 24 hour time formats, no am/pm specifier in mask
    QLocale::setDefault(QLocale(QLocale::NorwegianBokmal, QLocale::Norway));
    m_combo = new KTimeComboBox(nullptr);
    mask = m_combo->lineEdit()->inputMask();
    QVERIFY(!mask.contains(QLatin1String("xx")));
    delete m_combo;

    // Restore the previous locale
    QLocale::setDefault(currentLocale);
}

void KTimeComboBoxTest::testEdit_data()
{
    QTest::addColumn<QString>("locale");

    QTest::newRow("C") << QStringLiteral("C");
    QTest::newRow("el_GR") << QStringLiteral("el_GR"); // bug 361764; non-ASCII AM/PM
    QTest::newRow("en_AU") << QStringLiteral("en_AU"); // bug 361764
    QTest::newRow("en_CA") << QStringLiteral("en_CA"); // bug 405857
    QTest::newRow("en_IE") << QStringLiteral("en_IE"); // bug 361764
    QTest::newRow("en_US") << QStringLiteral("en_US");
    QTest::newRow("fr_CA") << QStringLiteral("fr_CA"); // bug 409912
    QTest::newRow("ko_KR") << QStringLiteral("ko_KR"); // non-ASCII AM/PM
}

static const QTime MIDNIGHT = QTime(00, 00, 00);
static const QTime END_AM = QTime(11, 59, 00);
static const QTime NOON = QTime(12, 00, 00);
static const QTime END_PM = QTime(23, 59, 00);

// Test that the widget can process times using the current locale and edit mask.
// See https://bugs.kde.org/show_bug.cgi?id=409867
void KTimeComboBoxTest::testEdit()
{
    QLocale currentLocale;

    QFETCH(QString, locale);

    QLocale::setDefault(QLocale(locale));
    m_combo = new KTimeComboBox(nullptr);

    m_combo->setTime(MIDNIGHT);
    QCOMPARE(m_combo->time(), MIDNIGHT);
    m_combo->setTime(END_AM);
    QCOMPARE(m_combo->time(), END_AM);
    m_combo->setTime(NOON);
    QCOMPARE(m_combo->time(), NOON);
    m_combo->setTime(END_PM);
    QCOMPARE(m_combo->time(), END_PM);

    delete m_combo;

    QLocale::setDefault(currentLocale);
}

#include "moc_ktimecomboboxtest.cpp"
