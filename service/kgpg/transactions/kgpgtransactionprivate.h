/*
    SPDX-FileCopyrightText: 2008, 2009, 2012, 2013, 2018 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGTRANSACTIONPRIVATE_H
#define KGPGTRANSACTIONPRIVATE_H

#include "gpgproc.h"

#include <QUrl>

#include <KPasswordDialog>
#include <KNewPasswordDialog>

class KGpgTransaction;

class KGpgTransactionPrivate : public QObject {
public:
	KGpgTransactionPrivate(KGpgTransaction *parent, bool allowChaining);
	~KGpgTransactionPrivate() override;

	KGpgTransaction *m_parent;
	GPGProc *m_process;
	KGpgTransaction *m_inputTransaction;
	KNewPasswordDialog *m_newPasswordDialog;
	KPasswordDialog *m_passwordDialog;
	int m_success;
	int m_tries;
	QString m_description;
	bool m_chainingAllowed;

	QStringList m_idhints;
	QStringList m_expectedFingerprints;

	QUrl m_overwriteUrl;	///< the file to overwrite or it's new name

	void slotReadReady();
	void slotProcessExited();
	void slotProcessStarted();
	void slotInputTransactionDone(int result);
	void slotPassphraseEntered(const QString &passphrase);

	/**
	 * @brief a slot to handle the case that the passphrase entry was aborted by the user
	 *
	 * This will delete the sender as well as do the internal passphrase aborted handling.
	 */
	void slotPassphraseAborted();
	/**
	 * @brief do the internal passphrase aborted handling
	 */
	void handlePassphraseAborted();

	QList<int *> m_argRefs;
	bool m_inputProcessDone;
	int m_inputProcessResult;
	bool m_ownProcessFinished;

	/**
	 * terminate GnuPG session
	 */
	void sendQuit(void);

	void write(const QByteArray &a);

	static const QStringList &hintNames(void);

	/**
	 * @brief handle if this a KEY_CONSIDERED line
	 * @param line the line from GnuPG output
	 * @returns if this is a KEY_CONSIDERED line
	 *
	 * In case this is a KEY_CONSIDERED line (i.e. the return value is true),
	 * but either it was malformed or the given fingerprint does not match any
	 * key in m_expectedFingerprints, the success value will be set to TS_MSG_SEQUENCE.
	 *
	 * @see setExpectedIds
	 */
	bool keyConsidered(const QString &line);

private:
	void processDone();

	unsigned int m_quitTries;	///< how many times we tried to quit
	QStringList m_quitLines;	///< what we received after we tried to quit
};

#endif // KGPGTRANSACTIONPRIVATE_H
