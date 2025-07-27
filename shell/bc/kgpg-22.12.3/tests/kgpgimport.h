#ifndef KGPGIMPORT_TEST_H
#define KGPGIMPORT_TEST_H

#include <QObject>
#include <QTemporaryDir>

class KGpgImportTest: public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void init();
	void testImportTextKey();
	void testImportIdsAll();
	void testImportIdsUnchanged();
	void testImportKeyFromFile();
	void testImportSameKeyTwice();
	void testLogMessage();
	void testImportSecretKey();

private:
	QTemporaryDir m_tempdir;
};

#endif
