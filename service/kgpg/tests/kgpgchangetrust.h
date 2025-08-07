#ifndef KGPGCHANGETRUST_TEST_H
#define KGPGCHANGETRUST_TEST_H

#include <gpgme.h>
#include <QMetaType>
#include <QObject>
#include <QTemporaryDir>

class KGpgChangeTrustTest : public QObject {
	Q_OBJECT
private Q_SLOTS:
	void init();
	void testChangeTrust();
	void testChangeTrust_data();

private:
	QTemporaryDir m_tempdir;
};

Q_DECLARE_METATYPE(gpgme_validity_t)

#endif
