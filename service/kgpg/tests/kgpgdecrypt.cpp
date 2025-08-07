#include "kgpgdecrypt.h"
#include "../transactions/kgpgdecrypt.h"
#include "common.h"

#include <QSignalSpy>
#include <QTest>

void KGpgDecryptTest::init()
{
	QVERIFY(resetGpgConf(m_tempdir));
}

void KGpgDecryptTest::testDecrypt(){
	QFETCH(QString, encryptedFile);
	QString passphrase = readFile(QLatin1String("keys/kgpgtest_BA7695F3C550DF14.pass"));
	addGpgKey(m_tempdir, QLatin1String("keys/kgpgtest_BA7695F3C550DF14_pub.asc"));
	addGpgKey(m_tempdir, QLatin1String("keys/kgpgtest_BA7695F3C550DF14.asc"), passphrase);
	QString text = readFile(QLatin1String("keys/sample_text"));
	QString encryptedText = readFile(encryptedFile);
	KGpgDecrypt *transaction = new KGpgDecrypt(this, encryptedText);
	QObject::connect(transaction, &KGpgDecrypt::done,
			 [](int result) { QCOMPARE(result, static_cast<int>(KGpgTransaction::TS_OK)); });
	QSignalSpy spy(transaction, &KGpgDecrypt::done);
	addPasswordArguments(transaction, passphrase);
	transaction->start();
	QVERIFY(spy.wait(decryptionTestTimeout));
	QVERIFY(text.compare(transaction->decryptedText().join(QLatin1Char('\n'))));
}

void KGpgDecryptTest::testDecrypt_data(){
	QTest::addColumn<QString>("encryptedFile");
	QTest::newRow("AsciiArmored") << QString(QLatin1String("keys/encrypt_text.txt"));
	QTest::newRow("HideKeyId") << QString(QLatin1String("keys/encrypt_text_hide_key_id.txt"));
	QTest::newRow("Symmetrical") << QString(QLatin1String("keys/encrypted_symmetrical.txt"));
}

QTEST_GUILESS_MAIN(KGpgDecryptTest)
