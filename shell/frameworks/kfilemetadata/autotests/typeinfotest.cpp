
/*
    This file is part of the KDE KFileMetaData project
    SPDX-FileCopyrightText: 2024 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "typeinfo.h"

#include <QTest>

using namespace KFileMetaData;

class TypeInfoTest : public QObject
{
    Q_OBJECT
public:
    void setLocalized(bool);

private Q_SLOTS:
    void init();
    void testNameIdMapping();
    void benchmarkTypeInfo();
    void benchmarkTypeInfo_data();
    void benchmarkTypeInfoFromName();
    void benchmarkTypeInfoFromName_data();
    void benchmarkTypeInfoDisplayName();
    void benchmarkTypeInfoDisplayName_data();
private:
    bool m_useLocalization = false;
};

// Replacement for QTEST_GUILESS_MAIN(TypeInfoTest)
// supporting the "--localized" argument
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    TypeInfoTest tc;

    auto arguments = app.arguments();
    if (arguments.contains(QStringLiteral("--localized"))) {
        arguments.removeAll(QStringLiteral("--localized"));
        tc.setLocalized(true);
    }

    return QTest::qExec(&tc, arguments);
}

void TypeInfoTest::setLocalized(bool localized)
{
    m_useLocalization = localized;
}

void TypeInfoTest::init()
{
    if (!m_useLocalization) {
        QLocale().setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    }
}

void TypeInfoTest::testNameIdMapping()
{
    const auto names = TypeInfo::allNames();

    for (const auto& name : names) {
        const auto ti = TypeInfo::fromName(name);

        QVERIFY(!ti.name().isEmpty());
        QVERIFY(!ti.displayName().isEmpty());
        QCOMPARE(name, ti.name());

        const auto ti2 = TypeInfo::fromName(ti.name());
        QCOMPARE(ti.type(), ti2.type());
        QCOMPARE(ti, ti2);
        QCOMPARE(ti, TypeInfo::fromName(ti.name().toLower()));
        QCOMPARE(ti, TypeInfo::fromName(ti.name().toUpper()));
    }
}

void TypeInfoTest::benchmarkTypeInfo()
{
    QFETCH(QString, typeName);
    QFETCH(KFileMetaData::Type::Type, typeId);

    QVERIFY(TypeInfo::fromName(typeName).type() == typeId);

    auto type = TypeInfo(typeId);

    QBENCHMARK {
        // Instantiate a TypeInfo from Id
        type = TypeInfo(typeId);
    }
}

void TypeInfoTest::benchmarkTypeInfoFromName()
{
    QFETCH(QString, typeName);
    auto type = TypeInfo::fromName(typeName);

    QBENCHMARK {
        // Instantiate a TypeInfo from its name
        type = TypeInfo::fromName(typeName);
    }
}

void TypeInfoTest::benchmarkTypeInfoDisplayName()
{
    QFETCH(KFileMetaData::Type::Type, typeId);

    auto displayName = TypeInfo(typeId).displayName();

    QBENCHMARK {
        // Instantiate a TypeInfo and get the displayName;
        auto type = TypeInfo(typeId);
        displayName = type.displayName();
    }
    QVERIFY(!displayName.isEmpty());
}

static void benchmarkTestData()
{
    QTest::addColumn<QString>("typeName");
    QTest::addColumn<KFileMetaData::Type::Type>("typeId");

    QTest::addRow("archive") << QStringLiteral("archive") << KFileMetaData::Type::Archive;
    QTest::addRow("spreadsheet") << QStringLiteral("spreadsheet") << KFileMetaData::Type::Spreadsheet;
    QTest::addRow("folder") << QStringLiteral("folder") << KFileMetaData::Type::Folder;
}

void TypeInfoTest::benchmarkTypeInfo_data()
{
    benchmarkTestData();
}

void TypeInfoTest::benchmarkTypeInfoFromName_data()
{
    benchmarkTestData();
}

void TypeInfoTest::benchmarkTypeInfoDisplayName_data()
{
    benchmarkTestData();
}

#include "typeinfotest.moc"
