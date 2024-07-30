/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "fallbackpackagetest.h"

#include "packageloader.h"
#include "packagestructure.h"
#include <KLocalizedString>

void FallbackPackageTest::initTestCase()
{
    m_fallPackagePath = QFINDTESTDATA("data/testpackage");
    m_fallbackPkg = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("KPackage/Generic"));
    m_fallbackPkg.addFileDefinition("mainscript", QStringLiteral("ui/main.qml"));
    m_fallbackPkg.setPath(m_fallPackagePath);

    m_packagePath = QFINDTESTDATA("data/testfallbackpackage");
    m_pkg = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("KPackage/Generic"));
    m_pkg.addFileDefinition("mainscript", QStringLiteral("ui/main.qml"));
    m_pkg.setPath(m_packagePath);
}

void FallbackPackageTest::beforeFallback()
{
    QVERIFY(m_fallbackPkg.hasValidStructure());
    QVERIFY(m_pkg.hasValidStructure());

    // m_fallbackPkg should have otherfile.qml, m_pkg shouldn't
    QVERIFY(!m_fallbackPkg.filePath("ui", QStringLiteral("otherfile.qml")).isEmpty());
    QVERIFY(m_pkg.filePath("ui", QStringLiteral("otherfile.qml")).isEmpty());
}

void FallbackPackageTest::afterFallback()
{
    m_pkg.setFallbackPackage(m_fallbackPkg);

    // after setting the fallback, m_pkg should resolve the exact same file as m_fallbackPkg
    // for otherfile.qml
    QVERIFY(!m_pkg.filePath("ui", QStringLiteral("otherfile.qml")).isEmpty());
    QCOMPARE(m_fallbackPkg.filePath("ui", QStringLiteral("otherfile.qml")), m_pkg.filePath("ui", QStringLiteral("otherfile.qml")));
    QVERIFY(m_fallbackPkg.filePath("mainscript") != m_pkg.filePath("mainscript"));
}

void FallbackPackageTest::cycle()
{
    m_fallbackPkg.setFallbackPackage(m_pkg);
    m_pkg.setFallbackPackage(m_fallbackPkg);

    // The cycle should have been detected and filePath should take a not infinite time
    QTRY_COMPARE_WITH_TIMEOUT(m_fallbackPkg.filePath("ui", QStringLiteral("otherfile.qml")), m_pkg.filePath("ui", QStringLiteral("otherfile.qml")), 1000);
}

QTEST_MAIN(FallbackPackageTest)

#include "moc_fallbackpackagetest.cpp"
