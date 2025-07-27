#include "kgpgimport.h"
#include "../transactions/kgpgimport.h"
#include "common.h"

#include <QList>
#include <QString>
#include <QStringList>
#include <QSignalSpy>
#include <QTest>
#include <QTextStream>
#include <QUrl>

void KGpgImportTest::init()
{
	QVERIFY(resetGpgConf(m_tempdir));
}

void KGpgImportTest::testImportTextKey()
{
	QString key = readFile(QLatin1String("keys/kgpgtest_BA7695F3C550DF14_pub.asc"));
	QVERIFY(KGpgImport::isKey(key));
	KGpgImport *transaction = new KGpgImport(this, key);
	QSignalSpy spy(transaction, &KGpgImport::done);
	QObject::connect(transaction, &KGpgImport::done, [transaction, key](int result) {
		QCOMPARE(transaction->getImportedKeys().size(), 1);
		QString keyID = QLatin1String("BA7695F3C550DF14");
		QVERIFY(transaction->getImportedKeys().first().startsWith(keyID));
		QCOMPARE(result, static_cast<int>(KGpgTransaction::TS_OK));
	});
	transaction->start();
	QVERIFY(spy.wait(10000));
}

void KGpgImportTest::testImportKeyFromFile()
{
	QList<QUrl> list;
	list.append(QUrl::fromLocalFile(QLatin1String("keys/kgpgtest_BA7695F3C550DF14_pub.asc")));

	KGpgImport *transaction = new KGpgImport(this, list);
	QSignalSpy spy(transaction, &KGpgImport::done);
	QObject::connect(transaction, &KGpgImport::done, [transaction](int result) {
		QCOMPARE(transaction->getImportedKeys().size(), 1);
		QString keyID = QLatin1String("BA7695F3C550DF14");
		QVERIFY(transaction->getImportedKeys().first().startsWith(keyID));
		QCOMPARE(result, static_cast<int>(KGpgTransaction::TS_OK));
	});
	transaction->start();
	QVERIFY(spy.wait(10000));
}

void KGpgImportTest::testImportSameKeyTwice()
{
	QList<QUrl> list;
	list.append(QUrl::fromLocalFile(QLatin1String("keys/kgpgtest_BA7695F3C550DF14_pub.asc")));
	KGpgImport *transaction = new KGpgImport(this, list);
	QSignalSpy spy(transaction, &KGpgImport::done);
	transaction->start();
	QVERIFY(spy.wait(10000));
	QObject::connect(transaction, &KGpgImport::done, [transaction](int result) {
		QCOMPARE(transaction->getImportedKeys().size(), 1);
		QString keyID = QLatin1String("BA7695F3C550DF14");
		QString duplicateMsg = QLatin1String("IMPORT_OK 0 FBAF08DD7D9D0921C15DDA9FBA7695F3C550DF14");
		qDebug() << "MESSAGES: " << transaction->getMessages();
		QVERIFY(transaction->getImportedKeys().first().startsWith(keyID));
		QVERIFY(transaction->getMessages().join(QLatin1Char(' ')).contains(duplicateMsg));
		QEXPECT_FAIL("", "Test is broken. Possible bug!", Continue);
		QCOMPARE(result, static_cast<int>(KGpgTransaction::TS_OK));
	});
	transaction->start();
	QVERIFY(spy.wait(10000));
}

void KGpgImportTest::testImportIdsAll()
{
	QList<QUrl> list;
	list.append(QUrl::fromLocalFile(QLatin1String("keys/kgpgtest_BA7695F3C550DF14_pub.asc")));
	KGpgImport *transaction = new KGpgImport(this, list);
	QSignalSpy spy(transaction, &KGpgImport::done);
	QObject::connect(transaction, &KGpgImport::done, [transaction](int result) {
		QCOMPARE(transaction->getImportedIds(-1).size(), 1);
		QString fingerprint = QLatin1String("FBAF08DD7D9D0921C15DDA9FBA7695F3C550DF14");
		QVERIFY(transaction->getImportedIds(-1).first().compare(fingerprint) == 0);
		QCOMPARE(result, static_cast<int>(KGpgTransaction::TS_OK));
	});
	transaction->start();
	QVERIFY(spy.wait(10000));
}

void KGpgImportTest::testImportIdsUnchanged()
{
	addGpgKey(m_tempdir, QLatin1String("keys/kgpgtest_BA7695F3C550DF14_pub.asc"));
	QList<QUrl> list;
	list.append(QUrl::fromLocalFile(QLatin1String("keys/kgpgtest_BA7695F3C550DF14_pub.asc")));
	KGpgImport *transaction = new KGpgImport(this, list);
	QSignalSpy spy(transaction, &KGpgImport::done);
	QObject::connect(transaction, &KGpgImport::done, [transaction](int result) {
		QCOMPARE(transaction->getImportedIds(0).size(), 1);
		QString fingerprint = QLatin1String("FBAF08DD7D9D0921C15DDA9FBA7695F3C550DF14");
		QVERIFY(transaction->getImportedIds(0).first().compare(fingerprint) == 0);
		QCOMPARE(result, static_cast<int>(KGpgTransaction::TS_OK));
	});
	transaction->start();
	QVERIFY(spy.wait(10000));
}

void KGpgImportTest::testLogMessage()
{
	QList<QUrl> list;
	list.append(QUrl::fromLocalFile(QLatin1String("keys/kgpgtest_BA7695F3C550DF14_pub.asc")));
	KGpgImport *transaction = new KGpgImport(this, list);
	QSignalSpy spy(transaction, &KGpgImport::done);
	QObject::connect(transaction, &KGpgImport::done, [transaction](int result) {
		QString msg = QLatin1String("IMPORT_OK 1 FBAF08DD7D9D0921C15DDA9FBA7695F3C550DF14");
		QVERIFY(transaction->getMessages().join(QLatin1Char(' ')).contains(msg));
		QCOMPARE(result, static_cast<int>(KGpgTransaction::TS_OK));
	});
	transaction->start();
	QVERIFY(spy.wait(10000));
}

void KGpgImportTest::testImportSecretKey()
{
	QList<QUrl> list;
	list.append(QUrl::fromLocalFile(QLatin1String("keys/kgpgtest_BA7695F3C550DF14.asc")));
	KGpgImport *transaction = new KGpgImport(this, list);
	addPasswordArguments(transaction,
			     readFile(QLatin1String("keys/kgpgtest_BA7695F3C550DF14.pass")));
	QSignalSpy spy(transaction, &KGpgImport::done);
	QObject::connect(transaction, &KGpgImport::done, [transaction]() {
		QVERIFY(transaction->getImportedKeys().size() == 1);
		QString keyID = QLatin1String("BA7695F3C550DF14");
		QVERIFY(transaction->getImportedKeys().first().startsWith(keyID));
	});
	transaction->start();
	QVERIFY(spy.wait(10000));
}

QTEST_GUILESS_MAIN(KGpgImportTest)
