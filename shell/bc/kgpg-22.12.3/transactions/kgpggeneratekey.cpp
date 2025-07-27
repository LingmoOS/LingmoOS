/*
    SPDX-FileCopyrightText: 2008-2022 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpggeneratekey.h"

#include "gpgproc.h"

#include <KEmailAddress>
#include <KLocalizedString>
#include <QApplication>

KGpgGenerateKey::KGpgGenerateKey(QObject *parent, const QString &name, const QString &email, const QString &comment,
		const KgpgCore::KgpgKeyAlgo algorithm, const uint size, const unsigned int expire,
		const char expireunit, const KgpgCore::KgpgSubKeyType capabilities)
	: KGpgTransaction(parent),
	m_name(name),
	m_email(email),
	m_comment(comment),
	m_algorithm(algorithm),
	m_capabilities(capabilities),
	m_size(size),
	m_expire(expire),
	m_expireunit(expireunit)
{
	Q_ASSERT((expireunit == 'd') || (expireunit == 'w') ||
			(expireunit == 'm') || (expireunit == 'y'));

	addArguments( { QLatin1String("--status-fd=1"),
			QLatin1String("--command-fd=0"),
			QLatin1String("--no-verbose"),
			QLatin1String("--gen-key"),
			QLatin1String("--batch")
			} );

	getProcess()->setOutputChannelMode(KProcess::SeparateChannels);
}

KGpgGenerateKey::~KGpgGenerateKey()
{
}

bool
KGpgGenerateKey::preStart()
{
	if (!m_email.isEmpty() && !KEmailAddress::isValidSimpleAddress(m_email)) {
		setSuccess(TS_INVALID_EMAIL);
		return false;
	}

	m_fingerprint.clear();

	setSuccess(TS_MSG_SEQUENCE);

	setDescription(i18n("Generating New Key for %1", m_name));

	return true;
}

void
KGpgGenerateKey::postStart()
{
	QByteArray keymessage = "Key-Type: ";

	switch (m_algorithm) {
	case KgpgCore::ALGO_RSA:
		keymessage.append("RSA");
		break;
	case KgpgCore::ALGO_RSA_RSA:
		keymessage.append("RSA\nSubkey-Type: RSA");
		break;
	case KgpgCore::ALGO_DSA_ELGAMAL:
		keymessage.append("DSA\nSubkey-Type: ELG-E");
		break;
	default:
		Q_ASSERT(m_algorithm == KgpgCore::ALGO_RSA);
		return;
	}

	const QByteArray keylen = QByteArray::number(m_size);

	keymessage.append("\nKey-Length: ");
	keymessage.append(keylen);
	keymessage.append("\nSubkey-Length: ");
	keymessage.append(keylen);
	keymessage.append("\nName-Real: ");
	keymessage.append(m_name.toUtf8());

	if (!m_email.isEmpty()) {
		keymessage.append("\nName-Email: ");
		keymessage.append(m_email.toLatin1());
	}

	if (!m_comment.isEmpty()) {
		keymessage.append("\nName-Comment: ");
		keymessage.append(m_comment.toUtf8());
	}

	if (m_expire != 0) {
		keymessage.append("\nExpire-Date: ");
		keymessage.append(QByteArray::number(m_expire));
		keymessage.append(m_expireunit);
	}

	if (m_capabilities) {
		keymessage.append("\nKey-Usage: ");
		QStringList usage;
#if 0
		// GnuPG always adds cert, but it does not allow this to be
		// explicitly specified
		if (m_capabilities & KgpgCore::SKT_CERTIFICATION)
			usage << QLatin1String("cert");
#endif
		if (m_capabilities & KgpgCore::SKT_AUTHENTICATION)
			usage << QLatin1String("auth");
		if (m_capabilities & KgpgCore::SKT_ENCRYPTION)
			usage << QLatin1String("encrypt");
		if (m_capabilities & KgpgCore::SKT_SIGNATURE)
			usage << QLatin1String("sign");
		keymessage.append(usage.join(QLatin1Char(' ')).toLatin1());
	}

	keymessage.append("\nPassphrase: ");
	write(keymessage, false);

	QString passdlgmessage;
	if (!m_email.isEmpty()) {
		passdlgmessage = i18n("<p><b>Enter passphrase for %1 &lt;%2&gt;</b>:<br />Passphrase should include non alphanumeric characters and random sequences.</p>",
				m_name, m_email);
	} else {
		passdlgmessage = i18n("<p><b>Enter passphrase for %1</b>:<br />Passphrase should include non alphanumeric characters and random sequences.</p>",
				m_name);
	}

	QApplication::restoreOverrideCursor();
	askNewPassphrase(passdlgmessage);
}

bool
KGpgGenerateKey::nextLine(const QString &line)
{
	QString msg = i18n("Generating Key");

	if (!line.startsWith(QLatin1String("[GNUPG:] ")))
		return false;

	int result = false;

	if (line.contains(QLatin1String( "PROGRESS" ))) {
		const QStringList parts = line.mid(18).split(QLatin1Char(' '));

		if (parts.count() >= 4) {
			const QString p0(parts.at(0));
			if (p0 == QLatin1String( "primegen" )) {
				msg = i18n("Generating prime numbers");
			} else if (p0 == QLatin1String( "pk_dsa" )) {
				msg = i18n("Generating DSA key");
			} else if (p0 == QLatin1String( "pk_elg" )) {
				msg = i18n("Generating ElGamal key");
			} else if (p0 == QLatin1String( "need_entropy" )) {
				msg = i18n("Waiting for entropy");

				// This message is currenlty not displayed. Nevertheless it's
				// included here so string freeze is not broken if it will be
				// displayed later on.
				QString msglong = i18n("The entropy pool ran empty. The key generation process is stalled until enough entropy is present. You can generate entropy e.g. by moving the mouse or typing at the keyboard. The easiest way is by using another application until the key generation continues.");
			}
			if (parts.at(3) != QLatin1String( "0" ))
				Q_EMIT infoProgress(parts.at(2).toUInt(), parts.at(3).toUInt());
		}
	} else if (line.contains(QLatin1String( "GOOD_PASSPHRASE" ))) {
		setSuccess(TS_MSG_SEQUENCE);
	} else if (line.contains(QLatin1String( "KEY_CREATED" ))) {
		m_fingerprint = line.right(40);
		setSuccess(TS_OK);
		result = true;
	} else if (line.contains(QLatin1String( "NEED_PASSPHRASE" ))) {
		setSuccess(TS_USER_ABORTED);
	} else if (line.contains(QLatin1String( "GET_" ))) {
		setSuccess(TS_MSG_SEQUENCE);
		result = true;
	} else if (line.contains(QLatin1String("KEY_NOT_CREATED"))) {
		result = true;
	} else
		m_errorOutput << line;

	Q_EMIT statusMessage(msg);

	return result;
}

void
KGpgGenerateKey::finish()
{
	switch (getSuccess()) {
	case TS_BAD_PASSPHRASE:
		Q_EMIT statusMessage(i18n("Bad passphrase. Cannot generate a new key pair."));
		break;
	case TS_USER_ABORTED:
		Q_EMIT statusMessage(i18n("Aborted by the user. Cannot generate a new key pair."));
		break;
	case TS_INVALID_EMAIL:
		Q_EMIT statusMessage(i18n("The email address is not valid. Cannot generate a new key pair."));
		break;
	case TS_INVALID_NAME:
		Q_EMIT statusMessage(i18n("The name is not accepted by gpg. Cannot generate a new key pair."));
		break;
	case TS_OK:
		Q_EMIT statusMessage(i18n("Key %1 generated", getFingerprint()));
		break;
	default:
		while (getProcess()->hasLineStandardError()) {
			QByteArray b;
			getProcess()->readLineStandardError(&b);
			m_errorOutput << QString::fromUtf8(b);
		}

		Q_EMIT statusMessage(i18n("gpg process did not finish. Cannot generate a new key pair."));
	}
}

void
KGpgGenerateKey::newPassphraseEntered()
{
	QApplication::setOverrideCursor(Qt::BusyCursor);
	write("%commit");
	getProcess()->closeWriteChannel();
}

QString
KGpgGenerateKey::getName() const
{
	return m_name;
}

QString
KGpgGenerateKey::getEmail() const
{
	return m_email;
}

QString
KGpgGenerateKey::getFingerprint() const
{
	return m_fingerprint;
}

QString
KGpgGenerateKey::gpgErrorMessage() const
{
	return m_errorOutput.join(QLatin1Char('\n'));
}
