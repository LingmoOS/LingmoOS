/*
    SPDX-FileCopyrightText: 2008, 2009, 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpguidtransaction.h"

KGpgUidTransaction::KGpgUidTransaction(QObject *parent, const QString &keyid, const QString &uid)
	: KGpgTransaction(parent)
{
	addArgument(QLatin1String( "--status-fd=1" ));
	addArgument(QLatin1String( "--command-fd=0" ));
	addArgument(QLatin1String( "--edit-key" ));
	m_keyidpos = addArgument(QString());
	addArgumentRef(&m_keyidpos);
	addArgument(QLatin1String( "uid" ));
	m_uidpos = addArgument(QString());
	addArgumentRef(&m_uidpos);

	setKeyId(keyid);
	setUid(uid);
}

KGpgUidTransaction::~KGpgUidTransaction()
{
}

bool
KGpgUidTransaction::preStart()
{
	setSuccess(TS_MSG_SEQUENCE);

	return true;
}

bool
KGpgUidTransaction::standardCommands(const QString &line)
{
	if (!line.startsWith(QLatin1String("[GNUPG:] ")))
		return false;

	if (line.contains(QLatin1String( "GOOD_PASSPHRASE" ))) {
		setSuccess(TS_MSG_SEQUENCE);
	} else if (line.contains(QLatin1String( "keyedit.prompt" ))) {
		write("save");
		if (getSuccess() == TS_MSG_SEQUENCE)
			setSuccess(TS_OK);
		return true;
	} else if (line.contains(QLatin1String( "GET_" ))) {
		setSuccess(TS_MSG_SEQUENCE);
		return true;
	}

	return false;
}

void
KGpgUidTransaction::setKeyId(const QString &keyid)
{
	m_keyid = keyid;

	replaceArgument(m_keyidpos, keyid);
}

QString
KGpgUidTransaction::getKeyId(void) const
{
	return m_keyid;
}

void
KGpgUidTransaction::setUid(const QString &uid)
{
	m_uid = uid;

	replaceArgument(m_uidpos, uid);
}

void
KGpgUidTransaction::setUid(const unsigned int uid)
{
	setUid(QString::number(uid));
}
