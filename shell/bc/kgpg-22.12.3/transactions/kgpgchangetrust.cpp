/*
    SPDX-FileCopyrightText: 2008, 2009, 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpgchangetrust.h"

KGpgChangeTrust::KGpgChangeTrust(QObject *parent, const QString &keyid, const gpgme_validity_t trust)
	: KGpgEditKeyTransaction(parent, keyid, QLatin1String( "trust" ), false)
{
	setTrust(trust);
}

KGpgChangeTrust::~KGpgChangeTrust()
{
}

bool
KGpgChangeTrust::preStart()
{
	setSuccess(TS_MSG_SEQUENCE);

	return true;
}

bool
KGpgChangeTrust::nextLine(const QString &line)
{
	if (line.contains(QLatin1String( "edit_ownertrust.value" ))) {
		write(QByteArray::number(m_trust));
		setSuccess(TS_OK);
	} else {
		return KGpgEditKeyTransaction::nextLine(line);
	}

	return false;
}

KGpgTransaction::ts_boolanswer
KGpgChangeTrust::boolQuestion(const QString& line)
{
	if (line == QLatin1String("edit_ownertrust.set_ultimate.okay")) {
		return BA_YES;
	} else {
		return KGpgTransaction::boolQuestion(line);
	}
}

void
KGpgChangeTrust::setTrust(const gpgme_validity_t trust)
{
	m_trust = trust;
}
