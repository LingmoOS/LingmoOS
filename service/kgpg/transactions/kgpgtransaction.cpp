/*
    SPDX-FileCopyrightText: 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2016, 2018 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpgtransaction.h"
#include "kgpg_transactions_debug.h"
#include "kgpg_general_debug.h"
#include "kgpgtransactionprivate.h"

#include "gpgproc.h"
#include "kgpginterface.h"

#include <QByteArray>

#include <QUrl>
#include <QWidget>

#include <KConfigGroup>
#include <KNewPasswordDialog>
#include <KLocalizedString>
#include <KPasswordDialog>

KGpgTransaction::KGpgTransaction(QObject *parent, const bool allowChaining)
	: QObject(parent),
	d(new KGpgTransactionPrivate(this, allowChaining))
{
}

KGpgTransaction::~KGpgTransaction()
{
	delete d;
}

void
KGpgTransaction::start()
{
	d->m_inputProcessResult = false;
	d->m_inputProcessDone = (d->m_inputTransaction == nullptr);

	setSuccess(TS_OK);
	d->m_idhints.clear();
	d->m_tries = 3;
	if (preStart()) {
		d->m_ownProcessFinished = false;
		if (d->m_inputTransaction != nullptr)
			d->m_inputTransaction->start();
#ifdef KGPG_DEBUG_TRANSACTIONS
		qCDebug(KGPG_LOG_TRANSACTIONS) << this << d->m_process->program();
#endif /* KGPG_DEBUG_TRANSACTIONS */
		d->m_process->start();
		Q_EMIT infoProgress(0, 1);
	} else {
		Q_EMIT done(d->m_success);
	}
}

void
KGpgTransaction::write(const QByteArray &a, const bool lf)
{
	if (lf)
		d->write(a + '\n');
	else
		d->write(a);
}

void
KGpgTransaction::write(const int i)
{
	write(QByteArray::number(i));
}

void
KGpgTransaction::askNewPassphrase(const QString& text)
{
	Q_EMIT statusMessage(i18n("Requesting Passphrase"));

	d->m_newPasswordDialog = new KNewPasswordDialog(qobject_cast<QWidget *>(parent()));
	d->m_newPasswordDialog->setPrompt(text);
	d->m_newPasswordDialog->setAllowEmptyPasswords(false);
	connect(d->m_newPasswordDialog, &KNewPasswordDialog::newPassword, d, &KGpgTransactionPrivate::slotPassphraseEntered);
	connect(d->m_newPasswordDialog, &KNewPasswordDialog::rejected, d, &KGpgTransactionPrivate::slotPassphraseAborted);
	connect(d->m_process, &GPGProc::processExited, d->m_newPasswordDialog, &KNewPasswordDialog::rejected);
	d->m_newPasswordDialog->show();
}

int
KGpgTransaction::getSuccess() const
{
	return d->m_success;
}

void
KGpgTransaction::setSuccess(const int v)
{
#ifdef KGPG_DEBUG_TRANSACTIONS
	qCDebug(KGPG_LOG_TRANSACTIONS) << "old" << d->m_success << "new" << v;
#endif /* KGPG_DEBUG_TRANSACTIONS */
	d->m_success = v;
}

KGpgTransaction::ts_boolanswer
KGpgTransaction::boolQuestion(const QString& line)
{
	Q_UNUSED(line)

	return BA_UNKNOWN;
}

KGpgTransaction::ts_boolanswer
KGpgTransaction::confirmOverwrite(QUrl &currentFile)
{
	Q_UNUSED(currentFile)

	return BA_UNKNOWN;
}

bool
KGpgTransaction::hintLine(const ts_hintType hint, const QString &args)
{
	switch (hint) {
	case HT_KEYEXPIRED:
	case HT_PINENTRY_LAUNCHED:
		return !args.isEmpty();
	default:
		return true;
	}
}

void
KGpgTransaction::finish()
{
}

void
KGpgTransaction::setDescription(const QString &description)
{
	d->m_description = description;
}

void
KGpgTransaction::waitForInputTransaction()
{
	Q_ASSERT(d->m_inputTransaction != nullptr);

	if (d->m_inputProcessDone)
		return;

	d->m_inputTransaction->waitForFinished();
}

void
KGpgTransaction::unexpectedLine(const QString &line)
{
	qCDebug(KGPG_LOG_GENERAL) << this << "unexpected input line" << line << "for command" << d->m_process->program();
}

bool
KGpgTransaction::passphraseRequested()
{
	return askPassphrase();
}

bool
KGpgTransaction::passphraseReceived()
{
	return true;
}

bool
KGpgTransaction::preStart()
{
	return true;
}

void
KGpgTransaction::postStart()
{
}

void
KGpgTransaction::addIdHint(QString txt)
{
	int cut = txt.indexOf(QLatin1Char( ' ' ), 22, Qt::CaseInsensitive);
	txt.remove(0, cut);

	if (txt.contains(QLatin1Char( '(' ), Qt::CaseInsensitive))
		txt = txt.section(QLatin1Char( '(' ), 0, 0) + txt.section(QLatin1Char( ')' ), -1);

	txt.replace(QLatin1Char( '<' ), QLatin1String( "&lt;" ));

	if (!d->m_idhints.contains(txt))
		d->m_idhints << txt;
}

QString
KGpgTransaction::getIdHints() const
{
	return d->m_idhints.join( i18n(" or " ));
}

GPGProc *
KGpgTransaction::getProcess()
{
	return d->m_process;
}

int
KGpgTransaction::addArgument(const QString &arg)
{
	int r = d->m_process->program().count();

	*d->m_process << arg;

	return r;
}

void
KGpgTransaction::addArguments(const QStringList &args)
{
	*d->m_process << args;
}

void
KGpgTransaction::replaceArgument(const int pos, const QString &arg)
{
	QStringList args(d->m_process->program());
	d->m_process->clearProgram();

	args.replace(pos, arg);

	d->m_process->setProgram(args);
}

void
KGpgTransaction::insertArgument(const int pos, const QString &arg)
{
	insertArguments(pos, QStringList(arg));
}

void
KGpgTransaction::insertArguments(const int pos, const QStringList &args)
{
	QStringList tmp(d->m_process->program());

	int tmppos = pos;
	for (const QString &s : args) {
		tmp.insert(tmppos++, s);
	}
	d->m_process->setProgram(tmp);

	int move = args.count();
	for (int *ref : qAsConst(d->m_argRefs)) {
		if (*ref >= pos)
			*ref += move;
	}
}

void
KGpgTransaction::addArgumentRef(int *ref)
{
	d->m_argRefs.append(ref);
}

bool
KGpgTransaction::askPassphrase(const QString &message)
{
	Q_EMIT statusMessage(i18n("Requesting Passphrase"));

	if (d->m_passwordDialog == nullptr) {
		d->m_passwordDialog = new KPasswordDialog(qobject_cast<QWidget *>(parent()));

		QString passdlgmessage;
		if (message.isEmpty()) {
			QString userIDs(getIdHints());
			if (userIDs.isEmpty())
				userIDs = i18n("[No user id found]");
			else
				userIDs.replace(QLatin1Char( '<' ), QLatin1String( "&lt;" ));

			passdlgmessage = i18n("Enter passphrase for <b>%1</b>", userIDs);
		} else {
			passdlgmessage = message;
		}

		d->m_passwordDialog->setPrompt(passdlgmessage);

		connect(d->m_passwordDialog, &KPasswordDialog::gotPassword, d, &KGpgTransactionPrivate::slotPassphraseEntered);
		connect(d->m_passwordDialog, &KPasswordDialog::rejected, d, &KGpgTransactionPrivate::slotPassphraseAborted);
		connect(d->m_process, &GPGProc::processExited, d->m_passwordDialog, &KPasswordDialog::rejected);
	} else {
		// we already have a dialog, so this is a "bad passphrase" situation
		--d->m_tries;

		d->m_passwordDialog->showErrorMessage(i18np("<p><b>Bad passphrase</b>. You have 1 try left.</p>",
				"<p><b>Bad passphrase</b>. You have %1 tries left.</p>", d->m_tries),
				KPasswordDialog::PasswordError);
	}

	d->m_passwordDialog->show();

	return true;
}

void
KGpgTransaction::setExpectedFingerprints(const QStringList &fingerprints)
{
	d->m_expectedFingerprints = fingerprints;
}

void
KGpgTransaction::setGnuPGHome(const QString &home)
{
	QStringList tmp(d->m_process->program());

	Q_ASSERT(tmp.count() > 3);
	int homepos = tmp.indexOf(QLatin1String("--options"), 1);
	if (homepos == -1)
		homepos = tmp.indexOf(QLatin1String("--homedir"), 1);
	Q_ASSERT(homepos != -1);
	Q_ASSERT(homepos + 1 < tmp.count());

	tmp[homepos] = QLatin1String("--homedir");
	tmp[homepos + 1] = home;

	d->m_process->setProgram(tmp);
}

int
KGpgTransaction::waitForFinished(const int msecs)
{
	int ret = TS_OK;

	if (d->m_inputTransaction != nullptr) {
		int iret = d->m_inputTransaction->waitForFinished(msecs);
		if ((iret != TS_OK) && (msecs != -1))
			return iret;
	}

	bool b = d->m_process->waitForFinished(msecs);

	if (ret != TS_OK)
		return ret;

	if (!b)
		return TS_USER_ABORTED;
	else
		return getSuccess();
}

const QString &
KGpgTransaction::getDescription() const
{
	return d->m_description;
}

void
KGpgTransaction::setInputTransaction(KGpgTransaction *ta)
{
	Q_ASSERT(d->m_chainingAllowed);

	if (d->m_inputTransaction != nullptr)
		clearInputTransaction();
	d->m_inputTransaction = ta;

	GPGProc *proc = ta->getProcess();
	proc->setStandardOutputProcess(d->m_process);
	connect(ta, &KGpgTransaction::done, d, &KGpgTransactionPrivate::slotInputTransactionDone);
}

void
KGpgTransaction::clearInputTransaction()
{
	disconnect(d->m_inputTransaction, &KGpgTransaction::done, d, &KGpgTransactionPrivate::slotInputTransactionDone);
	d->m_inputTransaction = nullptr;
}

bool
KGpgTransaction::hasInputTransaction() const
{
	return (d->m_inputTransaction != nullptr);
}

void
KGpgTransaction::kill()
{
	d->m_process->kill();
}

void
KGpgTransaction::newPassphraseEntered()
{
}
