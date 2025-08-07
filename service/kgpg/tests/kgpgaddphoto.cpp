#include "kgpgaddphoto.h"
#include "../transactions/kgpgaddphoto.h"
#include "../kgpginterface.h"
#include "common.h"

#include <QSignalSpy>
#include <QTest>

void KGpgAddPhotoTest::init()
{
	QVERIFY(resetGpgConf(m_tempdir));
}

void KGpgAddPhotoTest::testAddPhoto()
{
	const QString passphrase = readFile(QLatin1String("keys/kgpgtest_BA7695F3C550DF14.pass"));
	addGpgKey(m_tempdir, QLatin1String("keys/kgpgtest_BA7695F3C550DF14_pub.asc"));
	addGpgKey(m_tempdir, QLatin1String("keys/kgpgtest_BA7695F3C550DF14.asc"), passphrase);
	QString keyID = QLatin1String("BA7695F3C550DF14");
	QString imagepath = QLatin1String("keys/image_small.jpg");
	KGpgAddPhoto *transaction = new KGpgAddPhoto(this, keyID, imagepath);
	addPasswordArguments(transaction, passphrase);
	QSignalSpy spy(transaction, &KGpgAddPhoto::done);
	transaction->start();
	QVERIFY(spy.wait(10000));
	QVERIFY(hasPhoto(m_tempdir, keyID));
}

QTEST_GUILESS_MAIN(KGpgAddPhotoTest)
