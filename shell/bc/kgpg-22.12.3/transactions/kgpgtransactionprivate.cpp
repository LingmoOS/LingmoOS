/*
    SPDX-FileCopyrightText: 2009, 2010, 2012, 2014, 2016, 2018 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpgtransactionprivate.h"
#include "kgpgtransaction.h"
#include "kgpg_transactions_debug.h"
#include "kgpg_general_debug.h"

#include <QPointer>
#include <QWidget>

#include <KLocalizedString>
#include <KIO/RenameDialog>

KGpgTransactionPrivate::KGpgTransactionPrivate(KGpgTransaction *parent, bool allowChaining)
	: m_parent(parent),
	m_process(new GPGProc()),
	m_inputTransaction(nullptr),
	m_newPasswordDialog(nullptr),
	m_passwordDialog(nullptr),
	m_success(KGpgTransaction::TS_OK),
	m_tries(3),
	m_chainingAllowed(allowChaining),
	m_inputProcessDone(false),
	m_inputProcessResult(KGpgTransaction::TS_OK),
	m_ownProcessFinished(false),
	m_quitTries(0)
{
	connect(m_process, &GPGProc::readReady, this, &KGpgTransactionPrivate::slotReadReady);
	connect(m_process, &GPGProc::processExited, this, &KGpgTransactionPrivate::slotProcessExited);
	connect(m_process, &GPGProc::started, this, &KGpgTransactionPrivate::slotProcessStarted);
}

KGpgTransactionPrivate::~KGpgTransactionPrivate()
{
	if (m_newPasswordDialog) {
		m_newPasswordDialog->close();
		m_newPasswordDialog->deleteLater();
	}
	if (m_process->state() == QProcess::Running) {
		m_process->closeWriteChannel();
		m_process->terminate();
	}
	delete m_inputTransaction;
	delete m_process;
}

void
KGpgTransactionPrivate::slotReadReady()
{
	QString line;
	QPointer<GPGProc> process(m_process);
	QPointer<KGpgTransaction> par(m_parent);

	while (!process.isNull() && (m_process->readln(line, true) >= 0)) {
		if (m_quitTries)
			m_quitLines << line;
#ifdef KGPG_DEBUG_TRANSACTIONS
		qCDebug(KGPG_LOG_TRANSACTIONS) << m_parent << line;
#endif /* KGPG_DEBUG_TRANSACTIONS */

		static const QString getBool = QLatin1String("[GNUPG:] GET_BOOL ");
		if (keyConsidered(line)) {
			// already handled by keyConsidered - skip the line
		} else if (line.startsWith(QLatin1String("[GNUPG:] USERID_HINT "))) {
			m_parent->addIdHint(line);
		} else if (line.startsWith(QLatin1String("[GNUPG:] BAD_PASSPHRASE "))) {
			// the MISSING_PASSPHRASE line comes first, in that case ignore a
			// following BAD_PASSPHRASE
			if (m_success != KGpgTransaction::TS_USER_ABORTED)
				m_success = KGpgTransaction::TS_BAD_PASSPHRASE;
		} else if (line.startsWith(QLatin1String("[GNUPG:] GET_HIDDEN passphrase.enter"))) {
			const bool goOn = m_parent->passphraseRequested();

			// Check if the object was deleted while waiting for the result
			if (!goOn || par.isNull())
				return;

		} else if (line.startsWith(QLatin1String("[GNUPG:] GOOD_PASSPHRASE"))) {
			Q_EMIT m_parent->statusMessage(i18n("Got Passphrase"));

			if (m_passwordDialog != nullptr) {
				m_passwordDialog->close();
				m_passwordDialog->deleteLater();
				m_passwordDialog = nullptr;
			}

			if (m_parent->passphraseReceived()) {
				// signal GnuPG that there will be no further input and it can
				// begin sending output.
				m_process->closeWriteChannel();
			}

		} else if (line.startsWith(getBool)) {
			const QString question = line.mid(getBool.length());

			KGpgTransaction::ts_boolanswer answer;

			if (question.startsWith(QLatin1String("openfile.overwrite.okay"))) {
				m_overwriteUrl.clear();
				answer = m_parent->confirmOverwrite(m_overwriteUrl);

				if ((answer == KGpgTransaction::BA_UNKNOWN) && !m_overwriteUrl.isEmpty()) {
					QPointer<KIO::RenameDialog> over = new KIO::RenameDialog(qobject_cast<QWidget *>(m_parent->parent()),
							i18n("File Already Exists"), QUrl(),
							m_overwriteUrl, KIO::RenameDialog_Overwrite);

					m_overwriteUrl.clear();

					switch (over->exec()) {
					case KIO::Result_Overwrite:
						answer = KGpgTransaction::BA_YES;
						break;
					case KIO::Result_Rename:
						answer = KGpgTransaction::BA_NO;
						m_overwriteUrl = over->newDestUrl();
						break;
					default:
						answer = KGpgTransaction::BA_UNKNOWN;
						m_parent->setSuccess(KGpgTransaction::TS_USER_ABORTED);
						// Close the pipes, otherwise GnuPG will try to answer
						// further questions about this file.
						m_process->closeWriteChannel();
						m_process->closeReadChannel(QProcess::StandardOutput);
						break;
					}

					delete over;

					if (answer == KGpgTransaction::BA_UNKNOWN)
						continue;
				}
			} else {
				answer = m_parent->boolQuestion(question);
			}

			switch (answer) {
			case KGpgTransaction::BA_YES:
				write("YES\n");
				break;
			case KGpgTransaction::BA_NO:
				write("NO\n");
				break;
			case KGpgTransaction::BA_UNKNOWN:
				m_parent->setSuccess(KGpgTransaction::TS_MSG_SEQUENCE);
				m_parent->unexpectedLine(line);
				sendQuit();
			}
		} else if (!m_overwriteUrl.isEmpty() && line.startsWith(QLatin1String("[GNUPG:] GET_LINE openfile.askoutname"))) {
			write(m_overwriteUrl.toLocalFile().toUtf8() + '\n');
			m_overwriteUrl.clear();
		} else if (line.startsWith(QLatin1String("[GNUPG:] MISSING_PASSPHRASE"))) {
			m_success = KGpgTransaction::TS_USER_ABORTED;
		} else if (line.startsWith(QLatin1String("[GNUPG:] CARDCTRL "))) {
			// just ignore them, pinentry should handle that
		} else {
			// all known hints
			int i = 0;
			bool matched = false;
			for (const QString &hintName : hintNames()) {
				const KGpgTransaction::ts_hintType h = static_cast<KGpgTransaction::ts_hintType>(i++);
				if (!line.startsWith(hintName))
					continue;

				matched = true;

				bool r;
				const int skip = hintName.length();
				if (line.length() == skip) {
					r = m_parent->hintLine(h, QString());
				} else {
					r = m_parent->hintLine(h, line.mid(skip + 1).trimmed());
				}

				if (!r) {
					m_parent->setSuccess(KGpgTransaction::TS_MSG_SEQUENCE);
					sendQuit();
				}

				break;
			}

			if (!matched) {
				if (m_parent->nextLine(line))
					sendQuit();
			}
		}
	}
}

void
KGpgTransactionPrivate::slotProcessExited()
{
	Q_ASSERT(sender() == m_process);

	m_ownProcessFinished = true;

	if (m_inputProcessDone)
		processDone();
}

void
KGpgTransactionPrivate::slotProcessStarted()
{
	m_parent->postStart();
}

void
KGpgTransactionPrivate::sendQuit(void)
{
	write("quit\n");

#ifdef KGPG_DEBUG_TRANSACTIONS
	if (m_quitTries == 0)
		qCDebug(KGPG_LOG_TRANSACTIONS) << "sending quit";
#endif /* KGPG_DEBUG_TRANSACTIONS */

	if (m_quitTries++ >= 3) {
		qCDebug(KGPG_LOG_GENERAL) << "tried" << m_quitTries << "times to quit the GnuPG session";
		qCDebug(KGPG_LOG_GENERAL) << "last input was" << m_quitLines;
		qCDebug(KGPG_LOG_GENERAL) << "please file a bug report at https://bugs.kde.org";
		m_process->closeWriteChannel();
		m_success = KGpgTransaction::TS_MSG_SEQUENCE;
	}
}

void
KGpgTransactionPrivate::slotInputTransactionDone(int result)
{
	Q_ASSERT(sender() == m_inputTransaction);

	m_inputProcessDone = true;
	m_inputProcessResult = result;

	if (m_ownProcessFinished)
		processDone();
}

void
KGpgTransactionPrivate::slotPassphraseEntered(const QString &passphrase)
{
	// not calling KGpgTransactionPrivate::write() here for obvious privacy reasons
	m_process->write(passphrase.toUtf8() + '\n');
	if (sender() == m_newPasswordDialog) {
		m_newPasswordDialog->deleteLater();
		m_newPasswordDialog = nullptr;
		m_parent->newPassphraseEntered();
	} else {
		Q_ASSERT(sender() == m_passwordDialog);
	}
}

void
KGpgTransactionPrivate::slotPassphraseAborted()
{
	Q_ASSERT((sender() == m_passwordDialog) ^ (sender() == m_newPasswordDialog));
	sender()->deleteLater();
	m_newPasswordDialog = nullptr;
	m_passwordDialog = nullptr;
	handlePassphraseAborted();
}

void
KGpgTransactionPrivate::handlePassphraseAborted()
{
	// sending "quit" here is useless as it would be interpreted as the passphrase
	m_process->closeWriteChannel();
	m_success = KGpgTransaction::TS_USER_ABORTED;
}

void
KGpgTransactionPrivate::write(const QByteArray &a)
{
	m_process->write(a);
#ifdef KGPG_DEBUG_TRANSACTIONS
	qCDebug(KGPG_LOG_TRANSACTIONS) << m_parent << a;
#endif /* KGPG_DEBUG_TRANSACTIONS */
}

const QStringList &
KGpgTransactionPrivate::hintNames (void)
{
	static QStringList hints;

	if (hints.isEmpty()) {
		hints.insert(KGpgTransaction::HT_KEYEXPIRED,
				QLatin1String("[GNUPG:] KEYEXPIRED"));
		hints.insert(KGpgTransaction::HT_SIGEXPIRED,
				QLatin1String("[GNUPG:] SIGEXPIRED"));
		hints.insert(KGpgTransaction::HT_NOSECKEY,
				QLatin1String("[GNUPG:] NO_SECKEY"));
		hints.insert(KGpgTransaction::HT_ENCTO,
				QLatin1String("[GNUPG:] ENC_TO"));
		hints.insert(KGpgTransaction::HT_PINENTRY_LAUNCHED,
				QLatin1String("[GNUPG:] PINENTRY_LAUNCHED"));
	}

	return hints;
}

void
KGpgTransactionPrivate::processDone()
{
	m_parent->finish();
	Q_EMIT m_parent->infoProgress(100, 100);
	Q_EMIT m_parent->done(m_success);
#ifdef KGPG_DEBUG_TRANSACTIONS
	qCDebug(KGPG_LOG_TRANSACTIONS) << this << "result:" << m_success;
#endif /* KGPG_DEBUG_TRANSACTIONS */
}

bool KGpgTransactionPrivate::keyConsidered(const QString& line)
{
	if (!line.startsWith(QLatin1String("[GNUPG:] KEY_CONSIDERED ")))
		return false;

	const QStringList &parts = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
	if (parts.count() < 3)
		m_parent->setSuccess(KGpgTransaction::TS_MSG_SEQUENCE);
	else if (!m_expectedFingerprints.isEmpty() &&
			!m_expectedFingerprints.contains(parts[2], Qt::CaseInsensitive))
		m_parent->setSuccess(KGpgTransaction::TS_MSG_SEQUENCE);

	return true;
}
