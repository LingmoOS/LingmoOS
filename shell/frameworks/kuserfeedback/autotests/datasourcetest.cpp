/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include <applicationversionsource.h>
#include <compilerinfosource.h>
#include <cpuinfosource.h>
#include <localeinfosource.h>
#include <platforminfosource.h>
#include <propertyratiosource.h>
#include <qpainfosource.h>
#include <qtversionsource.h>
#include <screeninfosource.h>
#include <startcountsource.h>
#include <usagetimesource.h>

#include <QDebug>
#include <QtTest/qtest.h>
#include <QObject>
#include <QSettings>
#include <QStandardPaths>

using namespace KUserFeedback;

class DataSourceTest : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int prop READ prop WRITE setProp NOTIFY propChanged)
public:
    int prop() const
    {
        return m_propValue;
    }

    void setProp(int v)
    {
        m_propValue = v;
        Q_EMIT propChanged();
    }

Q_SIGNALS:
    void propChanged();

private:
    int m_propValue = 42;

private Q_SLOTS:
    void initTestCase()
    {
        QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
        QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
        QStandardPaths::setTestModeEnabled(true);
    }

    void testPlatformInfoSource()
    {
        PlatformInfoSource src;
        auto obj = src.data().toMap();
        QVERIFY(obj.contains(QLatin1String("os")));
        auto v = obj.value(QLatin1String("os"));
        QCOMPARE(v.type(), QVariant::String);
        auto s = v.toString();
        QVERIFY(!s.isEmpty());

        QVERIFY(obj.contains(QLatin1String("version")));
        v = obj.value(QLatin1String("version"));
        QCOMPARE(v.type(), QVariant::String);
        s = v.toString();
        QVERIFY(!s.isEmpty());
    }

    void testScreenInfoSource()
    {
        ScreenInfoSource src;
        auto v = src.data();
        QVERIFY(v.canConvert<QVariantList>());
        auto a = v.value<QVariantList>();
        QVERIFY(a.size() > 0);

        for (int i = 0; i < a.size(); ++i) {
            v = a.at(i);
            QVERIFY(v.canConvert<QVariantMap>());
            const auto scr = v.toMap();
            QVERIFY(scr.contains(QLatin1String("height")));
            QVERIFY(scr.contains(QLatin1String("width")));
            QVERIFY(scr.contains(QLatin1String("dpi")));
        }
    }

    void testPropertyRatioSource()
    {
        PropertyRatioSource src(this, "prop", QStringLiteral("ratioSample"));
        src.addValueMapping(42, QStringLiteral("value1"));
        src.addValueMapping(23, QStringLiteral("value2"));
        QTest::qWait(1);

        auto v = src.data();
        QVERIFY(v.canConvert<QVariantMap>());
        auto o = v.toMap();
        QCOMPARE(o.size(), 0); // nothing recorded

        QTest::qWait(1200);
        v = src.data();
        o = v.toMap();
        QCOMPARE(o.size(), 1);
        QVERIFY(o.contains(QLatin1String("value1")));
        v = o.value(QLatin1String("value1")).toMap().value(QLatin1String("property"));;
        QCOMPARE(v.type(), QVariant::Double);

        setProp(23);
        QTest::qWait(1200);
        v = src.data();
        o = v.toMap();
        QCOMPARE(o.size(), 2);
        QVERIFY(o.contains(QLatin1String("value2")));
        v = o.value(QLatin1String("value2")).toMap().value(QLatin1String("property"));
        QCOMPARE(v.type(), QVariant::Double);
    }

    void testMultiPropertyRatioSource()
    {
        QSettings s;
        s.remove(QStringLiteral("MultiPropSource"));
        s.beginGroup(QStringLiteral("MultiPropSource"));

        {
            setProp(5198);

            PropertyRatioSource src1(this, "prop", QStringLiteral("ratioSample"));
            src1.addValueMapping(5198, QStringLiteral("value1"));
            src1.load(&s);

            PropertyRatioSource src2(this, "prop", QStringLiteral("ratioSample"));
            src2.addValueMapping(5198, QStringLiteral("value2"));
            src2.load(&s);

            QTest::qWait(1200);

            src1.store(&s);
            src2.store(&s);
        }

        {
            PropertyRatioSource src3(this, "prop", QStringLiteral("ratioSample"));
            src3.load(&s);
            const auto map = src3.data().toMap();
            QCOMPARE(map.size(), 2);
            QVERIFY(map.contains(QStringLiteral("value1")));
            QVERIFY(map.contains(QStringLiteral("value2")));
            src3.reset(&s);
            QVERIFY(src3.data().toMap().isEmpty());
        }

        {
            PropertyRatioSource src4(this, "prop", QStringLiteral("ratioSample"));
            src4.load(&s);
            const auto map = src4.data().toMap();
            QCOMPARE(map.size(), 0);
        }
    }

    void testApplicationVersionSource()
    {
        ApplicationVersionSource src;
        auto v = src.data();
        QVERIFY(v.isNull());

        QCoreApplication::setApplicationVersion(QStringLiteral("1.9.84"));
        auto m = src.data().toMap();
        QVERIFY(m.contains(QLatin1String("value")));
        QCOMPARE(m.value(QLatin1String("value")).toString(), QLatin1String("1.9.84"));
    }

    void testQtVersionSource()
    {
        QtVersionSource src;
        const auto m = src.data().toMap();
        QVERIFY(m.contains(QLatin1String("value")));
        QCOMPARE(m.value(QLatin1String("value")).toString(), QLatin1String(QT_VERSION_STR));
    }

    void testStartCountSource()
    {
        Provider p;
        auto src = new StartCountSource;
        QVERIFY(!src->description().isEmpty());
        p.addDataSource(src);
        const auto m = src->data().toMap();
        QVERIFY(m.contains(QLatin1String("value")));
        QVERIFY(m.value(QLatin1String("value")).toInt() >= 1);
    }

    void testUsageTimeSource()
    {
        Provider p;
        auto src = new UsageTimeSource;
        QVERIFY(!src->description().isEmpty());
        src->setTelemetryMode(Provider::DetailedUsageStatistics);
        p.addDataSource(src);
        QTest::qWait(1200);
        const auto m = src->data().toMap();
        QVERIFY(m.contains(QLatin1String("value")));
        QVERIFY(m.value(QLatin1String("value")).toInt() >= 1);
    }

    void testCpuInfoSource()
    {
        CpuInfoSource src;
        const auto m = src.data().toMap();
        QVERIFY(m.contains(QLatin1String("architecture")));
        QVERIFY(!m.value(QLatin1String("architecture")).toString().isEmpty());
        QVERIFY(m.contains(QLatin1String("count")));
        QVERIFY(m.value(QLatin1String("count")).toInt() >= 1);
    }

    void testLocaleInfoSource()
    {
        LocaleInfoSource src;
        const auto m = src.data().toMap();
        QVERIFY(m.contains(QLatin1String("language")));
        QVERIFY(!m.value(QLatin1String("language")).toString().isEmpty());
        QVERIFY(m.contains(QLatin1String("region")));
        QVERIFY(!m.value(QLatin1String("region")).toString().isEmpty());
    }

    void testCompilerInfoSource()
    {
        CompilerInfoSource src;
        const auto m = src.data().toMap();
        QVERIFY(m.contains(QLatin1String("type")));
        QVERIFY(!m.value(QLatin1String("type")).toString().isEmpty());
        QVERIFY(m.contains(QLatin1String("version")));
        QVERIFY(!m.value(QLatin1String("version")).toString().isEmpty());
    }

    void testQPAInfoSource()
    {
        QPAInfoSource src;
        QVERIFY(!src.description().isEmpty());
        const auto m = src.data().toMap();
        QVERIFY(m.contains(QLatin1String("name")));
        QVERIFY(!m.value(QLatin1String("name")).toString().isEmpty());
    }

    void testActiveState()
    {
        CpuInfoSource src;

        QVERIFY(src.isActive());

        src.setActive(false);

        QVERIFY(!src.isActive());
    }

    void testCommonActiveStateSetting()
    {
        CpuInfoSource src;
        src.setActive(false);

        QSettings s;

        src.store(&s);

        src.setActive(true);

        src.load(&s);

        QVERIFY(!src.isActive());

        // NOTE: reset shouldn't change global settings that might be changed by a user via UI
        src.reset(&s);

        QVERIFY(!src.isActive());
    }
};

QTEST_MAIN(DataSourceTest)

#include "datasourcetest.moc"
