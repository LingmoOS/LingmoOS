#include "kgpgdelkey.h"
#include "../core/kgpgkey.h"
#include "../core/KGpgRootNode.h"
#include "../kgpginterface.h"
#include "../model/kgpgitemmodel.h"
#include "../transactions/kgpgdelkey.h"
#include "common.h"

#include <QSignalSpy>
#include <QTest>

void KGpgDelKeyTest::init()
{
	resetGpgConf(m_tempdir);
}

void KGpgDelKeyTest::testDeleteKey()
{
	QFETCH(QString, passphrase);
	addGpgKey(m_tempdir, QLatin1String("keys/kgpgtest_BA7695F3C550DF14.asc"), passphrase);
	QString keyId = QLatin1String("BA7695F3C550DF14");
	KGpgItemModel *model = new KGpgItemModel(this);
	KGpgRootNode *rootNode = new KGpgRootNode(model);
	rootNode->addKeys(QStringList(keyId));
	KGpgKeyNode *keyNode = rootNode->findKey(keyId);
	QVERIFY(keyNode != nullptr);
	KGpgDelKey *transaction = new KGpgDelKey(this, keyNode);
	QObject::connect(transaction, &KGpgDelKey::done,
			 [](int result) { QCOMPARE(result, static_cast<int>(KGpgTransaction::TS_OK)); });
	QSignalSpy spy(transaction, &KGpgDelKey::done);
    transaction->addArgument(QStringLiteral("--yes"));
	transaction->start();
	QVERIFY(spy.wait(10000));
	QCOMPARE(KgpgInterface::readSecretKeys().size(), 0);
	QCOMPARE(KgpgInterface::readPublicKeys().size(), 0);
}

void KGpgDelKeyTest::testDeleteKey_data()
{
	QTest::addColumn<QString>("passphrase");
	QTest::newRow("public") << QString();
	QTest::newRow("secret")
		<< readFile(QLatin1String("keys/kgpgtest_BA7695F3C550DF14.pass"));
}

QTEST_GUILESS_MAIN(KGpgDelKeyTest)
