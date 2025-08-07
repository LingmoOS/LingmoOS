/*
    SPDX-FileCopyrightText: 2009, 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpgsignkey.h"

#include "model/kgpgitemnode.h"

KGpgSignKey::KGpgSignKey(QObject *parent, const QString &signer, KGpgKeyNode *key, const bool local, const carefulCheck checking)
	: KGpgEditKeyTransaction(parent, key->getId(), QString(), false, false),
	KGpgSignTransactionHelper(signer, !local, checking)
{
	insertArgument(1, QLatin1String( "-u" ));
	insertArgument(2, signer);
	m_signerPos = 2;
	addArgumentRef(&m_signerPos);

	addArgument(QLatin1String("save"));

	setKey(key);

	setLocal(local);
}

KGpgSignKey::~KGpgSignKey()
{
}

bool
KGpgSignKey::nextLine(const QString &line)
{
	switch (KGpgSignTransactionHelper::nextLine(line)) {
	case KGpgSignTransactionHelper::handledFalse:
		return false;
	case KGpgSignTransactionHelper::handledTrue:
		return true;
	default:
		// just to keep the compiler happy
		Q_ASSERT(0);
	case KGpgSignTransactionHelper::notHandled:
		return KGpgEditKeyTransaction::nextLine(line);
	}
}

KGpgTransaction::ts_boolanswer
KGpgSignKey::boolQuestion(const QString& line)
{
	if (line.contains(QLatin1String("sign_all.okay")))
		return BA_YES;

	ts_boolanswer ret = KGpgSignTransactionHelper::boolQuestion(line);

	if (ret == BA_UNKNOWN)
		ret = KGpgTransaction::boolQuestion(line);

	return ret;
}

bool
KGpgSignKey::passphraseReceived()
{
	setSuccess(KGpgTransaction::TS_OK);
	return true;
}

KGpgTransaction *
KGpgSignKey::asTransaction()
{
	return this;
}

void
KGpgSignKey::replaceCmd(const QString &cmd)
{
	replaceCommand(cmd);
}
