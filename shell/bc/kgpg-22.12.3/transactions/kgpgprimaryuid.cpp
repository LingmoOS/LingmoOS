/*
    SPDX-FileCopyrightText: 2009, 2012, 2013 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpgprimaryuid.h"

#include "model/kgpgitemnode.h"

KGpgPrimaryUid::KGpgPrimaryUid(QObject *parent, KGpgUidNode *uid)
	: KGpgUidTransaction(parent, uid->getParentKeyNode()->getId(), uid->getId())
{
	addArgument(QLatin1String("primary"));
	addArgument(QLatin1String("save"));
}

KGpgPrimaryUid::~KGpgPrimaryUid()
{
}

bool
KGpgPrimaryUid::nextLine(const QString &)
{
	setSuccess(TS_MSG_SEQUENCE);
	return true;
}

bool
KGpgPrimaryUid::passphraseReceived()
{
	setSuccess(TS_OK);
	return KGpgTransaction::passphraseReceived();
}
