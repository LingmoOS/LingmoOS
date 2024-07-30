/*
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
*/

#include <QFileInfo>
#include <QObject>
#include <QTest>

#include <KLibexec>

class KLibexecTest : public QObject
{
    Q_OBJECT

    const QString m_relative = QStringLiteral("fakeexec/kf6");
    const QString m_fixtureName =
#ifdef Q_OS_WIN
        QStringLiteral("klibexectest-fixture-binary.exe");
#else
        QStringLiteral("klibexectest-fixture-binary");
#endif
    QString m_fixtureDir;
    QString m_fixturePath;

private Q_SLOTS:
    void initTestCase()
    {
        m_fixtureDir = QDir::cleanPath(QCoreApplication::applicationDirPath() + QDir::separator() + m_relative);
        m_fixturePath = QDir::cleanPath(m_fixtureDir + QDir::separator() + m_fixtureName);
        QVERIFY(QDir().mkpath(m_fixtureDir));
        QFile fixture(m_fixturePath);
        QVERIFY(fixture.open(QFile::ReadWrite));
        fixture.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);

        m_fixtureDir = QFileInfo(m_fixtureDir).canonicalFilePath();
        m_fixturePath = QFileInfo(m_fixtureDir).canonicalFilePath();
    }

    void testPath()
    {
        QCOMPARE(KLibexec::path(m_relative), m_fixtureDir);
    }

    void testKDEFrameworksPaths()
    {
        auto paths = KLibexec::kdeFrameworksPaths(m_relative);
        QVERIFY(paths.contains(QCoreApplication::applicationDirPath()));
        QVERIFY(paths.contains(m_fixtureDir));
        // not exhaustive verification
    }
};

QTEST_MAIN(KLibexecTest)

#include "klibexectest.moc"
