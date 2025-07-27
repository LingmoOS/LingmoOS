#ifndef KGPGADDUID_TEST_H
#define KGPGADDUID_TEST_H

#include <QObject>
#include <QTemporaryDir>

class KGpgAddUidTest : public QObject {
	Q_OBJECT
private Q_SLOTS:
	void init();
	void testAddUid();
	void testAddUid_data();

	void testAddUidInvalid();

private:
	QTemporaryDir m_tempdir;
};

#endif
