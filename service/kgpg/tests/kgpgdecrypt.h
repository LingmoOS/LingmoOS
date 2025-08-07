#ifndef KGPGDECRYPT_TEST_H
#define KGPGDECRYPT_TEST_H

#include <QObject>
#include <QTemporaryDir>

class KGpgDecryptTest : public QObject {
	Q_OBJECT
private Q_SLOTS:
	void init();
	void testDecrypt();
	void testDecrypt_data();

private:
	QTemporaryDir m_tempdir;
};

#endif
