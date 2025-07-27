#include "kgpginterface.h"
#include "../kgpginterface.h"
#include "common.h"

#include <QSignalSpy>
#include <QTest>

void KGpgInterfaceTest::init()
{
	QVERIFY(resetGpgConf(m_tempdir));
}

void KGpgInterfaceTest::testReadPublicKeys()
{
	addGpgKey(m_tempdir, QLatin1String("keys/kgpgtest_BA7695F3C550DF14_pub.asc"));
	KgpgCore::KgpgKeyList keys = KgpgInterface::readPublicKeys();
	QString fingerprint = QLatin1String("FBAF 08DD 7D9D 0921 C15D DA9F BA76 95F3 C550 DF14");
	QCOMPARE(keys.size(), 1);
	KgpgCore::KgpgKey key = keys.first();
	QVERIFY(fingerprint.compare(key.fingerprint()));
}

void KGpgInterfaceTest::testReadSecretKeys()
{
	addGpgKey(m_tempdir, QLatin1String("keys/kgpgtest_BA7695F3C550DF14.asc"),
		  readFile(QLatin1String("keys/kgpgtest_BA7695F3C550DF14.pass")));
	KgpgCore::KgpgKeyList keys = KgpgInterface::readSecretKeys();
	QString keyID = QLatin1String("BA7695F3C550DF14");
	QCOMPARE(keys.size(), 1);
	QVERIFY(keys.first().fullId().compare(keyID) == 0);
}

void KGpgInterfaceTest::testReadEmptyKeyring()
{
	KgpgCore::KgpgKeyList keys = KgpgInterface::readSecretKeys();
	KgpgCore::KgpgKeyList pub_keys = KgpgInterface::readPublicKeys();
	QCOMPARE(keys.size(), 0);
	QCOMPARE(pub_keys.size(), 0);
}

QTEST_GUILESS_MAIN(KGpgInterfaceTest)
