#include "kgpgverify.h"
#include "../transactions/kgpgverify.h"
#include "common.h"

#include <QList>
#include <QString>
#include <QSignalSpy>
#include <QTest>
#include <QUrl>

void KGpgVerifyTest::init()
{
	QVERIFY(resetGpgConf(m_tempdir));
}

void KGpgVerifyTest::testVerifySignedText()
{
	addGpgKey(m_tempdir, QLatin1String("keys/kgpgtest_BA7695F3C550DF14.asc"));
	QString text = readFile(QLatin1String("keys/signed_text"));
	KGpgVerify *transaction = new KGpgVerify(this, text);
	QSignalSpy spy(transaction, &KGpgVerify::done);
	QObject::connect(transaction, &KGpgVerify::done,
			 [](int result) { QCOMPARE(result, static_cast<int>(KGpgTransaction::TS_OK)); });
	transaction->start();
	QVERIFY(spy.wait(10000));
}

void KGpgVerifyTest::testVerifySignedFile()
{
	addGpgKey(m_tempdir, QLatin1String("keys/kgpgtest_BA7695F3C550DF14.asc"));
	QList<QUrl> list;
	list.append(QUrl::fromLocalFile(QLatin1String("keys/signed_text")));
	KGpgVerify *transaction = new KGpgVerify(this, list);
	QSignalSpy spy(transaction, &KGpgVerify::done);
	QObject::connect(transaction, &KGpgVerify::done,
			 [](int result) { QCOMPARE(result, static_cast<int>(KGpgTransaction::TS_OK)); });
	transaction->start();
	QVERIFY(spy.wait(10000));
}

void KGpgVerifyTest::testVerifyReturnMissingKey()
{
	QList<QUrl> list;
	list.append(QUrl::fromLocalFile(QLatin1String("keys/signed_text")));
	KGpgVerify *transaction = new KGpgVerify(this, list);
	QSignalSpy spy(transaction, &KGpgVerify::done);
	QObject::connect(transaction, &KGpgVerify::done,
			 [](int result) { QCOMPARE(result, static_cast<int>(KGpgVerify::TS_MISSING_KEY)); });
	transaction->start();
	QVERIFY(spy.wait(10000));
}

void KGpgVerifyTest::testVerifyMissingId()
{
	QList<QUrl> list;
	list.append(QUrl::fromLocalFile(QLatin1String("keys/signed_text")));
	KGpgVerify *transaction = new KGpgVerify(this, list);
	QSignalSpy spy(transaction, &KGpgVerify::done);
	QObject::connect(transaction, &KGpgVerify::done, [transaction]() {
		QString keyID = QLatin1String("7882C615210F1022");
		QVERIFY(transaction->missingId().compare(keyID) == 0);
	});
	transaction->start();
	QVERIFY(spy.wait(10000));
}

void KGpgVerifyTest::testVerifyReturnBadSignature()
{
	QList<QUrl> list;
	addGpgKey(m_tempdir, QLatin1String("keys/kgpgtest_BA7695F3C550DF14.asc"));
	list.append(QUrl::fromLocalFile(QLatin1String("keys/signed_bad_sig")));
	KGpgVerify *transaction = new KGpgVerify(this, list);
	QSignalSpy spy(transaction, &KGpgVerify::done);
	QObject::connect(transaction, &KGpgVerify::done,
			 [](int result) { QCOMPARE(result, static_cast<int>(KGpgVerify::TS_BAD_SIGNATURE)); });
	transaction->start();
	QVERIFY(spy.wait(10000));
}

QTEST_GUILESS_MAIN(KGpgVerifyTest)
