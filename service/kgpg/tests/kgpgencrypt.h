#ifndef KGPGENCRYPT_TEST_H
#define KGPGENCRYPT_TEST_H

#include <QObject>
#include <QTemporaryDir>

class KGpgEncryptTest : public QObject {
	Q_OBJECT
private Q_SLOTS:
	void init();
	void testAsciiArmoredEncryption();
	void testHideKeyIdEncryption();
	void testSymmetricEncryption();

private:
	QTemporaryDir m_tempdir;
};

#endif
