#include "kgpgadduid.h"
#include "../transactions/kgpgadduid.h"
#include "../kgpginterface.h"
#include "../kgpgsettings.h"
#include "common.h"

#include <QLatin1String>
#include <QSignalSpy>
#include <QString>
#include <QTest>

void KGpgAddUidTest::init()
{
	QVERIFY(resetGpgConf(m_tempdir));
	QString passphrase = readFile(QLatin1String("keys/kgpgtest_BA7695F3C550DF14.pass"));
	addGpgKey(m_tempdir, QLatin1String("keys/kgpgtest_BA7695F3C550DF14_pub.asc"));
	addGpgKey(m_tempdir, QLatin1String("keys/kgpgtest_BA7695F3C550DF14.asc"), passphrase);
}

void KGpgAddUidTest::testAddUid()
{
	QString passphrase = readFile(QLatin1String("keys/kgpgtest_BA7695F3C550DF14.pass"));

	QString keyId = QLatin1String("BA7695F3C550DF14");

	QFETCH(QString, uid_name);
	QFETCH(QString, uid_email);
	QFETCH(QString, uid_comment);
	KGpgAddUid *transaction = new KGpgAddUid(this, keyId, uid_name, uid_email, uid_comment);
	addPasswordArguments(transaction, passphrase);
	QObject::connect(transaction, &KGpgAddUid::done,
			 [](int result) { QCOMPARE(result, static_cast<int>(KGpgTransaction::TS_OK)); });
	QSignalSpy spy(transaction, &KGpgAddUid::done);
	transaction->start();
	QVERIFY(spy.wait(10000));
	KgpgCore::KgpgKeyList keyList = KgpgInterface::readPublicKeys();
	QCOMPARE(keyList.size(), 1);
	KgpgCore::KgpgKey key = keyList.first();
	QCOMPARE(key.name(), uid_name);
	QCOMPARE(key.email(), uid_email);
	QCOMPARE(key.comment(), uid_comment);
	QCOMPARE(key.fullId(), keyId);
}

void KGpgAddUidTest::testAddUid_data()
{
	QTest::addColumn<QString>("uid_name");
	QTest::addColumn<QString>("uid_email");
	QTest::addColumn<QString>("uid_comment");

	QTest::newRow("all fields") << QString(QLatin1String("Test name"))
			<< QString(QLatin1String("test@kde.org"))
			<< QString(QLatin1String("Test comment"));
	QTest::newRow("only name") << QString(QLatin1String("Test name 2"))
			<< QString() << QString();
	QTest::newRow("name and comment") << QString(QLatin1String("Test name 2"))
			<< QString() << QString(QLatin1String("another comment"));
}

void KGpgAddUidTest::testAddUidInvalid()
{
	QString keyId = QLatin1String("BA7695F3C550DF14");
	QString name = QLatin1String("Test name");
	QString email = QLatin1String("a b"); // intentionally invalid
	QString comment = QLatin1String("Test comment");
	KGpgAddUid *transaction = new KGpgAddUid(this, keyId, name, email, comment);
	QObject::connect(transaction, &KGpgAddUid::done,
			 [](int result) { QCOMPARE(result, static_cast<int>(KGpgAddUid::TS_INVALID_EMAIL)); });
	QSignalSpy spy(transaction, &KGpgAddUid::done);
	transaction->start();

	// for whatever reason spy.wait() does not work here
	int i = 0;
	while (spy.isEmpty() && i++ < 50)
		QTest::qWait(250);
	QCOMPARE(spy.count(), 1);
}

QTEST_GUILESS_MAIN(KGpgAddUidTest)
