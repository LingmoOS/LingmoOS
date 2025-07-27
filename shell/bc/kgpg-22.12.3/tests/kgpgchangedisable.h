#ifndef KGPGCHANGEDISABLE_TEST_H
#define KGPGCHANGEDISABLE_TEST_H

#include <QObject>
#include <QTemporaryDir>

class KGpgChangeDisableTest : public QObject {
	Q_OBJECT
private Q_SLOTS:
	void init();
	void testDisableKey();
	void testEnableKey();

private:
	QTemporaryDir m_tempdir;
};

#endif
