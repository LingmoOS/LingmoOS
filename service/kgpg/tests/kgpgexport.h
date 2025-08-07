#ifndef KGPGEXPORT_TEST_H
#define KGPGEXPORT_TEST_H

#include <QObject>
#include <QTemporaryDir>

class KGpgExportTest : public QObject {
	Q_OBJECT
private Q_SLOTS:
	void init();
	void testExportPublicKeyToFile();
	void testExportSecretKeyToFile();
	void testExportPublicKeyToStdOutput();

private:
	QTemporaryDir m_tempdir;
};

#endif
