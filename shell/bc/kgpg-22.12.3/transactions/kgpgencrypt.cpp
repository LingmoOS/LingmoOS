/*
    SPDX-FileCopyrightText: 2011, 2012, 2013, 2016, 2017 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpgencrypt.h"

#include "kgpgsettings.h"
#include "gpgproc.h"

static QStringList trustOptions(const QString &binary)
{
	const int gpgver = GPGProc::gpgVersion(GPGProc::gpgVersionString(binary));
	QStringList args;
	if (gpgver >= 0x10302)
		args << QLatin1String("--trust-model")
				<< QLatin1String("always");
	else
		args << QLatin1String("--always-trust");

	return args;
}

KGpgEncrypt::KGpgEncrypt(QObject *parent, const QStringList &userIds, const QString &text, const EncryptOptions &options, const QStringList &extraOptions)
	: KGpgTextOrFileTransaction(parent, text),
	m_fileIndex(-1),
	m_options(options),
	m_userIds(userIds),
	m_extraOptions(extraOptions)
{
	if ((m_options & AllowUntrustedEncryption) && !m_userIds.isEmpty())
		m_extraOptions << trustOptions(getProcess()->program().at(0));
}

KGpgEncrypt::KGpgEncrypt(QObject *parent, const QStringList &userIds, const QList<QUrl> &files, const EncryptOptions &options, const QStringList &extraOptions)
	: KGpgTextOrFileTransaction(parent, files),
	m_fileIndex(0),
	m_options(options),
	m_userIds(userIds),
	m_extraOptions(extraOptions)
{
	if ((m_options & AllowUntrustedEncryption) && !m_userIds.isEmpty())
		m_extraOptions << trustOptions(getProcess()->program().at(0));
}

KGpgEncrypt::~KGpgEncrypt()
{
}

QStringList
KGpgEncrypt::command() const
{
	QStringList ret = m_extraOptions;

	if (m_options.testFlag(AsciiArmored))
		ret << QLatin1String("--armor");

	if (m_userIds.isEmpty()) {
		ret << QLatin1String( "--symmetric" );
	} else {
		if (m_options.testFlag(HideKeyId))
			ret << QLatin1String("--throw-keyid");

		ret.reserve(ret.size() + 2 * m_userIds.size() + 1);
		for (const QString &uid : m_userIds)
			ret << QLatin1String( "--recipient" ) << uid;
		ret << QLatin1String( "--encrypt" );
	}

	return ret;
}

QStringList
KGpgEncrypt::encryptedText() const
{
	QStringList result;

	for (const QString &line : getMessages())
		if (!line.startsWith(QLatin1String("[GNUPG:] ")))
			result.append(line);

	return result;
}

bool
KGpgEncrypt::nextLine(const QString &line)
{
	const QList<QUrl> &inputFiles = getInputFiles();

	if (!inputFiles.isEmpty()) {
		static const QString encStart = QLatin1String("[GNUPG:] FILE_START 2 ");
		static const QString encDone = QLatin1String("[GNUPG:] FILE_DONE");

		if (line.startsWith(encStart)) {
			m_currentFile = line.mid(encStart.length());
			Q_EMIT statusMessage(i18nc("Status message 'Encrypting <filename>' (operation starts)", "Encrypting %1", m_currentFile));
			Q_EMIT infoProgress(2 * m_fileIndex + 1, inputFiles.count() * 2);
		} else if (line == encDone) {
			Q_EMIT statusMessage(i18nc("Status message 'Encrypted <filename>' (operation was completed)", "Encrypted %1", m_currentFile));
			m_fileIndex++;
			Q_EMIT infoProgress(2 * m_fileIndex, inputFiles.count() * 2);
		}
	}

	return KGpgTextOrFileTransaction::nextLine(line);
}

KGpgTransaction::ts_boolanswer
KGpgEncrypt::confirmOverwrite(QUrl &currentFile)
{
	const QString ext = encryptExtension(m_options.testFlag(AsciiArmored));

	if (m_currentFile.isEmpty())
		currentFile = QUrl::fromLocalFile(getInputFiles().at(m_fileIndex).toLocalFile() + ext);
	else
		currentFile = QUrl::fromLocalFile(m_currentFile + ext);
	return BA_UNKNOWN;
}


QString
KGpgEncrypt::encryptExtension(const bool ascii)
{
	if (ascii)
		return QLatin1String( ".asc" );
	else if (KGpgSettings::pgpExtension())
		return QLatin1String( ".pgp" );
	else
		return QLatin1String( ".gpg" );
}
