#ifndef KGPG_INTERFACE_TEST_H
#define KGPG_INTERFACE_TEST_H

#include <QObject>
#include <QTemporaryDir>

class KGpgInterfaceTest: public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void init();
	void testReadPublicKeys();
	void testReadSecretKeys();
	void testReadEmptyKeyring();

private:
	QTemporaryDir m_tempdir;
};

#endif
