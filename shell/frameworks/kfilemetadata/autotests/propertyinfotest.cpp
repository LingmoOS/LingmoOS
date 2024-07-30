/*
    This file is part of the KDE KFileMetaData project
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "propertyinfotest.h"
#include "propertyinfo.h"

#include <QTest>

using namespace KFileMetaData;

//QTEST_GUILESS_MAIN(PropertyInfoTest)
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    PropertyInfoTest tc;

    auto arguments = app.arguments();
    if (arguments.contains(QStringLiteral("--localized"))) {
        arguments.removeAll(QStringLiteral("--localized"));
        tc.setLocalized(true);
    }

    return QTest::qExec(&tc, arguments);
}

void PropertyInfoTest::setLocalized(bool localized)
{
    m_useLocalization = localized;
}

void PropertyInfoTest::init()
{
    if (!m_useLocalization) {
        QLocale().setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    }
}

void PropertyInfoTest::testNameIdMapping()
{
    const auto names = PropertyInfo::allNames();

    for (const auto& name : names) {
        const auto pi = PropertyInfo::fromName(name);

        QVERIFY(!pi.name().isEmpty());
        QVERIFY(!pi.displayName().isEmpty());
        QCOMPARE(name, pi.name());

        const auto pi2 = PropertyInfo::fromName(pi.name());
        QCOMPARE(pi.property(), pi2.property());
        QCOMPARE(pi, pi2);
        QCOMPARE(pi, PropertyInfo::fromName(pi.name().toLower()));
        QCOMPARE(pi, PropertyInfo::fromName(pi.name().toUpper()));
    }
}

void PropertyInfoTest::testFormatAsDisplayString() {
    QFETCH(KFileMetaData::PropertyInfo, propertyInfo);
    QFETCH(QVariant, value);
    QFETCH(QString, expected);
    QFETCH(bool, maybeLocalized);

    if (m_useLocalization && maybeLocalized) {
        qDebug() << "Expected:" << expected << ", formatted/localized:" << propertyInfo.formatAsDisplayString(value);
        if (expected != propertyInfo.formatAsDisplayString(value)) {
            QEXPECT_FAIL("", "Expected value not localized", Continue);
        }
    }
    QCOMPARE(propertyInfo.formatAsDisplayString(value), expected);
}

void PropertyInfoTest::testFormatAsDisplayString_data()
{
    QTest::addColumn<KFileMetaData::PropertyInfo>("propertyInfo");
    QTest::addColumn<QVariant>("value");
    // expected values for an en_US locale
    QTest::addColumn<QString>("expected");
    QTest::addColumn<bool>("maybeLocalized");

    auto emptyProperty = PropertyInfo::fromName(QStringLiteral("no valid property name"));
    QTest::addRow("<invalid>")
        << emptyProperty << QVariant(QStringLiteral("empty")) << QStringLiteral("empty") << true;

    QStringList artistList = {QStringLiteral("Artist1"), QStringLiteral("Artist2"), QStringLiteral("Artist3")};
    QStringList authorList = {QStringLiteral("Author1")};
    QVariantList arrangerList = {QStringLiteral("Arranger1"), QStringLiteral("Arranger2")};
    QVariantList bitRateList = {128000, 130000};
    QVariantList titleList = {QStringLiteral("Title1"), QStringLiteral("Title2")};

    struct {
        KFileMetaData::Property::Property property;
        bool maybeLocalized;
        QVariant value;
        QString expected;
    } rows[] = {
        { Property::DiscNumber,             true,  2018,                    QStringLiteral("2018")},
        { Property::Title,                  false, QStringLiteral("Title"), QStringLiteral("Title")},
        { Property::Title,                  true,  titleList,               QStringLiteral("Title1 and Title2")},
        { Property::Artist,                 true,  artistList,              QStringLiteral("Artist1, Artist2, and Artist3")},
        { Property::Author,                 true,  authorList,              QStringLiteral("Author1")},
        { Property::Arranger,               true,  arrangerList,            QStringLiteral("Arranger1 and Arranger2")},
        { Property::Duration,               true,  1800,                    QStringLiteral("0:30:00")},
        { Property::SampleRate,             true,  44100,                   QStringLiteral("44.1 kHz")},
        { Property::BitRate,                true,  128000,                  QStringLiteral("128 kbit/s")},
        { Property::BitRate,                true,  1350000,                 QStringLiteral("1.35 Mbit/s")},
        { Property::BitRate,                true,  14700000,                QStringLiteral("14.7 Mbit/s")},
        { Property::BitRate,                true,  bitRateList,             QStringLiteral("128 kbit/s and 130 kbit/s")},
        { Property::ImageOrientation,       true,  5,                       QStringLiteral("Transposed")},
        { Property::PhotoFlash,             true,  0x00,                    QStringLiteral("No flash")},
        { Property::PhotoFlash,             true,  0x50,                    QStringLiteral("No, red-eye reduction")},
        { Property::PhotoGpsAltitude,       true,  1.1,                     QStringLiteral("1.1 m")},
        // make VisualStudio compiler happy: QChar(0x00B0) = "°"
        { Property::PhotoGpsLatitude,       true,  25,                      QStringLiteral("25") + QChar(0x00B0)},
        { Property::PhotoGpsLongitude,      true,  13.5,                    QStringLiteral("13.5") + QChar(0x00B0)},
        { Property::PhotoExposureTime,      true,  0.0015625,               QStringLiteral("1/640 s")},
        { Property::PhotoExposureTime,      true,  0.5,                     QStringLiteral("0.5 s")},
        { Property::PhotoExposureTime,      true,  0.15,                    QStringLiteral("0.15 s")},
        { Property::PhotoExposureBiasValue, true,  0.33333,                 QStringLiteral("1/3 EV")},
        { Property::PhotoExposureBiasValue, true,  0.66667,                 QStringLiteral("2/3 EV")},
        { Property::PhotoExposureBiasValue, true,  1,                       QStringLiteral("1 EV")},
        { Property::PhotoExposureBiasValue, true,  1.66667,                 QStringLiteral("1 2/3 EV")},
        { Property::PhotoExposureBiasValue, true,  0.1888,                  QStringLiteral("0.189 EV")},
        { Property::PhotoExposureBiasValue, true,  -0.33333,                QStringLiteral("-1/3 EV")},
        { Property::PhotoExposureBiasValue, true,  0,                       QStringLiteral("0 EV")},
        { Property::PhotoExposureBiasValue, true,  -1.5,                    QStringLiteral("-1 1/2 EV")},
        { Property::PhotoFNumber,           true,  4.0,                     QStringLiteral("f/4")},
        { Property::PhotoFNumber,           true,  2.8,                     QStringLiteral("f/2.8")},
        { Property::ReplayGainAlbumGain,    true,  -9.90,                   QStringLiteral("-9.9")},
        { Property::ReplayGainAlbumPeak,    true,  1.512,                   QStringLiteral("1.51")},
        { Property::ReplayGainAlbumGain,    true,  10.44,                   QStringLiteral("10.4")},
        { Property::ReplayGainAlbumPeak,    true,  1.306,                   QStringLiteral("1.31")},
        { Property::FrameRate,              true,  23,                      QStringLiteral("23 fps")},
        { Property::FrameRate,              true,  23.976,                  QStringLiteral("23.98 fps")},
        { Property::AspectRatio,            true,  1.77778,                 QStringLiteral("1.78:1")},
        { Property::PhotoFocalLength,       true,  2.0,                     QStringLiteral("2 mm")},
        { Property::PhotoFocalLength,       true,  2.4,                     QStringLiteral("2.4 mm")},
    };

    for (auto row : rows) {
        PropertyInfo info(row.property);
        QTest::addRow("%s", info.displayName().toUtf8().constData())
            << info << row.value << row.expected << row.maybeLocalized;
    }
}

void PropertyInfoTest::benchmarkPropertyInfo()
{
    QFETCH(QString, propertyName);
    QFETCH(KFileMetaData::Property::Property, propertyId);

    QVERIFY(PropertyInfo::fromName(propertyName).property() == propertyId);

    auto property = PropertyInfo(propertyId);

    QBENCHMARK {
        // Instantiate a PropertyInfo from Id
        property = PropertyInfo(propertyId);
    }
}

void PropertyInfoTest::benchmarkPropertyInfoFromName()
{
    QFETCH(QString, propertyName);
    auto property = PropertyInfo::fromName(propertyName);

    QBENCHMARK {
        // Instantiate a PropertyInfo from its name
        property = PropertyInfo::fromName(propertyName);
    }
}

void PropertyInfoTest::benchmarkPropertyInfoDisplayName()
{
    QFETCH(KFileMetaData::Property::Property, propertyId);

    auto displayName = PropertyInfo(propertyId).displayName();

    QBENCHMARK {
        // Instantiate a PropertyInfo and get the displayName;
        auto property = PropertyInfo(propertyId);
        displayName = property.displayName();
    }
    QVERIFY(!displayName.isEmpty());
}

static void benchmarkTestData()
{
    QTest::addColumn<QString>("propertyName");
    QTest::addColumn<KFileMetaData::Property::Property>("propertyId");

    QTest::addRow("album") << QStringLiteral("album") << KFileMetaData::Property::Album;
    QTest::addRow("width") << QStringLiteral("width") << KFileMetaData::Property::Width;
    QTest::addRow("originUrl") << QStringLiteral("originUrl") << KFileMetaData::Property::OriginUrl;
}

void PropertyInfoTest::benchmarkPropertyInfo_data()
{
    benchmarkTestData();
}

void PropertyInfoTest::benchmarkPropertyInfoFromName_data()
{
    benchmarkTestData();
}

void PropertyInfoTest::benchmarkPropertyInfoDisplayName_data()
{
    benchmarkTestData();
}

#include "moc_propertyinfotest.cpp"
