/*
    SPDX-FileCopyrightText: 2009, 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpgkeyservergettransaction.h"

#include "gpgproc.h"

KGpgKeyserverGetTransaction::KGpgKeyserverGetTransaction(QObject *parent, const QString &keyserver, const QStringList &keys, const bool withProgress, const QString &proxy)
	: KGpgKeyserverTransaction(parent, keyserver, withProgress, proxy)
{
	m_cmdpos = addArgument(QString());
	setKeyIds(keys);
}

KGpgKeyserverGetTransaction::~KGpgKeyserverGetTransaction()
{
}

bool
KGpgKeyserverGetTransaction::preStart()
{
	GPGProc *proc = getProcess();
	QStringList args(proc->program());

	int num = args.count();
	while (num > m_cmdpos)
		args.removeAt(--num);

	args << getGpgCommand() << m_keys;

	proc->setProgram(args);

	setSuccess(TS_MSG_SEQUENCE);

	return KGpgKeyserverTransaction::preStart();
}

bool
KGpgKeyserverGetTransaction::nextLine(const QString &line)
{
	m_log.append(line);

	setSuccess(TS_OK);

	return false;
}

const QStringList &
KGpgKeyserverGetTransaction::getLog() const
{
	return m_log;
}

void
KGpgKeyserverGetTransaction::setKeyIds(const QStringList &keys)
{
	m_keys = keys;
}

KGpgReceiveKeys::KGpgReceiveKeys(QObject *parent, const QString &keyserver, const QStringList &keys, const bool withProgress, const QString &proxy)
	: KGpgKeyserverGetTransaction(parent, keyserver, keys, withProgress, proxy)
{
}

KGpgReceiveKeys::~KGpgReceiveKeys()
{
}

QString
KGpgReceiveKeys::getGpgCommand() const
{
	return QLatin1String( "--recv-keys" );
}

KGpgRefreshKeys::KGpgRefreshKeys(QObject *parent, const QString &keyserver, const QStringList &keys, const bool withProgress, const QString &proxy)
	: KGpgKeyserverGetTransaction(parent, keyserver, keys, withProgress, proxy)
{
}

KGpgRefreshKeys::~KGpgRefreshKeys()
{
}

QString
KGpgRefreshKeys::getGpgCommand() const
{
	return QLatin1String( "--refresh-keys" );
}
