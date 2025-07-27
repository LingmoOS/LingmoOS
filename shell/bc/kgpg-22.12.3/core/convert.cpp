/*
    SPDX-FileCopyrightText: 2006 Jimmy Gilles <jimmygilles@gmail.com>
    SPDX-FileCopyrightText: 2010, 2013, 2014, 2016, 2017 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "convert.h"


#include "kgpg_general_debug.h"

#include "kgpgsettings.h"

// Backport of gpgme enums from version 1.7.0
#ifndef GPGME_PK_EDDSA
#define GPGME_PK_EDDSA 303
#endif
#ifndef GPGME_PK_ECC
#define GPGME_PK_ECC 18
#endif

/* Conversion from OpenPGP algorithm number to GPGME algorithm numbers. */
static int _gpgme_map_pk_algo (int algo)
{
	switch (algo)
	{
		case 18: algo = GPGME_PK_ECDH; break;
		case 19: algo = GPGME_PK_ECDSA; break;
		case 22: algo = GPGME_PK_EDDSA; break;
		default: break;
	}
	return algo;
}

namespace KgpgCore
{

namespace Convert
{

QString toString(const KgpgKeyAlgo algorithm)
{
	switch (algorithm) {
	case ALGO_RSA:
		return i18nc("Encryption algorithm", "RSA");
	case ALGO_DSA:
		return i18nc("Encryption algorithm", "DSA");
	case ALGO_ELGAMAL:
		return i18nc("Encryption algorithm", "ElGamal");
	case ALGO_DSA_ELGAMAL:
		return i18nc("Encryption algorithm", "DSA & ElGamal");
	case ALGO_RSA_RSA:
		return i18nc("Encryption algorithm RSA, Signing algorithm RSA", "RSA & RSA");
	case ALGO_ECC:
		return i18nc("Encryption algorithm", "ECC");
	case ALGO_ECDSA:
		return i18nc("Signing algorithm", "ECDSA");
	case ALGO_ECDH:
		return i18nc("Encryption algorithm", "ECDH");
	case ALGO_EDDSA:
		return i18nc("Signing algorithm", "EdDSA");
	case ALGO_UNKNOWN:
	default:
		return i18nc("Unknown algorithm", "Unknown");
	}
}

QString toString(const gpgme_validity_t ownertrust)
{
	switch (ownertrust) {
	case GPGME_VALIDITY_UNDEFINED:
		return i18n("I do not know");
	case GPGME_VALIDITY_NEVER:
		return i18n("I do NOT trust");
	case GPGME_VALIDITY_MARGINAL:
		return i18n("Marginally");
	case GPGME_VALIDITY_FULL:
		return i18n("Fully");
	case GPGME_VALIDITY_ULTIMATE:
		return i18n("Ultimately");
	case GPGME_VALIDITY_UNKNOWN:
	default:
		return i18nc("Unknown trust in key owner", "Unknown");
	}
}

QString toString(const KgpgKeyTrust trust)
{
	switch (trust) {
	case TRUST_INVALID:
		return i18nc("Invalid key", "Invalid");
	case TRUST_DISABLED:
		return i18nc("Disabled key", "Disabled");
	case TRUST_REVOKED:
		return i18n("Revoked");
	case TRUST_EXPIRED:
		return i18nc("Expired key", "Expired");
	case TRUST_UNDEFINED:
		return i18nc("Undefined key trust", "Undefined");
	case TRUST_NONE:
		return i18nc("No trust in key", "None");
	case TRUST_MARGINAL:
		return i18nc("Marginal trust in key", "Marginal");
	case TRUST_FULL:
		return i18nc("Full trust in key", "Full");
	case TRUST_ULTIMATE:
		return i18nc("Ultimate trust in key", "Ultimate");
	case TRUST_UNKNOWN:
	default:
		return i18nc("Unknown trust in key", "Unknown");
	}
}

QString toString(const KgpgCore::KgpgSubKeyType type)
{
	QStringList res;

	if (type & SKT_SIGNATURE)
		res << i18nc("key capability", "Signature");
	if (type & SKT_ENCRYPTION)
		res << i18nc("key capability", "Encryption");
	if (type & SKT_AUTHENTICATION)
		res << i18nc("key capability", "Authentication");
	if (type & SKT_CERTIFICATION)
		res << i18nc("key capability", "Certification");

	return res.join(i18nc("used to join a list of key types, e.g. 'encryption, signature'", ", "));
}

KgpgKeyAlgo toAlgo(const uint v)
{
	uint gpgme_algo = _gpgme_map_pk_algo(v);
	switch (gpgme_algo) {
	case GPGME_PK_RSA:
		return ALGO_RSA;
	case GPGME_PK_ELG_E:
	case GPGME_PK_ELG:
		return ALGO_ELGAMAL;
	case GPGME_PK_DSA:
		return ALGO_DSA;
	case GPGME_PK_ECC:
		return ALGO_ECC;
	case GPGME_PK_ECDSA:
		return ALGO_ECDSA;
	case GPGME_PK_ECDH:
		return ALGO_ECDH;
	case GPGME_PK_EDDSA:
		return ALGO_EDDSA;
	default:
		return ALGO_UNKNOWN;
	}
}

KgpgKeyAlgo toAlgo(const QString &s)
{
    bool b;
    unsigned int u = s.toUInt(&b);
    return b ? toAlgo(u) : ALGO_UNKNOWN;
}

KgpgKeyTrust toTrust(const QChar &c)
{
	switch (c.toLatin1()) {
	case 'o':
		return TRUST_UNKNOWN;
	case 'i':
		return TRUST_INVALID;
	case 'd':
		return TRUST_DISABLED;
	case 'r':
		return TRUST_REVOKED;
	case 'e':
		return TRUST_EXPIRED;
	case 'q':
		return TRUST_UNDEFINED;
	case 'n':
		return TRUST_NONE;
	case 'm':
		return TRUST_MARGINAL;
	case 'f':
		return TRUST_FULL;
	case 'u':
		return TRUST_ULTIMATE;
	default:
		return TRUST_UNKNOWN;
	}
}

KgpgKeyTrust toTrust(const QString &s)
{
    return s.isEmpty() ? TRUST_UNKNOWN : toTrust(s[0]);
}

gpgme_validity_t toOwnerTrust(const QChar &c)
{
	switch (c.toLatin1()) {
	case 'n':
		return GPGME_VALIDITY_NEVER;
	case 'm':
		return GPGME_VALIDITY_MARGINAL;
	case 'f':
		return GPGME_VALIDITY_FULL;
	case 'u':
		return GPGME_VALIDITY_ULTIMATE;
	default:
		return GPGME_VALIDITY_UNDEFINED;
	}
}

KgpgSubKeyType toSubType(const QString& capString, bool upper)
{
	KgpgSubKeyType ret;

	for (const QChar &ch : capString) {
		switch (ch.toLatin1()) {
		case 's':
		case 'S':
			if (upper != ch.isUpper())
				continue;
			ret |= SKT_SIGNATURE;
			break;
		case 'e':
		case 'E':
			if (upper != ch.isUpper())
				continue;
			ret |= SKT_ENCRYPTION;
			break;
		case 'a':
		case 'A':
			if (upper != ch.isUpper())
				continue;
			ret |= SKT_AUTHENTICATION;
			break;
		case 'c':
		case 'C':
			if (upper != ch.isUpper())
				continue;
			ret |= SKT_CERTIFICATION;
			break;
		case 'D':	// disabled key
		case '?':	// unknown to GnuPG
			continue;
		default:
			qCDebug(KGPG_LOG_GENERAL) << "unknown capability letter" << ch
			<< "in cap string" << capString;
		}
	}

	return ret;
}

QDateTime toDateTime(const QString &s)
{
	QDateTime ret;

	if (s.isEmpty())
		return ret;

	if (s.contains(QLatin1Char('T')))
		ret = QDateTime::fromString(s, QLatin1String("yyyyMMddTHHmmss"));
	else
		ret = QDateTime::fromSecsSinceEpoch(s.toUInt());

	return ret;
}

} // namespace Convert

} // namespace KgpgCore
