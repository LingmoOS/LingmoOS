/*
    SPDX-FileCopyrightText: 2008, 2009, 2012, 2016, 2017, 2018 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpgdelkey.h"

#include "gpgproc.h"

#include <QString>
#include <QStringList>

static QStringList keyFingerprints(const KGpgKeyNode::List &keys)
{
	QStringList ret;
	ret.reserve(keys.count());

	for (const KGpgKeyNode *key : keys)
		ret << key->getFingerprint();

	return ret;
}

KGpgDelKey::KGpgDelKey(QObject *parent, const KGpgKeyNode::List &keys)
	: KGpgTransaction(parent)
	, keys(keys)
	, fingerprints(keyFingerprints(keys))
{
	setCmdLine();
	setExpectedFingerprints(fingerprints);
}

KGpgDelKey::~KGpgDelKey()
{
}

bool
KGpgDelKey::nextLine(const QString &line)
{
	if (!line.startsWith(QLatin1String("[GNUPG:] GOT_IT")))
		setSuccess(KGpgTransaction::TS_MSG_SEQUENCE);

	return false;
}

KGpgTransaction::ts_boolanswer
KGpgDelKey::boolQuestion(const QString &line)
{
	if (line.startsWith(QLatin1String("delete_key.okay")))
		return KGpgTransaction::BA_YES;

	if (line.startsWith(QLatin1String("delete_key.secret.okay")))
		return KGpgTransaction::BA_YES;

	return KGpgTransaction::boolQuestion(line);
}

bool
KGpgDelKey::preStart()
{
	GPGProc *proc = getProcess();
	const QStringList args = proc->program() + fingerprints;

	proc->setProgram(args);

	setSuccess(KGpgTransaction::TS_OK);

	return true;
}

void
KGpgDelKey::setCmdLine()
{
	addArguments( { QLatin1String("--status-fd=1"),
			QLatin1String("--command-fd=0"),
			QLatin1String("--delete-secret-and-public-key")
			});

	m_argscount = getProcess()->program().count();
}
