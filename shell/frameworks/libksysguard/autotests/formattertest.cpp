/*
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QLocale>
#include <QTest>

#include <unistd.h>

#include "Formatter.h"
#include "Unit.h"

#define QSL QStringLiteral

class FormatterTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        // Ensure we use a known locale for the test.
        QLocale::setDefault(QLocale{QLocale::English, QLocale::UnitedStates});
    }

    void testDouble_data()
    {
        QTest::addColumn<double>("input");
        QTest::addColumn<KSysGuard::Unit>("unit");
        QTest::addColumn<QString>("output");

        QTest::newRow("1.0, B") << 1.0 << KSysGuard::UnitByte << QSL("1.0\u2009B");
        QTest::newRow("1.0, KiB") << 1.0 << KSysGuard::UnitKiloByte << QSL("1.0\u202FKi\u200BB");
        QTest::newRow("1.0, KiB/s") << 1.0 << KSysGuard::UnitKiloByteRate << QSL("1.0\u202FKi\u200BB/s");
        QTest::newRow("1.0, %") << 1.0 << KSysGuard::UnitPercent << QSL("1.0\u200B%");

        QTest::newRow("0.213, B") << 0.213 << KSysGuard::UnitByte << QString::number(0.2) + QSL("\u2009B");
        QTest::newRow("5.647, KiB") << 5.647 << KSysGuard::UnitKiloByte << QString::number(5.6) + QSL("\u202FKi\u200BB");
        QTest::newRow("99.99, KiB/s") << 99.99 << KSysGuard::UnitKiloByteRate << QString::number(100.0, 'f', 1) + QSL("\u202FKi\u200BB/s");
        QTest::newRow("0.2567, %") << 0.2567 << KSysGuard::UnitPercent << QString::number(0.3) + QSL("\u200B%");
    }

    void testDouble()
    {
        QFETCH(double, input);
        QFETCH(KSysGuard::Unit, unit);
        QFETCH(QString, output);

        auto formatted = KSysGuard::Formatter::formatValue(input, unit);
        QCOMPARE(formatted, output);
    }

    void testFormatTime_data()
    {
        QTest::addColumn<int>("input");
        QTest::addColumn<QString>("output");
        QTest::newRow("1 s") << 1 << QSL("0:00:01");
        QTest::newRow("10 s") << 10 << QSL("0:00:10");
        QTest::newRow("1 m") << 60 << QSL("0:01:00");
        QTest::newRow("10m") << 60 * 10 << QSL("0:10:00");
        QTest::newRow("1h") << 60 * 60 << QSL("1:00:00");
        QTest::newRow("1h 1 m 1s") << (60 * 60) + 60 + 1 << QSL("1:01:01");
        QTest::newRow("25h 0m 0s") << (25 * 3600) << QSL("25:00:00");
    }

    void testFormatTime()
    {
        QFETCH(int, input);
        QFETCH(QString, output);
        auto formatted = KSysGuard::Formatter::formatValue(input, KSysGuard::UnitTime);
        QCOMPARE(formatted, output);
    }

    void testFormatBootTimestamp_data()
    {
        QTest::addColumn<int>("input");
        // This uses a list because we are comparing translated strings and those may
        // end up slightly different in some cases.
        QTest::addColumn<QStringList>("output");
        QTest::newRow("1s ago") << 1 << QStringList{QSL("Just now")};
        QTest::newRow("10s ago") << 10 << QStringList{QSL("Just now")};
        QTest::newRow("1m ago") << 60 << QStringList{QSL("Just now")};
        QTest::newRow("10m ago") << 600 << QStringList{QSL("10 minute(s) ago"), QSL("10 minutes ago")};
        QTest::newRow("59m59s ago") << 3559 << QStringList{QSL("59 minute(s) ago"), QSL("59 minutes ago")};
    }

    void testFormatBootTimestamp()
    {
        QFETCH(int, input);
        QFETCH(QStringList, output);

        timespec tp;
#ifdef Q_OS_LINUX
        clock_gettime(CLOCK_BOOTTIME, &tp);
#else
        clock_gettime(CLOCK_MONOTONIC, &tp);
#endif

        const auto ticks = sysconf(_SC_CLK_TCK);

        const QDateTime systemBootTime = QDateTime::currentDateTime().addSecs(-tp.tv_sec);
        auto elapsed = QDateTime::currentDateTime().addSecs(-input);
        auto formatted = KSysGuard::Formatter::formatValue(systemBootTime.secsTo(elapsed) * ticks, KSysGuard::UnitBootTimestamp);
        QVERIFY(output.contains(formatted));
    }
};

QTEST_MAIN(FormatterTest);

#include "formattertest.moc"
