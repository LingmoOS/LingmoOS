#include "kgpgchangedisable.h"
#include "../transactions/kgpgchangedisable.h"
#include "../kgpginterface.h"
#include "common.h"

#include <QSignalSpy>
#include <QTest>

void KGpgChangeDisableTest::init()
{
	QVERIFY(resetGpgConf(m_tempdir));
	addGpgKey(m_tempdir, QLatin1String("keys/kgpgtest_BA7695F3C550DF14_pub.asc"));
}

void KGpgChangeDisableTest::testDisableKey()
{
	const QString keyID = QLatin1String("FBAF08DD7D9D0921C15DDA9FBA7695F3C550DF14");
	KGpgChangeDisable *transaction = new KGpgChangeDisable(this, keyID, true);
	QSignalSpy spy(transaction, &KGpgChangeDisable::done);
	QObject::connect(transaction, &KGpgChangeDisable::done, [](int result) {
		QCOMPARE(result, static_cast<int>(KGpgTransaction::TS_OK));
	});
	transaction->start();
	QVERIFY(spy.wait(10000));
	KgpgCore::KgpgKeyList keyList = KgpgInterface::readPublicKeys(QStringList(keyID));
	QVERIFY(!keyList.isEmpty());
	QVERIFY(!keyList.first().valid());
}

void KGpgChangeDisableTest::testEnableKey()
{
	const QString keyID = QLatin1String("FBAF08DD7D9D0921C15DDA9FBA7695F3C550DF14");
	KGpgChangeDisable *transaction = new KGpgChangeDisable(this, keyID, true);
	QSignalSpy spy(transaction, &KGpgChangeDisable::done);
	transaction->start();
	QVERIFY(spy.wait(10000));
	QObject::connect(transaction, &KGpgChangeDisable::done, [](int result) {
		QCOMPARE(result, static_cast<int>(KGpgTransaction::TS_OK));
	});
	transaction->setDisable(false);
	transaction->start();
	QVERIFY(spy.wait(10000));
	KgpgCore::KgpgKeyList keyList = KgpgInterface::readPublicKeys(QStringList(keyID));
	QVERIFY(!keyList.isEmpty());
	QVERIFY(keyList.first().valid());
}

QTEST_GUILESS_MAIN(KGpgChangeDisableTest)
