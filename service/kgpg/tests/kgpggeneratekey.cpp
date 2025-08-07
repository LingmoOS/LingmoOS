/*
    SPDX-FileCopyrightText: 2022 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpggeneratekey.h"
#include "kgpggeneratekeytesttransaction.h"

#include "../core/kgpgkey.h"
#include "../core/KGpgRootNode.h"
#include "../kgpginterface.h"
#include "../model/kgpgitemmodel.h"
#include "../transactions/kgpggeneratekey.h"
#include "common.h"

#include <QSignalSpy>
#include <QTest>

void KGpgGenerateKeyTest::init()
{
	resetGpgConf(m_tempdir);
}

void KGpgGenerateKeyTest::testDeleteKey()
{
	QFETCH(QString, name);
	QFETCH(QString, email);
	QFETCH(QByteArray, passphrase);

	KGpgItemModel *model = new KGpgItemModel(this);

	KGpgGenerateKey *transaction = new KGpgGenerateKeyTestTransaction(this, name, email, QString(),
									passphrase, KgpgCore::ALGO_RSA_RSA, 2048);

	QObject::connect(transaction, &KGpgGenerateKey::done,
			 [](int result) { QCOMPARE(result, static_cast<int>(KGpgTransaction::TS_OK)); });
	QSignalSpy spy(transaction, &KGpgGenerateKey::done);

	transaction->start();
	QVERIFY(spy.wait(10000));
	QCOMPARE(spy.count(), 1);
	QCOMPARE(transaction->getName(), name);
	QCOMPARE(transaction->getEmail(), email);

	const QString fingerprint = transaction->getFingerprint();

	QCOMPARE(KgpgInterface::readSecretKeys().size(), 1);
	QCOMPARE(KgpgInterface::readPublicKeys().size(), 1);
	QCOMPARE(KgpgInterface::readSecretKeys( { fingerprint } ).size(), 1);
	QCOMPARE(KgpgInterface::readPublicKeys( { fingerprint } ).size(), 1);

	model->refreshKey(fingerprint);

	QCOMPARE(model->rowCount(), 1);
}

void KGpgGenerateKeyTest::testDeleteKey_data()
{
	QTest::addColumn<QString>("name");
	QTest::addColumn<QString>("email");
	QTest::addColumn<QByteArray>("passphrase");

	QTest::newRow("with email")
		<< QStringLiteral("test name with email")
		<< QStringLiteral("foobar@example.org")
		<< QByteArray("secret");

	QTest::newRow("without email")
		<< QStringLiteral("test name without email")
		<< QString()
		<< QByteArray("secret");
}

QTEST_MAIN(KGpgGenerateKeyTest)
