/*
    SPDX-FileCopyrightText: 2008, 2009, 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpgadduid.h"

#include <KEmailAddress>

KGpgAddUid::KGpgAddUid(QObject *parent, const QString &keyid, const QString &name, const QString &email, const QString &comment)
	: KGpgEditKeyTransaction(parent, keyid, QLatin1String("adduid"), false, true)
{
	setName(name);
	setEmail(email);
	setComment(comment);
}

KGpgAddUid::~KGpgAddUid()
{
}

bool
KGpgAddUid::preStart()
{
	if (!KGpgEditKeyTransaction::preStart())
		return false;

	if (!m_email.isEmpty() && !KEmailAddress::isValidSimpleAddress(m_email)) {
		setSuccess(TS_INVALID_EMAIL);
		return false;
	}

	return true;
}

bool
KGpgAddUid::nextLine(const QString &line)
{
	if (!line.startsWith(QLatin1String("[GNUPG:] ")))
		return false;

	if (line.contains(QLatin1String( "GOOD_PASSPHRASE" ))) {
		setSuccess(TS_OK);
	} else if (line.contains(QLatin1String( "keygen.name" ))) {
		write(m_name.toUtf8());
	} else if (line.contains(QLatin1String( "keygen.email" ))) {
		write(m_email.toLatin1());
	} else if (line.contains(QLatin1String( "keygen.comment" ))) {
		write(m_comment.toUtf8());
	} else {
		return KGpgEditKeyTransaction::nextLine(line);
	}

	return false;
}

void
KGpgAddUid::setName(const QString &name)
{
	m_name = name;
}

void
KGpgAddUid::setEmail(const QString &email)
{
	m_email = email;
}

void
KGpgAddUid::setComment(const QString &comment)
{
	m_comment = comment;
}
