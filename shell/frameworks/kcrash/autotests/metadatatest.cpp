/*
    SPDX-License-Identifier: LGPL-2.0-or-later
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
*/

#include <QTest>

#include "../src/metadata.cpp"

using namespace KCrash;

class MetadataTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testEverything()
    {
        QTemporaryDir tmpDir;
        QVERIFY(tmpDir.isValid());

        const QString iniFile = QStringLiteral("%1/foo.ini").arg(tmpDir.path());
        Metadata data("BEFEHL");
#ifdef Q_OS_LINUX
        MetadataINIWriter iniWriter(iniFile.toLocal8Bit());
        data.setAdditionalWriter(&iniWriter);
#endif
        data.add("--ABC", "FOO\nBAR");
        data.addBool("--Meow");
        data.close();
        const int argc = data.argc;
        QCOMPARE(argc, 4);
        QCOMPARE(data.argv.at(0), QStringLiteral("BEFEHL")); // make sure we do stringy comparison
        QCOMPARE(data.argv.at(1), QStringLiteral("--ABC"));
        QCOMPARE(data.argv.at(2), QStringLiteral("FOO\nBAR"));
        QCOMPARE(data.argv.at(3), QStringLiteral("--Meow"));
        QCOMPARE(data.argv.at(4), nullptr); // list should be null terminated

#ifdef Q_OS_LINUX
        QFile::exists(iniFile);
        QFile ini(iniFile);
        QVERIFY(ini.open(QFile::ReadOnly));
        QCOMPARE(ini.readLine(), "[KCrash]\n");
        QCOMPARE(ini.readLine(), "ABC=FOO\\nBAR\n");
        QCOMPARE(ini.readLine(), "Meow=true\n");
        QVERIFY(ini.atEnd()); // nothing after final newline
#endif
    }

    void testNoFile()
    {
        // Doesn't explode without writer
        Metadata data("BEFEHL");
        data.add("--ABC", "FOO");
        data.close();
        const int argc = data.argc;
        QCOMPARE(argc, 3);
    }
};

QTEST_MAIN(MetadataTest)

#include "metadatatest.moc"
