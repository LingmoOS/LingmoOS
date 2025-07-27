#include "kgpgexport.h"
#include "../transactions/kgpgexport.h"
#include "../kgpginterface.h"
#include "common.h"

#include <QSignalSpy>
#include <QTest>
#include <QTemporaryFile>

void KGpgExportTest::init()
{
	QVERIFY(resetGpgConf(m_tempdir));
}

void KGpgExportTest::testExportPublicKeyToFile()
{
	addGpgKey(m_tempdir, QLatin1String("keys/kgpgtest_BA7695F3C550DF14_pub.asc"));
    QTemporaryFile file(QStringLiteral("key_pub.asc"));
	QVERIFY(file.open());
	QString filename = file.fileName();

	QStringList ids(QLatin1String("BA7695F3C550DF14"));
	// Output in Ascii mode
	QStringList options(QLatin1String("--armor"));
	KGpgExport *transaction = new KGpgExport(this, ids, filename, options, false);
	QSignalSpy spy(transaction, &KGpgExport::done);
	QObject::connect(transaction, &KGpgExport::done,
			 [](int result) { QCOMPARE(result, static_cast<int>(KGpgTransaction::TS_OK)); });
	transaction->start();
	QVERIFY(spy.wait(10000));
	QString exportedKey = readFile(filename);
	QString key = readFile(QLatin1String("keys/kgpgtest_BA7695F3C550DF14_pub.asc"));
	QVERIFY(key.compare(exportedKey) == 0);
}

void KGpgExportTest::testExportSecretKeyToFile()
{
	QString passphrase = readFile(QLatin1String("keys/kgpgtest_BA7695F3C550DF14.pass"));
	addGpgKey(m_tempdir, QLatin1String("keys/kgpgtest_BA7695F3C550DF14.asc"), passphrase);
    QTemporaryFile file(QStringLiteral("key.asc"));
	QVERIFY(file.open());
	QString filename = file.fileName();
	QStringList ids(QLatin1String("BA7695F3C550DF14"));
	// Output in Ascii mode
	QStringList options(QLatin1String("--armor"));
	KGpgExport *transaction = new KGpgExport(this, ids, filename, options, true);
	addPasswordArguments(transaction, passphrase);
	QSignalSpy spy(transaction, &KGpgExport::done);
	QObject::connect(transaction, &KGpgExport::done,
			 [](int result) { QCOMPARE(result, static_cast<int>(KGpgTransaction::TS_OK)); });
	transaction->start();
	QVERIFY(spy.wait(10000));

	//reset gpg home dir
	QVERIFY(resetGpgConf(m_tempdir));
	//Import exported key
	addGpgKey(m_tempdir, filename, passphrase);
	KgpgCore::KgpgKeyList keys = KgpgInterface::readSecretKeys();
	QString keyID = QLatin1String("BA7695F3C550DF14");
	QCOMPARE(keys.size(), 1);
	QVERIFY(keys.first().fullId().compare(keyID) == 0);
}

void KGpgExportTest::testExportPublicKeyToStdOutput()
{
	addGpgKey(m_tempdir, QLatin1String("keys/kgpgtest_BA7695F3C550DF14_pub.asc"));

	QStringList ids(QLatin1String("BA7695F3C550DF14"));
	// Output in Ascii mode
	QStringList options(QLatin1String("--armor"));
	KGpgExport *transaction = new KGpgExport(this, ids, options, false);
	QSignalSpy spy(transaction, &KGpgExport::done);
	QObject::connect(transaction, &KGpgExport::done,
			 [](int result) { QCOMPARE(result, static_cast<int>(KGpgTransaction::TS_OK)); });
	transaction->start();
	QVERIFY(spy.wait(10000));
	QString exportedKey = QLatin1String(transaction->getOutputData());
	QString key = readFile(QLatin1String("keys/kgpgtest_BA7695F3C550DF14_pub.asc"));
	QVERIFY(key.compare(exportedKey) == 0);
}

QTEST_GUILESS_MAIN(KGpgExportTest)
