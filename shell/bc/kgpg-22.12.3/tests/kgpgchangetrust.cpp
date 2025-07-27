#include "kgpgchangetrust.h"
#include "../transactions/kgpgchangetrust.h"
#include "../kgpginterface.h"
#include "common.h"

#include <QString>
#include <QSignalSpy>
#include <QTest>

void KGpgChangeTrustTest::init()
{
	QVERIFY(resetGpgConf(m_tempdir));
}

void KGpgChangeTrustTest::testChangeTrust()
{
	QFETCH(gpgme_validity_t, target_trust);
	QLatin1String keyID("BA7695F3C550DF14");
	addGpgKey(m_tempdir, QLatin1String("keys/kgpgtest_BA7695F3C550DF14.asc"));
	KGpgChangeTrust *transaction = new KGpgChangeTrust(this, keyID, target_trust);
	QSignalSpy spy(transaction, &KGpgChangeTrust::done);
	QObject::connect(transaction, &KGpgChangeTrust::done,
			 [](int result) { QCOMPARE(result, static_cast<int>(KGpgTransaction::TS_OK)); });
	transaction->start();
	QVERIFY(spy.wait(10000));
	KgpgCore::KgpgKeyList keyList = KgpgInterface::readPublicKeys(QStringList(keyID));
	QVERIFY(!keyList.isEmpty());
	QCOMPARE(keyList.first().ownerTrust(), target_trust);
}

void KGpgChangeTrustTest::testChangeTrust_data()
{
	QTest::addColumn<gpgme_validity_t>("target_trust");
	QTest::newRow("ultimate") << GPGME_VALIDITY_ULTIMATE;
	QTest::newRow("full") << GPGME_VALIDITY_FULL;
	QTest::newRow("marginal") << GPGME_VALIDITY_MARGINAL;
	QTest::newRow("never") << GPGME_VALIDITY_NEVER;
}

QTEST_GUILESS_MAIN(KGpgChangeTrustTest)
