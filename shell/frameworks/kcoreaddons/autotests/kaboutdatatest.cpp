/*
    SPDX-FileCopyrightText: 2008 Friedrich W. H. Kossebau <kossebau@kde.org>
    SPDX-FileCopyrightText: 2017 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

// test object
#include <kaboutdata.h>
// Qt
#include <QFile>
#include <QLatin1String>
#include <QObject>
#include <QTest>
#include <QTextStream>
#ifndef Q_OS_WIN
void initLocale()
{
    qputenv("LC_ALL", "en_US.utf-8");
}

Q_CONSTRUCTOR_FUNCTION(initLocale)
#endif

class KAboutDataTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testLongFormConstructorWithDefaults();
    void testLongFormConstructor();
    void testShortFormConstructor();
    void testSetAddLicense();
    void testSetDesktopFileName();
    void testCopying();

    void testKAboutDataOrganizationDomain();

    void testLicenseSPDXID();
    void testLicenseOrLater();

    void testProductName();
};

static const char AppName[] = "app";
static const char ProgramName[] = "ProgramName";
static const char Version[] = "Version";
static const char ShortDescription[] = "ShortDescription";
static const char CopyrightStatement[] = "CopyrightStatement";
static const char Text[] = "Text";
static const char HomePageAddress[] = "http://test.no.where/";
static const char HomePageSecure[] = "https://test.no.where/";
static const char OrganizationDomain[] = "no.where";
static const char BugsEmailAddress[] = "bugs@no.else";
static const char LicenseText[] = "free to write, reading forbidden";
static const char LicenseFileName[] = "testlicensefile";
static const char LicenseFileText[] = "free to write, reading forbidden, in the file";

void KAboutDataTest::testLongFormConstructorWithDefaults()
{
    KAboutData aboutData(QString::fromLatin1(AppName),
                         QLatin1String(ProgramName),
                         QString::fromLatin1(Version),
                         QLatin1String(ShortDescription),
                         KAboutLicense::Unknown);

    QCOMPARE(aboutData.componentName(), QString::fromLatin1(AppName));
    QCOMPARE(aboutData.productName(), QString::fromLatin1(AppName));
    QCOMPARE(aboutData.displayName(), QLatin1String(ProgramName));
    QCOMPARE(aboutData.programLogo(), QVariant());
    QCOMPARE(aboutData.organizationDomain(), QString::fromLatin1("kde.org"));
    QCOMPARE(aboutData.version(), QString::fromLatin1(Version));
    QCOMPARE(aboutData.homepage(), QString());
    QCOMPARE(aboutData.bugAddress(), QString::fromLatin1("submit@bugs.kde.org"));
    QVERIFY(aboutData.authors().isEmpty());
    QVERIFY(aboutData.credits().isEmpty());
    QVERIFY(aboutData.translators().isEmpty());
    QCOMPARE(aboutData.otherText(), QString());
    QCOMPARE(aboutData.licenses().count(), 1);
    // We don't know the default text, do we?
    //     QCOMPARE( aboutData.licenses().at(0).name(KAboutLicense::ShortName), QString(WarningText) );
    QVERIFY(!aboutData.licenses().at(0).name(KAboutLicense::ShortName).isEmpty());
    //     QCOMPARE( aboutData.licenses().at(0).name(KAboutLicense::FullName), QString(WarningText) );
    QVERIFY(!aboutData.licenses().at(0).name(KAboutLicense::FullName).isEmpty());
    //     QCOMPARE( aboutData.licenses().at(0).text(), QString(WarningText) );
    QVERIFY(!aboutData.licenses().at(0).text().isEmpty());
    QCOMPARE(aboutData.copyrightStatement(), QString());
    QCOMPARE(aboutData.shortDescription(), (QLatin1String(ShortDescription)));
    QCOMPARE(aboutData.customAuthorPlainText(), QString());
    QCOMPARE(aboutData.customAuthorRichText(), QString());
    QVERIFY(!aboutData.customAuthorTextEnabled());
    QCOMPARE(aboutData.desktopFileName(), QStringLiteral("org.kde.app"));

    QCOMPARE(aboutData.internalVersion(), Version);
    QCOMPARE(aboutData.internalProgramName(), ProgramName);
    QCOMPARE(aboutData.internalBugAddress(), "submit@bugs.kde.org");
    QCOMPARE(aboutData.internalProductName(), nullptr);
}

void KAboutDataTest::testLongFormConstructor()
{
    KAboutData aboutData(QString::fromLatin1(AppName),
                         QLatin1String(ProgramName),
                         QString::fromLatin1(Version),
                         QLatin1String(ShortDescription),
                         KAboutLicense::Unknown,
                         QLatin1String(CopyrightStatement),
                         QLatin1String(Text),
                         QString::fromLatin1(HomePageAddress),
                         QString::fromLatin1(BugsEmailAddress));

    QCOMPARE(aboutData.componentName(), QLatin1String(AppName));
    QCOMPARE(aboutData.productName(), QLatin1String(AppName));
    QCOMPARE(aboutData.displayName(), QLatin1String(ProgramName));
    QCOMPARE(aboutData.programLogo(), QVariant());
    QCOMPARE(aboutData.organizationDomain(), QString::fromLatin1(OrganizationDomain));
    QCOMPARE(aboutData.version(), QString::fromLatin1(Version));
    QCOMPARE(aboutData.homepage(), QString::fromLatin1(HomePageAddress));
    QCOMPARE(aboutData.bugAddress(), QString::fromLatin1(BugsEmailAddress));
    QVERIFY(aboutData.authors().isEmpty());
    QVERIFY(aboutData.credits().isEmpty());
    QVERIFY(aboutData.translators().isEmpty());
    QCOMPARE(aboutData.otherText(), QLatin1String(Text));
    QCOMPARE(aboutData.licenses().count(), 1);
    // We don't know the default text, do we?
    //     QCOMPARE( aboutData.licenses().at(0).name(KAboutLicense::ShortName), QString(WarningText) );
    QVERIFY(!aboutData.licenses().at(0).name(KAboutLicense::ShortName).isEmpty());
    //     QCOMPARE( aboutData.licenses().at(0).name(KAboutLicense::FullName), QString(WarningText) );
    QVERIFY(!aboutData.licenses().at(0).name(KAboutLicense::FullName).isEmpty());
    //     QCOMPARE( aboutData.licenses().at(0).text(), QString(WarningText) );
    QVERIFY(!aboutData.licenses().at(0).text().isEmpty());
    QCOMPARE(aboutData.copyrightStatement(), QLatin1String(CopyrightStatement));
    QCOMPARE(aboutData.shortDescription(), QLatin1String(ShortDescription));
    QCOMPARE(aboutData.customAuthorPlainText(), QString());
    QCOMPARE(aboutData.customAuthorRichText(), QString());
    QVERIFY(!aboutData.customAuthorTextEnabled());
    QCOMPARE(aboutData.desktopFileName(), QStringLiteral("where.no.app"));

    QCOMPARE(aboutData.internalVersion(), Version);
    QCOMPARE(aboutData.internalProgramName(), ProgramName);
    QCOMPARE(aboutData.internalBugAddress(), BugsEmailAddress);
    QCOMPARE(aboutData.internalProductName(), nullptr);

    aboutData.addAuthor(QStringLiteral("John"),
                        QStringLiteral("Stuff"),
                        QStringLiteral("john@john.com"),
                        QStringLiteral("john.com"),
                        QStringLiteral("ocsjohn"));
    QCOMPARE(aboutData.authors().length(), 1);
    QCOMPARE(aboutData.credits().length(), 0);
    QCOMPARE(aboutData.authors().at(0).avatarUrl(), QUrl(QStringLiteral("https://store.kde.org/avatar/ocsjohn")));

    // We support http and https protocols on the homepage address, ensure they
    // give the same org. domain and desktop file name.
    KAboutData aboutDataSecure(QString::fromLatin1(AppName),
                               QLatin1String(ProgramName),
                               QString::fromLatin1(Version),
                               QLatin1String(ShortDescription),
                               KAboutLicense::Unknown,
                               QLatin1String(CopyrightStatement),
                               QLatin1String(Text),
                               QString::fromLatin1(HomePageSecure),
                               QString::fromLatin1(BugsEmailAddress));
    QCOMPARE(aboutDataSecure.componentName(), QLatin1String(AppName));
    QCOMPARE(aboutDataSecure.productName(), QLatin1String(AppName));
    QCOMPARE(aboutDataSecure.organizationDomain(), QString::fromLatin1(OrganizationDomain));
    QCOMPARE(aboutDataSecure.desktopFileName(), QStringLiteral("where.no.app"));
}

void KAboutDataTest::testShortFormConstructor()
{
    KAboutData aboutData(QString::fromLatin1(AppName), QLatin1String(ProgramName), QString::fromLatin1(Version));

    QCOMPARE(aboutData.componentName(), QString::fromLatin1(AppName));
    QCOMPARE(aboutData.productName(), QString::fromLatin1(AppName));
    QCOMPARE(aboutData.displayName(), QLatin1String(ProgramName));
    QCOMPARE(aboutData.programLogo(), QVariant());
    QCOMPARE(aboutData.organizationDomain(), QString::fromLatin1("kde.org"));
    QCOMPARE(aboutData.version(), QString::fromLatin1(Version));
    QCOMPARE(aboutData.homepage(), QString());
    QCOMPARE(aboutData.bugAddress(), QString::fromLatin1("submit@bugs.kde.org"));
    QVERIFY(aboutData.authors().isEmpty());
    QVERIFY(aboutData.credits().isEmpty());
    QVERIFY(aboutData.translators().isEmpty());
    QCOMPARE(aboutData.otherText(), QString());
    QCOMPARE(aboutData.licenses().count(), 1);
    // We don't know the default text, do we?
    //     QCOMPARE( aboutData.licenses().at(0).name(KAboutLicense::ShortName), QString(WarningText) );
    QVERIFY(!aboutData.licenses().at(0).name(KAboutLicense::ShortName).isEmpty());
    //     QCOMPARE( aboutData.licenses().at(0).name(KAboutLicense::FullName), QString(WarningText) );
    QVERIFY(!aboutData.licenses().at(0).name(KAboutLicense::FullName).isEmpty());
    //     QCOMPARE( aboutData.licenses().at(0).text(), QString(WarningText) );
    QVERIFY(!aboutData.licenses().at(0).text().isEmpty());
    QCOMPARE(aboutData.copyrightStatement(), QString());
    QCOMPARE(aboutData.shortDescription(), QString());
    QCOMPARE(aboutData.customAuthorPlainText(), QString());
    QCOMPARE(aboutData.customAuthorRichText(), QString());
    QVERIFY(!aboutData.customAuthorTextEnabled());
    QCOMPARE(aboutData.desktopFileName(), QStringLiteral("org.kde.app"));

    QCOMPARE(aboutData.internalVersion(), Version);
    QCOMPARE(aboutData.internalProgramName(), ProgramName);
    QCOMPARE(aboutData.internalBugAddress(), "submit@bugs.kde.org");
    QCOMPARE(aboutData.internalProductName(), nullptr);
}

void KAboutDataTest::testKAboutDataOrganizationDomain()
{
    KAboutData data(QString::fromLatin1("app"),
                    QLatin1String("program"),
                    QString::fromLatin1("version"),
                    QLatin1String("description"),
                    KAboutLicense::LGPL,
                    QLatin1String("copyright"),
                    QLatin1String("hello world"),
                    QStringLiteral("http://www.koffice.org"));
    QCOMPARE(data.organizationDomain(), QString::fromLatin1("koffice.org"));
    QCOMPARE(data.desktopFileName(), QStringLiteral("org.koffice.app"));

    KAboutData data2(QString::fromLatin1("app"),
                     QLatin1String("program"),
                     QString::fromLatin1("version"),
                     QLatin1String("description"),
                     KAboutLicense::LGPL,
                     QString::fromLatin1("copyright"),
                     QLatin1String("hello world"),
                     QStringLiteral("app"));
    QCOMPARE(data2.organizationDomain(), QString::fromLatin1("kde.org"));
    QCOMPARE(data2.desktopFileName(), QStringLiteral("org.kde.app"));
}

void KAboutDataTest::testSetAddLicense()
{
    // prepare a file with a license text
    QFile licenseFile(QString::fromLatin1(LicenseFileName));
    licenseFile.open(QIODevice::WriteOnly);
    QTextStream licenseFileStream(&licenseFile);
    licenseFileStream << LicenseFileText;
    licenseFile.close();

    const QString copyrightStatement = QLatin1String(CopyrightStatement);
    const QString lineFeed = QString::fromLatin1("\n\n");

    KAboutData aboutData(QString::fromLatin1(AppName),
                         QLatin1String(ProgramName),
                         QString::fromLatin1(Version),
                         QLatin1String(ShortDescription),
                         KAboutLicense::Unknown,
                         QLatin1String(CopyrightStatement),
                         QLatin1String(Text),
                         QString::fromLatin1(HomePageAddress),
                         QString::fromLatin1(BugsEmailAddress));

    // set to GPL2
    aboutData.setLicense(KAboutLicense::GPL_V2);

    QCOMPARE(aboutData.licenses().count(), 1);
    QCOMPARE(aboutData.licenses().at(0).name(KAboutLicense::ShortName), QString::fromLatin1("GPL v2"));
    QCOMPARE(aboutData.licenses().at(0).name(KAboutLicense::FullName), QString::fromLatin1("GNU General Public License Version 2"));
    //     QCOMPARE( aboutData.licenses().at(0).text(), QString(GPL2Text) );
    QVERIFY(!aboutData.licenses().at(0).text().isEmpty());

    // set to Unknown again
    aboutData.setLicense(KAboutLicense::Unknown);

    QCOMPARE(aboutData.licenses().count(), 1);
    // We don't know the default text, do we?
    //     QCOMPARE( aboutData.licenses().at(0).name(KAboutLicense::ShortName), QString(WarningText) );
    QVERIFY(!aboutData.licenses().at(0).name(KAboutLicense::ShortName).isEmpty());
    //     QCOMPARE( aboutData.licenses().at(0).name(KAboutLicense::FullName), QString(WarningText) );
    QVERIFY(!aboutData.licenses().at(0).name(KAboutLicense::FullName).isEmpty());
    //     QCOMPARE( aboutData.licenses().at(0).text(), QString(WarningText) );
    QVERIFY(!aboutData.licenses().at(0).text().isEmpty());

    // add GPL3
    aboutData.addLicense(KAboutLicense::GPL_V3);

    QCOMPARE(aboutData.licenses().count(), 1);
    QCOMPARE(aboutData.licenses().at(0).name(KAboutLicense::ShortName), QString::fromLatin1("GPL v3"));
    QCOMPARE(aboutData.licenses().at(0).name(KAboutLicense::FullName), QString::fromLatin1("GNU General Public License Version 3"));
    //     QCOMPARE( aboutData.licenses().at(0).text(), QString(GPL3Text) );
    QVERIFY(!aboutData.licenses().at(0).text().isEmpty());

    // add GPL2, Custom and File
    aboutData.addLicense(KAboutLicense::GPL_V2);
    aboutData.addLicenseText(QLatin1String(LicenseText));
    aboutData.addLicenseTextFile(QLatin1String(LicenseFileName));

    QCOMPARE(aboutData.licenses().count(), 4);
    QCOMPARE(aboutData.licenses().at(0).name(KAboutLicense::ShortName), QString::fromLatin1("GPL v3"));
    QCOMPARE(aboutData.licenses().at(0).name(KAboutLicense::FullName), QString::fromLatin1("GNU General Public License Version 3"));
    //     QCOMPARE( aboutData.licenses().at(0).text(), QString(GPL3Text) );
    QVERIFY(!aboutData.licenses().at(0).text().isEmpty());
    QCOMPARE(aboutData.licenses().at(1).name(KAboutLicense::ShortName), QString::fromLatin1("GPL v2"));
    QCOMPARE(aboutData.licenses().at(1).name(KAboutLicense::FullName), QString::fromLatin1("GNU General Public License Version 2"));
    //     QCOMPARE( aboutData.licenses().at(1).text(), QString(GPL2Text) );
    QVERIFY(!aboutData.licenses().at(1).text().isEmpty());
    QCOMPARE(aboutData.licenses().at(2).name(KAboutLicense::ShortName), QString::fromLatin1("Custom"));
    QCOMPARE(aboutData.licenses().at(2).name(KAboutLicense::FullName), QString::fromLatin1("Custom"));
    QCOMPARE(aboutData.licenses().at(2).text(), QLatin1String(LicenseText));
    QCOMPARE(aboutData.licenses().at(3).name(KAboutLicense::ShortName), QString::fromLatin1("Custom"));
    QCOMPARE(aboutData.licenses().at(3).name(KAboutLicense::FullName), QString::fromLatin1("Custom"));
    QCOMPARE(aboutData.licenses().at(3).text(), QString(copyrightStatement + lineFeed + QLatin1String(LicenseFileText)));
}

void KAboutDataTest::testCopying()
{
    KAboutData aboutData(QString::fromLatin1(AppName),
                         QLatin1String(ProgramName),
                         QString::fromLatin1(Version),
                         QLatin1String(ShortDescription),
                         KAboutLicense::GPL_V2);

    {
        KAboutData aboutData2(QString::fromLatin1(AppName),
                              QLatin1String(ProgramName),
                              QString::fromLatin1(Version),
                              QLatin1String(ShortDescription),
                              KAboutLicense::GPL_V3);
        aboutData2.addLicense(KAboutLicense::GPL_V2, KAboutLicense::OrLaterVersions);
        aboutData = aboutData2;
    }
    QList<KAboutLicense> licenses = aboutData.licenses();
    QCOMPARE(licenses.count(), 2);
    QCOMPARE(licenses.at(0).key(), KAboutLicense::GPL_V3);
    QCOMPARE(aboutData.licenses().at(0).spdx(), QStringLiteral("GPL-3.0"));
    // check it doesn't crash
    QVERIFY(!licenses.at(0).text().isEmpty());
    QCOMPARE(licenses.at(1).key(), KAboutLicense::GPL_V2);
    QCOMPARE(aboutData.licenses().at(1).spdx(), QStringLiteral("GPL-2.0+"));
    // check it doesn't crash
    QVERIFY(!licenses.at(1).text().isEmpty());
}

void KAboutDataTest::testSetDesktopFileName()
{
    KAboutData aboutData(QString::fromLatin1(AppName),
                         QLatin1String(ProgramName),
                         QString::fromLatin1(Version),
                         QLatin1String(ShortDescription),
                         KAboutLicense::Unknown);
    QCOMPARE(aboutData.desktopFileName(), QStringLiteral("org.kde.app"));

    // set different desktopFileName
    aboutData.setDesktopFileName(QStringLiteral("foo.bar.application"));
    QCOMPARE(aboutData.desktopFileName(), QStringLiteral("foo.bar.application"));
}

void KAboutDataTest::testLicenseSPDXID()
{
    // Input with + should output with +.
    auto license = KAboutLicense::byKeyword(QStringLiteral("GPLv2+"));
    QCOMPARE(license.spdx(), QStringLiteral("GPL-2.0+"));
    // Input without should output without.
    license = KAboutLicense::byKeyword(QStringLiteral("GPLv2"));
    QCOMPARE(license.spdx(), QStringLiteral("GPL-2.0"));
    // Input of spdx with or-later should also work
    license = KAboutLicense::byKeyword(QStringLiteral("GPL-2.0-or-later"));
    QCOMPARE(license.spdx(), QStringLiteral("GPL-2.0+")); // TODO: should not return the deprecatd version

    // we should be able to match by spdx too
    // create a KAboutLicense from enum, then make sure going to spdx and back gives the same enum
    for (int i = 1; i <= KAboutLicense::LGPL_V2_1; ++i) { /*current highest enum value*/
        KAboutData aboutData(QString::fromLatin1(AppName),
                             QLatin1String(ProgramName),
                             QString::fromLatin1(Version),
                             QLatin1String(ShortDescription),
                             KAboutLicense::GPL_V2);
        aboutData.setLicense(KAboutLicense::LicenseKey(i));
        QVERIFY(aboutData.licenses().count() == 1);
        const auto license = aboutData.licenses().constFirst();
        auto licenseFromKeyword = KAboutLicense::byKeyword(license.spdx());

        QCOMPARE(license.key(), licenseFromKeyword.key());
    }
}

void KAboutDataTest::testLicenseOrLater()
{
    // For kaboutdata we can replace the license with an orLater version. Or add a second one.
    KAboutData aboutData(QString::fromLatin1(AppName),
                         QLatin1String(ProgramName),
                         QString::fromLatin1(Version),
                         QLatin1String(ShortDescription),
                         KAboutLicense::GPL_V2);
    QCOMPARE(aboutData.licenses().at(0).spdx(), QStringLiteral("GPL-2.0"));
    aboutData.setLicense(KAboutLicense::GPL_V2, KAboutLicense::OrLaterVersions);
    QCOMPARE(aboutData.licenses().at(0).spdx(), QStringLiteral("GPL-2.0+"));
    aboutData.addLicense(KAboutLicense::LGPL_V3, KAboutLicense::OrLaterVersions);
    bool foundLGPL = false;
    const QList<KAboutLicense> licenses = aboutData.licenses();
    for (const auto &license : licenses) {
        if (license.key() == KAboutLicense::LGPL_V3) {
            QCOMPARE(license.spdx(), QStringLiteral("LGPL-3.0+"));
            foundLGPL = true;
            break;
        }
    }
    QCOMPARE(foundLGPL, true);
}

void KAboutDataTest::testProductName()
{
    KAboutData aboutData(QString::fromLatin1(AppName), QString::fromLatin1(ProgramName));
    QCOMPARE(aboutData.productName(), QString::fromLatin1(AppName));
    QCOMPARE(aboutData.internalProductName(), nullptr);
    aboutData.setProductName("frameworks-kcoreaddons/aboutdata");
    QCOMPARE(aboutData.productName(), QString::fromLatin1("frameworks-kcoreaddons/aboutdata"));
    QCOMPARE(aboutData.internalProductName(), "frameworks-kcoreaddons/aboutdata");
}

QTEST_MAIN(KAboutDataTest)

#include "kaboutdatatest.moc"
