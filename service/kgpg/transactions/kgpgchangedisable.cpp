/*
    SPDX-FileCopyrightText: 2008, 2009, 2012, 2018 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpgchangedisable.h"

KGpgChangeDisable::KGpgChangeDisable(QObject *parent, const QString &keyid, const bool disable)
	: KGpgEditKeyTransaction(parent, keyid, QString(), false)
{
	setDisable(disable);
	setExpectedFingerprints( { keyid } );
}

KGpgChangeDisable::~KGpgChangeDisable()
{
}

void
KGpgChangeDisable::setDisable(bool disable)
{
	QString cmd;
	if (disable)
		cmd = QLatin1String( "disable" );
	else
		cmd = QLatin1String( "enable" );

	replaceCommand(cmd);
}

bool
KGpgChangeDisable::preStart()
{
	if (!KGpgEditKeyTransaction::preStart())
		return false;

	setSuccess(TS_OK);

	return true;
}
