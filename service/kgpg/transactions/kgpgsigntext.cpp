/*
    SPDX-FileCopyrightText: 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpgsigntext.h"

#include "kgpgsettings.h"

KGpgSignText::KGpgSignText(QObject *parent, const QString &signId, const QString &text, const SignOptions &options, const QStringList &extraOptions)
	: KGpgTextOrFileTransaction(parent, text),
	m_fileIndex(-1),
	m_options(options),
	m_signId(signId),
	m_extraOptions(extraOptions)
{
}

KGpgSignText::KGpgSignText(QObject *parent, const QString &signId, const QList<QUrl> &files, const SignOptions &options, const QStringList &extraOptions)
	: KGpgTextOrFileTransaction(parent, files),
	m_fileIndex(0),
	m_options(options),
	m_signId(signId),
	m_extraOptions(extraOptions)
{
	/* GnuPG can only handle one file at a time when signing */
	Q_ASSERT(files.count() == 1);
}

KGpgSignText::~KGpgSignText()
{
}

QStringList
KGpgSignText::command() const
{
	QStringList ret = m_extraOptions;

	const QList<QUrl> &files = getInputFiles();
	QString fileName;

	if (!files.isEmpty())
		fileName = files.first().path();

	ret << QLatin1String("-u") << m_signId;

	if (m_options & AsciiArmored) {
		if (fileName.isEmpty())
			ret << QLatin1String("--clearsign");
		else
			ret << QLatin1String("--armor");
	}
	if (KGpgSettings::pgpCompatibility())
		ret << QLatin1String("--pgp6");

	if (!fileName.isEmpty()) {
		if (m_options & DetachedSignature)
			ret << QLatin1String("--detach-sign") <<
					QLatin1String("--output") << fileName + QLatin1String(".sig");
	}

	return ret;
}

QStringList
KGpgSignText::signedText() const
{
	QStringList result;

	for (const QString &line : getMessages())
		if (!line.startsWith(QLatin1String("[GNUPG:] "))) {
			result.append(line);
		}

	return result;
}
