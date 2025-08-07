#include "kgpgencrypt.h"

#include "common.h"
#include "../transactions/kgpgencrypt.h"
#include "../transactions/kgpgdecrypt.h"

#include <QLatin1Char>
#include <QLatin1String>
#include <QSignalSpy>
#include <QStringList>
#include <QTest>

void KGpgEncryptTest::init()
{
	QVERIFY(resetGpgConf(m_tempdir));
}

static void checkEncryptedText(const KGpgEncrypt *encryption)
{
	const QStringList elines = encryption->encryptedText();
	QString encryptedText = elines.join(QLatin1Char('\n'));
	//Check if the encrypted text has a header and footer
	QCOMPARE(elines.first(), QStringLiteral("-----BEGIN PGP MESSAGE-----"));
	QCOMPARE(elines.last(), QStringLiteral("-----END PGP MESSAGE-----"));
	//Test if encrypted text contains "KGpg"
	QVERIFY(!elines.contains(QLatin1String("KGpg")));
	QVERIFY(KGpgDecrypt::isEncryptedText(encryptedText, nullptr, nullptr));
	int startPos = -1;
	QVERIFY(KGpgDecrypt::isEncryptedText(encryptedText, &startPos, nullptr));
	QCOMPARE(startPos, 0);
}

void KGpgEncryptTest::testAsciiArmoredEncryption()
{
	//Add keys to keyring
	addGpgKey(m_tempdir, QLatin1String("keys/kgpgtest_BA7695F3C550DF14_pub.asc"));
	QString passphrase = readFile(QLatin1String("keys/kgpgtest_BA7695F3C550DF14.pass"));
	addGpgKey(m_tempdir, QLatin1String("keys/kgpgtest_BA7695F3C550DF14.asc"), passphrase);

	//Encrypt text
	QString text = readFile(QLatin1String("keys/sample_text"));
	QStringList userIds(QLatin1String("Test KGpg"));
	KGpgEncrypt *encryption = new KGpgEncrypt(
		this, userIds, text,
		KGpgEncrypt::AsciiArmored | KGpgEncrypt::AllowUntrustedEncryption);
	QObject::connect(encryption, &KGpgEncrypt::done,
			 [](int result) { QCOMPARE(result, static_cast<int>(KGpgTransaction::TS_OK)); });
	QSignalSpy spy(encryption, &KGpgEncrypt::done);
	encryption->start();
	QVERIFY(spy.wait(encryptionTestTimeout));

	checkEncryptedText(encryption);
	if (QTest::currentTestFailed())
		return;
	QString encryptedText = encryption->encryptedText().join(QLatin1Char('\n'));

	//Decrypt encrypted text
	KGpgDecrypt *decryption = new KGpgDecrypt(this, encryptedText);
	QObject::connect(decryption, &KGpgDecrypt::done,
			 [](int result) { QCOMPARE(result, static_cast<int>(KGpgTransaction::TS_OK)); });
	QSignalSpy spy2(decryption, &KGpgDecrypt::done);
	addPasswordArguments(decryption, passphrase);
	decryption->start();
	QVERIFY(spy2.wait(decryptionTestTimeout));

	//Check if decrypted text is equal to original text
	QVERIFY(text.compare(decryption->decryptedText().join(QLatin1Char('\n'))));
}

void KGpgEncryptTest::testHideKeyIdEncryption()
{
	//Add keys to keyring
	addGpgKey(m_tempdir, QLatin1String("keys/kgpgtest_BA7695F3C550DF14_pub.asc"));
	QString passphrase = readFile(QLatin1String("keys/kgpgtest_BA7695F3C550DF14.pass"));
	addGpgKey(m_tempdir, QLatin1String("keys/kgpgtest_BA7695F3C550DF14.asc"), passphrase);

	//Encrypt text
	QString text = readFile(QLatin1String("keys/sample_text"));
	QStringList userIds(QLatin1String("Test KGpg"));
	KGpgEncrypt *encryption =
		new KGpgEncrypt(this, userIds, text,
				KGpgEncrypt::AsciiArmored | KGpgEncrypt::HideKeyId |
					KGpgEncrypt::AllowUntrustedEncryption);
	QObject::connect(encryption, &KGpgEncrypt::done,
			 [](int result) { QCOMPARE(result, static_cast<int>(KGpgTransaction::TS_OK)); });
	QSignalSpy spy(encryption, &KGpgEncrypt::done);
	encryption->start();
	QVERIFY(spy.wait(encryptionTestTimeout));

	checkEncryptedText(encryption);
	if (QTest::currentTestFailed())
		return;
	QString encryptedText = encryption->encryptedText().join(QLatin1Char('\n'));
	//Check if encrypted text contains key Id
	QString keyId = QLatin1String("BA7695F3C550DF14");
	QString log = encryption->getMessages().join(QLatin1Char('\n'));
	QVERIFY(!log.contains(keyId));
	
	//Decrypt encrypted text
	KGpgDecrypt *decryption = new KGpgDecrypt(this, encryptedText);
	QObject::connect(decryption, &KGpgDecrypt::done,
			 [](int result) { QCOMPARE(result, static_cast<int>(KGpgTransaction::TS_OK)); });
	QSignalSpy spy2(decryption, &KGpgDecrypt::done);
	addPasswordArguments(decryption, passphrase);
	decryption->start();
	QVERIFY(spy2.wait(decryptionTestTimeout));

	//Check if decrypted text is equal to original text
	QVERIFY(text.compare(decryption->decryptedText().join(QLatin1Char('\n'))));
}

void KGpgEncryptTest::testSymmetricEncryption()
{
	QString passphrase = QLatin1String("symmetric key");
	QString text = readFile(QLatin1String("keys/sample_text"));
	QStringList userIds;
	
	//Encrypt text
	KGpgEncrypt *encryption = new KGpgEncrypt(
		this, userIds, text,
		KGpgEncrypt::AsciiArmored | KGpgEncrypt::AllowUntrustedEncryption);
	QObject::connect(encryption, &KGpgEncrypt::done,
			 [](int result) { QCOMPARE(result, static_cast<int>(KGpgTransaction::TS_OK)); });
	QSignalSpy spy(encryption, &KGpgEncrypt::done);
	addPasswordArguments(encryption, passphrase);
	encryption->start();
	QVERIFY(spy.wait(encryptionTestTimeout));

	checkEncryptedText(encryption);
	if (QTest::currentTestFailed())
		return;
	QString encryptedText = encryption->encryptedText().join(QLatin1Char('\n'));
	
	//Decrypt encrypted text
	KGpgDecrypt *decryption = new KGpgDecrypt(this, encryptedText);
	QObject::connect(decryption, &KGpgDecrypt::done,
			 [](int result) { QCOMPARE(result, static_cast<int>(KGpgTransaction::TS_OK)); });
	QSignalSpy spy2(decryption, &KGpgDecrypt::done);
	addPasswordArguments(decryption, passphrase);
	decryption->start();
	QVERIFY(spy2.wait(decryptionTestTimeout));

	//Check if decrypted text is equal to original text
	QVERIFY(text.compare(decryption->decryptedText().join(QLatin1Char('\n'))));
}

QTEST_GUILESS_MAIN(KGpgEncryptTest)
