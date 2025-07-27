/*
    SPDX-FileCopyrightText: 2010, 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpgkeyserversearchtransaction.h"

KGpgKeyserverSearchTransaction::KGpgKeyserverSearchTransaction(QObject *parent, const QString &keyserver, const QString &pattern, const bool withProgress, const QString &proxy)
	: KGpgKeyserverTransaction(parent, keyserver, withProgress, proxy),
	m_pageEmpty(true)
{
	addArgument(QLatin1String( "--with-colons" ));
	addArgument(QLatin1String( "--search-keys" ));
	m_patternPos = addArgument(pattern);
}

KGpgKeyserverSearchTransaction::~KGpgKeyserverSearchTransaction()
{
}

bool
KGpgKeyserverSearchTransaction::preStart()
{
	setSuccess(TS_MSG_SEQUENCE);
	m_keyLines.clear();

	return KGpgKeyserverTransaction::preStart();
}

bool
KGpgKeyserverSearchTransaction::nextLine(const QString &line)
{
	if (line.startsWith(QLatin1String("[GNUPG:] GET_LINE keysearch.prompt"))) {
		if (!m_pageEmpty) {
			write("n");
			m_pageEmpty = true;
		} else {
			return true;
		}
	} else if (!line.isEmpty() && !line.startsWith(QLatin1String("[GNUPG:] "))) {
		m_pageEmpty = false;
		if (line.startsWith(QLatin1String("pub:"))) {
			if (!m_keyLines.isEmpty()) {
				Q_EMIT newKey(m_keyLines);
				m_keyLines.clear();
			}
			m_keyLines.append(line);
		} else if (!m_keyLines.isEmpty() && (line != QLatin1String("\r")))
			m_keyLines.append(line);
	}

	return false;
}

void
KGpgKeyserverSearchTransaction::finish()
{
	if (!m_keyLines.isEmpty()) {
		Q_EMIT newKey(m_keyLines);
		m_keyLines.clear();
	}
}

void
KGpgKeyserverSearchTransaction::setPattern(const QString &pattern)
{
	replaceArgument(m_patternPos, pattern);
}
