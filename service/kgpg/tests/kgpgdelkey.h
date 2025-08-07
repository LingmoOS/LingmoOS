#ifndef KGPGDELKEY_TEST_H
#define KGPGDELKEY_TEST_H

#include <QObject>
#include <QTemporaryDir>

class KGpgDelKeyTest : public QObject {
	Q_OBJECT
private Q_SLOTS:
	void init();
	void testDeleteKey();
	void testDeleteKey_data();

private:
	QTemporaryDir m_tempdir;
};

#endif
