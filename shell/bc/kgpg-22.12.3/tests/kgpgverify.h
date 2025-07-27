#ifndef KGPGVERIFYTEST_H
#define KGPGVERIFYTEST_H

#include <QObject>
#include <QTemporaryDir>

class KGpgVerifyTest: public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void init();
	void testVerifySignedText();
	void testVerifySignedFile();
	void testVerifyReturnMissingKey();
	void testVerifyMissingId();
	void testVerifyReturnBadSignature();

private:
	QTemporaryDir m_tempdir;
};

#endif // KGPGVERIFYTEST_H
