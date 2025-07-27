/*
    SPDX-FileCopyrightText: 2012, 2013, 2017 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpgverify.h"

#include "gpgproc.h"
#include "core/KGpgKeyNode.h"
#include "model/kgpgitemmodel.h"

#include <QLocale>
#include <QRegularExpression>

#include <KLocalizedString>

KGpgVerify::KGpgVerify(QObject *parent, const QString &text)
	: KGpgTextOrFileTransaction(parent, text),
	m_fileIndex(-1)
{
}

KGpgVerify::KGpgVerify(QObject *parent, const QList<QUrl> &files)
	: KGpgTextOrFileTransaction(parent, files),
	m_fileIndex(0)
{
}

KGpgVerify::~KGpgVerify()
{
}

QStringList
KGpgVerify::command() const
{
	QStringList ret(QLatin1String("--verify"));

	return ret;
}

bool
KGpgVerify::nextLine(const QString &line)
{
	if (line.startsWith(QLatin1String("[GNUPG:] NO_PUBKEY "))) {
		setSuccess(TS_MISSING_KEY);
		m_missingId = line.mid(19).simplified();
		return false;
	}

	if (line.startsWith(QLatin1String("[GNUPG:] ")) &&
			line.contains(QLatin1String("SIG"))) {
		if (line.startsWith(QLatin1String("[GNUPG:] BADSIG")))
			setSuccess(KGpgVerify::TS_BAD_SIGNATURE);
		else
			setSuccess(KGpgTransaction::TS_OK);
	}

	return KGpgTextOrFileTransaction::nextLine(line);
}

void
KGpgVerify::finish()
{
	// GnuPG will return error code 2 if it wasn't able to verify the file.
	// If it complained about a missing signature before that is fine.
	if (((getProcess()->exitCode() == 2) && (getSuccess() == TS_MISSING_KEY)) ||
			((getProcess()->exitCode() == 1) && (getSuccess() == TS_BAD_SIGNATURE)))
		return;

	KGpgTextOrFileTransaction::finish();
}

static QString
sigTimeMessage(const QString &sigtime)
{
	QDateTime stamp;
	if (sigtime.contains(QLatin1Char('T'))) {
		stamp = QDateTime::fromString(sigtime, Qt::ISODate);
	} else {
		bool ok;
		qint64 secs = sigtime.toLongLong(&ok);
		if (ok)
			stamp = QDateTime::fromMSecsSinceEpoch(secs * 1000);
	}

	if (!stamp.isValid())
		return QString();

	return xi18nc("@info first argument is formatted date, second argument is formatted time",
					"<para>The signature was created at %1 %2</para>",
					QLocale().toString(stamp.date(), QLocale::ShortFormat),
					QLocale().toString(stamp.time(), QLocale::ShortFormat));
}

QString
KGpgVerify::getReport(const QStringList &log, const KGpgItemModel *model)
{
	QString result;
	// newer versions of GnuPG emit both VALIDSIG and GOODSIG
	// for a good signature. Since VALIDSIG has more information
	// we use that.
	const QRegularExpression validsig(QStringLiteral("^\\[GNUPG:\\] VALIDSIG([ ]+[^ ]+){10,}.*$"));
	const bool useGoodSig = (model == nullptr) || (log.indexOf(validsig) == -1);
	QString sigtime;	// timestamp of signature creation

	for (const QString &line : log) {
		if (!line.startsWith(QLatin1String("[GNUPG:] ")))
			continue;

		const QString msg = line.mid(9);

		if (!useGoodSig && msg.startsWith(QLatin1String("VALIDSIG "))) {
			// from GnuPG source, doc/DETAILS:
			//   VALIDSIG    <fingerprint in hex> <sig_creation_date> <sig-timestamp>
			//                <expire-timestamp> <sig-version> <reserved> <pubkey-algo>
			//                <hash-algo> <sig-class> <primary-key-fpr>
			const QStringList vsig = msg.mid(9).split(QLatin1Char(' '), Qt::SkipEmptyParts);
			Q_ASSERT(vsig.count() >= 10);

			const KGpgKeyNode *node = model->findKeyNode(vsig[9]);

			if (node != nullptr) {
				// ignore for now if this is signed with the primary id (vsig[0] == vsig[9]) or not
				if (node->getEmail().isEmpty())
					result += xi18nc("@info Good signature from: NAME , Key ID: HEXID",
							"<para>Good signature from:<nl/><emphasis strong='true'>%1</emphasis><nl/>Key ID: %2<nl/></para>",
							node->getName(), vsig[9]);
				else
					result += xi18nc("@info Good signature from: NAME <EMAIL>, Key ID: HEXID",
							"<para>Good signature from:<nl/><emphasis strong='true'>%1 <email>%2</email></emphasis><nl/>Key ID: %3</para>",
							node->getName(), node->getEmail(), vsig[9]);

				result += sigTimeMessage(vsig[2]);
			} else {
				// this should normally never happen, but one could delete
				// the key just after the verification. Brute force solution:
				// do the whole report generation again, but this time make
				// sure GOODSIG is used.
				return getReport(log, nullptr);
			}
		} else if (msg.startsWith(QLatin1String("UNEXPECTED")) ||
				msg.startsWith(QLatin1String("NODATA"))) {
			result += xi18nc("@info", "No signature found.") + QLatin1Char('\n');
		} else if (useGoodSig && msg.startsWith(QLatin1String("GOODSIG "))) {
            int sigpos = msg.indexOf( QLatin1Char(' ') , 8);
			const QString keyid = msg.mid(8, sigpos - 8);

			// split the name/email pair to give translators more power to handle this
			QString email;
			QString name = msg.mid(sigpos + 1);

			int oPos = name.indexOf(QLatin1Char('<'));
			int cPos = name.indexOf(QLatin1Char('>'));
			if ((oPos >= 0) && (cPos >= 0)) {
				email = name.mid(oPos + 1, cPos - oPos - 1);
				name = name.left(oPos).simplified();
			}

			if (email.isEmpty())
				result += xi18nc("@info", "<para>Good signature from:<nl/><emphasis strong='true'>%1</emphasis><nl/>Key ID: %2<nl/></para>",
						name, keyid);
			else
				result += xi18nc("@info Good signature from: NAME <EMAIL>, Key ID: HEXID",
						"<para>Good signature from:<nl/><emphasis strong='true'>%1 <email>%2</email></emphasis><nl/>Key ID: %3<nl/></para>",
						name, email, keyid);
			if (!sigtime.isEmpty()) {
				result += sigTimeMessage(sigtime);
				sigtime.clear();
			}
		} else if (msg.startsWith(QLatin1String("SIG_ID "))) {
			const QStringList parts = msg.simplified().split(QLatin1Char(' '));
			if (parts.count() > 2)
				sigtime = parts[2];
		} else if (msg.startsWith(QLatin1String("BADSIG"))) {
            int sigpos = msg.indexOf( QLatin1Char(' '), 7);
			result += xi18nc("@info", "<para><emphasis strong='true'>BAD signature</emphasis> from:<nl/> %1<nl/>Key ID: %2<nl/><nl/><emphasis strong='true'>The file is corrupted</emphasis></para>",
					msg.mid(sigpos + 1), msg.mid(7, sigpos - 7));
		} else  if (msg.startsWith(QLatin1String("TRUST_UNDEFINED"))) {
			result += xi18nc("@info", "<para>The signature is valid, but the key is untrusted</para>");
		} else if (msg.startsWith(QLatin1String("TRUST_ULTIMATE"))) {
			result += xi18nc("@info", "<para>The signature is valid, and the key is ultimately trusted</para>");
		}
	}

	return result;
}

QString
KGpgVerify::missingId() const
{
	return m_missingId;
}
