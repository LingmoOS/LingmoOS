/*
    SPDX-FileCopyrightText: 2008, 2009, 2010, 2011, 2012, 2013, 2017, 2018 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpgchangepass.h"

#include <KLocalizedString>

KGpgChangePass::KGpgChangePass(QObject *parent, const QString &keyid)
	: KGpgTransaction(parent),
	m_seenold(false)
{
	addArguments( { QLatin1String("--status-fd=1"),
			QLatin1String("--command-fd=0"),
			QLatin1String("--edit-key"),
			keyid,
			QLatin1String("passwd")
			} );
}

KGpgChangePass::~KGpgChangePass()
{
}

bool
KGpgChangePass::preStart()
{
	setSuccess(TS_MSG_SEQUENCE);

	return true;
}

bool
KGpgChangePass::nextLine(const QString &line)
{
	if (!line.startsWith(QLatin1String("[GNUPG:] ")))
		return false;

	if (line.contains(QLatin1String( "keyedit.prompt" ))) {
		if (m_seenold && (getSuccess() != TS_USER_ABORTED))
			setSuccess(TS_OK);
		// no need to save, change is automatically saved by GnuPG
		return true;
	} else if (line.contains(QLatin1String( "GET_" ))) {
		setSuccess(TS_MSG_SEQUENCE);
		return true;
	}

	return false;
}

bool
KGpgChangePass::passphraseRequested()
{
	const QString userIDs = getIdHints();

	if (!m_seenold) {
		return askPassphrase(i18n("Enter old passphrase for <b>%1</b>", userIDs));
	} else {
		askNewPassphrase(i18n("<qt>Enter new passphrase for <b>%1</b><br />If you forget this passphrase all your encrypted files and messages will be inaccessible.</qt>", userIDs));
	}

	return true;
}

bool
KGpgChangePass::passphraseReceived()
{
	m_seenold = true;
	setSuccess(TS_MSG_SEQUENCE);
	return false;
}

bool
KGpgChangePass::hintLine(const KGpgTransaction::ts_hintType hint, const QString &args)
{
	switch (hint) {
	case HT_PINENTRY_LAUNCHED:
		// If pinentry message appear 2 should be seen: the one asking for the old password,
		// and one asking for the new. So the old password has successfully been given if the
		// second one appears. BUT: if the user cancels one of these boxes they will reappear
		// up to 3 times, so even the third one could be the one asking for the old password.
		// Simply assume that everything is fine when pinentry is used, which will make the
		// result being set to TS_OK once keyedit.prompt is received.
		m_seenold = true;
		return true;
	default:
		return KGpgTransaction::hintLine(hint, args);
	}
}
