/*
    SPDX-FileCopyrightText: 2010, 2012, 2013, 2017, 2018 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpggeneraterevoke.h"
#include "kgpg_general_debug.h"

#include <KLocalizedString>


#include <QFile>

KGpgGenerateRevoke::KGpgGenerateRevoke(QObject *parent, const QString &keyID, const QUrl &revokeUrl, const int reason, const QString &description)
	: KGpgTransaction(parent),
	m_keyid(keyID),
	m_revUrl(revokeUrl),
	m_reason(reason),
	m_description(description)
{
	QStringList args = { QLatin1String("--status-fd=1"),
			QLatin1String("--command-fd=0"),
			QLatin1String("--no-verbose")
			};

	if (!revokeUrl.isEmpty())
		args << QLatin1String("-o") << revokeUrl.toLocalFile();

	args << QLatin1String("--gen-revoke") << keyID;
	addArguments(args);
}

KGpgGenerateRevoke::~KGpgGenerateRevoke()
{
}

bool
KGpgGenerateRevoke::preStart()
{
	setSuccess(TS_OK);

	setDescription(i18n("Generating Revocation Certificate for key %1", m_keyid));

	return true;
}

bool
KGpgGenerateRevoke::nextLine(const QString &line)
{
	if (!line.startsWith(QLatin1String("[GNUPG:] "))) {
		m_output.append(line + QLatin1Char( '\n' ));
		return false;
	}

	if (line.contains(QLatin1String( "NEED_PASSPHRASE" ))) {
		setSuccess(TS_USER_ABORTED);
	} else if (line.contains(QLatin1String( "ask_revocation_reason.code" ))) {
		write(QByteArray::number(m_reason));
	} else if (line.contains(QLatin1String( "ask_revocation_reason.text" ))) {
		write(m_description.toUtf8());
		// GnuPG stops asking if we pass an empty line
		m_description.clear();
	} else if (line.contains(QLatin1String( "GET_" ))) {
		setSuccess(TS_MSG_SEQUENCE);
		qCDebug(KGPG_LOG_GENERAL) << line;
		return true;
	}

	return false;
}

KGpgTransaction::ts_boolanswer
KGpgGenerateRevoke::boolQuestion(const QString& line)
{
	if (line == QLatin1String("gen_revoke.okay")) {
		return BA_YES;
	} else if (line == QLatin1String("ask_revocation_reason.okay")) {
		return BA_YES;
	} else {
		return KGpgTransaction::boolQuestion(line);
	}
}

void
KGpgGenerateRevoke::finish()
{
	if (getSuccess() == TS_OK) {
		if (!m_revUrl.isEmpty()) {
			QFile of(m_revUrl.toLocalFile());
			if (of.open(QIODevice::ReadOnly)) {
				m_output = QLatin1String( of.readAll() );
				of.close();
			}
		}
		Q_EMIT revokeCertificate(m_output);
	}
}

bool
KGpgGenerateRevoke::passphraseReceived()
{
	return false;
}

KGpgTransaction::ts_boolanswer
KGpgGenerateRevoke::confirmOverwrite(QUrl &currentFile)
{
	currentFile = m_revUrl;
	return BA_UNKNOWN;
}

const QString &
KGpgGenerateRevoke::getOutput() const
{
	return m_output;
}
