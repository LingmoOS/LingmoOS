/*
    SPDX-FileCopyrightText: 2009, 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpgsignuid.h"

#include "gpgproc.h"
#include "model/kgpgitemnode.h"

KGpgSignUid::KGpgSignUid(QObject *parent, const QString &signer, const KGpgSignableNode *uid, const bool local, const carefulCheck checking)
	: KGpgUidTransaction(parent),
	KGpgSignTransactionHelper(signer, !local, checking)
{
	insertArgument(1, QLatin1String( "-u" ));
	insertArgument(2, signer);
	m_signerPos = 2;
	addArgumentRef(&m_signerPos);

	m_cmdPos = addArgument(QString());
	addArgumentRef(&m_cmdPos);

	addArgument(QLatin1String("save"));

	setUid(uid);

	setLocal(local);
}

KGpgSignUid::~KGpgSignUid()
{
}

void
KGpgSignUid::setUid(const KGpgSignableNode *uid)
{
	switch (uid->getType()) {
	case ITYPE_PUBLIC:
	case ITYPE_PAIR:
		KGpgUidTransaction::setUid(1);
		setKey(uid->toKeyNode());
		break;
	case ITYPE_UAT:
	case ITYPE_UID:
		KGpgUidTransaction::setUid(uid->getId());
		setKey(uid->getParentKeyNode()->toKeyNode());
		break;
	default:
		Q_ASSERT(0);
	}

	setKeyId(getKey()->getId());
}

bool
KGpgSignUid::preStart()
{
	if (!KGpgUidTransaction::preStart())
		return false;

	/* GnuPG >= 2.1 do not send a GOOD_PASSPHRASE anymore which could be used as a trigger that
	 * everything worked fine, so just assume everything is OK until an ERROR is received. */
	if (GPGProc::gpgVersion(GPGProc::gpgVersionString(QString())) > 0x20100)
		setSuccess(TS_OK);

	return true;
}

bool
KGpgSignUid::nextLine(const QString &line)
{
	switch (KGpgSignTransactionHelper::nextLine(line)) {
	case KGpgSignTransactionHelper::handledFalse:
		return false;
	case KGpgSignTransactionHelper::handledTrue:
		return true;
	default:
		Q_ASSERT(0);
	case KGpgSignTransactionHelper::notHandled:
		if (line.contains(QLatin1String(" ERROR keysig "))) {
			setSuccess(TS_BAD_PASSPHRASE);
			return true;
		} else if (line.contains(QLatin1String(" ERROR "))) {
			setSuccess(TS_MSG_SEQUENCE);
			return true;
		} else {
			return standardCommands(line);
		}
	}
}

KGpgTransaction::ts_boolanswer
KGpgSignUid::boolQuestion(const QString& line)
{
	ts_boolanswer ret = KGpgSignTransactionHelper::boolQuestion(line);

	if (ret == BA_UNKNOWN)
		ret = KGpgTransaction::boolQuestion(line);

	return ret;
}

bool
KGpgSignUid::passphraseReceived()
{
	setSuccess(KGpgTransaction::TS_OK);
	return true;
}

KGpgTransaction *
KGpgSignUid::asTransaction()
{
	return this;
}

void
KGpgSignUid::replaceCmd(const QString &cmd)
{
	replaceArgument(m_cmdPos, cmd);
}
