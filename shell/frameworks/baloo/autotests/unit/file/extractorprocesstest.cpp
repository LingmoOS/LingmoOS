/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2021 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "extractorprocess.h"
#include "testsconfig.h"

#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

namespace Baloo {
namespace Test {

namespace {
QString getWorkerPath()
{
    return QStandardPaths::findExecutable(QStringLiteral("extractorprocess_fake"),
		                          {QStringLiteral(EXTRACTOR_TESTS_HELPER_PATH)});
}
} // <anonymous> namespace

class ExtractorProcessTest : public QObject
{
    Q_OBJECT
public:
    ExtractorProcessTest()
        : m_workerPath(getWorkerPath())
    {}

private Q_SLOTS:
    void initTestCase();

    void testSignals();
    void testSignals2();
    void testExit();

private:
    QString m_workerPath;
};

void ExtractorProcessTest::initTestCase()
{
    QVERIFY(!m_workerPath.isEmpty());
}

void ExtractorProcessTest::testSignals()
{
    Baloo::ExtractorProcess extractor{m_workerPath};
    QSignalSpy spyS(&extractor, &ExtractorProcess::startedIndexingFile);
    QSignalSpy spyF(&extractor, &ExtractorProcess::finishedIndexingFile);
    QSignalSpy spyD(&extractor, &ExtractorProcess::done);

    extractor.index({123, 125});
    QVERIFY(spyD.wait());

    extractor.index({127, 129});
    QVERIFY(spyD.wait());
    QCOMPARE(spyF.size(), 4);
}

void ExtractorProcessTest::testSignals2()
{
    Baloo::ExtractorProcess extractor{m_workerPath};
    QSignalSpy spyS(&extractor, &ExtractorProcess::startedIndexingFile);
    QSignalSpy spyF(&extractor, &ExtractorProcess::finishedIndexingFile);
    QSignalSpy spyD(&extractor, &ExtractorProcess::done);

    extractor.index({123, 23});
    QVERIFY(spyD.wait());
    QCOMPARE(spyS.size(), 2);
    QCOMPARE(spyF.size(), 2);
}

void ExtractorProcessTest::testExit()
{
    Baloo::ExtractorProcess extractor{m_workerPath};
    QSignalSpy spyS(&extractor, &ExtractorProcess::startedIndexingFile);
    QSignalSpy spyF(&extractor, &ExtractorProcess::finishedIndexingFile);
    QSignalSpy spyD(&extractor, &ExtractorProcess::done);
    QSignalSpy spyX(&extractor, &ExtractorProcess::failed);

    extractor.index({123, 0, 23});
    QVERIFY(spyX.wait());
    QCOMPARE(spyS.size(), 2);
    QCOMPARE(spyF.size(), 1);
    QCOMPARE(spyD.size(), 0);
}

} // namespace Test
} // namespace Baloo

using namespace Baloo::Test;

QTEST_GUILESS_MAIN(ExtractorProcessTest)

#include "extractorprocesstest.moc"
