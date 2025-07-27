/*
    SPDX-FileCopyrightText: 2010, 2011, 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpgdecrypt.h"

#include "gpgproc.h"
#include "kgpgsettings.h"

#include <KLocalizedString>

KGpgDecrypt::KGpgDecrypt(QObject *parent, const QString &text)
	: KGpgTextOrFileTransaction(parent, text),
	m_fileIndex(-1),
	m_plainLength(-1)
{
}

KGpgDecrypt::KGpgDecrypt(QObject *parent, const QList<QUrl> &files)
	: KGpgTextOrFileTransaction(parent, files),
	m_fileIndex(0),
	m_plainLength(-1)
{
}

KGpgDecrypt::KGpgDecrypt(QObject* parent, const QUrl& infile, const QUrl& outfile)
	: KGpgTextOrFileTransaction(parent, QList<QUrl>({infile})),
	m_fileIndex(0),
	m_plainLength(-1),
	m_outFilename(outfile.toLocalFile())
{
}

KGpgDecrypt::~KGpgDecrypt()
{
}

QStringList
KGpgDecrypt::command() const
{
	QStringList ret;

	ret << QLatin1String("--decrypt") << QLatin1String("--command-fd=0");

	if (!m_outFilename.isEmpty())
		ret << QLatin1String("-o") << m_outFilename;

	ret << KGpgSettings::customDecrypt().simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts);

	return ret;
}

QStringList
KGpgDecrypt::decryptedText() const
{
	QStringList result;
	int txtlength = 0;

	for (const QString &line : getMessages())
		if (!line.startsWith(QLatin1String("[GNUPG:] "))) {
			result.append(line);
			txtlength += line.length() + 1;
		}

	if (result.isEmpty())
		return result;

	QString last = result.last();
	// this may happen when the original text did not end with a newline
	if (last.endsWith(QLatin1String("[GNUPG:] DECRYPTION_OKAY"))) {
		// if GnuPG doesn't tell us the length assume that this happend
		// if it told us the length then check if it _really_ happend
		if (((m_plainLength != -1) && (txtlength != m_plainLength)) ||
				(m_plainLength == -1)) {
			last.chop(24);
			result[result.count() - 1] = last;
		}
	}

	return result;
}

bool
KGpgDecrypt::isEncryptedText(const QString &text, int *startPos, int *endPos)
{
	int posStart = text.indexOf(QLatin1String("-----BEGIN PGP MESSAGE-----"));
	if (posStart == -1)
		return false;

	int posEnd = text.indexOf(QLatin1String("-----END PGP MESSAGE-----"), posStart);
	if (posEnd == -1)
		return false;

	if (startPos != nullptr)
		*startPos = posStart;
	if (endPos != nullptr)
		*endPos = posEnd;

	return true;
}

bool
KGpgDecrypt::nextLine(const QString& line)
{
	const QList<QUrl> &inputFiles = getInputFiles();

	if (line == QLatin1String("[GNUPG:] DECRYPTION_OKAY")) {
		// Assume Gpg decryption was successful even if gpg returns
		// an error code.
		decryptSuccess = true;
	} else if (!inputFiles.isEmpty()) {
		if (line == QLatin1String("[GNUPG:] BEGIN_DECRYPTION")) {
			Q_EMIT statusMessage(i18nc("Status message 'Decrypting <filename>' (operation starts)", "Decrypting %1", inputFiles.at(m_fileIndex).fileName()));
			Q_EMIT infoProgress(2 * m_fileIndex + 1, inputFiles.count() * 2);
		} else if (line == QLatin1String("[GNUPG:] END_DECRYPTION")) {
			Q_EMIT statusMessage(i18nc("Status message 'Decrypted <filename>' (operation was completed)", "Decrypted %1", inputFiles.at(m_fileIndex).fileName()));
			m_fileIndex++;
			Q_EMIT infoProgress(2 * m_fileIndex, inputFiles.count() * 2);
		}
	} else {
		if (line.startsWith(QLatin1String("[GNUPG:] PLAINTEXT_LENGTH "))) {
			bool ok;
			m_plainLength = QStringView(line).mid(26).toInt(&ok);
			if (!ok)
				m_plainLength = -1;
		} else if (line == QLatin1String("[GNUPG:] BEGIN_DECRYPTION")) {
			// close the command channel (if any) to signal GnuPG that it
			// can start sending the output.
			getProcess()->closeWriteChannel();
		}
	}

	return KGpgTextOrFileTransaction::nextLine(line);
}

void
KGpgDecrypt::finish()
{
	if (decryptSuccess) {
		// Gpg error code is ignored.
		// https://bugs.kde.org/show_bug.cgi?id=357462
		setSuccess(TS_OK);
	} else {
		KGpgTextOrFileTransaction::finish();
	}
}

bool KGpgDecrypt::closeInputAfterText() const
{
	// otherwise decryption does never start in GnuPG 2.3,
	// but doesn't hurt on older versions as well
	return true;
}
