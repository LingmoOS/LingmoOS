/*
    SPDX-FileCopyrightText: 2006, 2007 Jimmy Gilles <jimmygilles@gmail.com>
    SPDX-FileCopyrightText: 2007, 2008, 2009, 2010, 2012, 2013, 2014, 2017 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpgkey.h"

#include "convert.h"


namespace KgpgCore
{

static QString _describe_key_strength(KgpgKeyAlgo algorithm, const uint size, const QString &curve)
{
    if (!curve.isEmpty())
        return curve;

    QString prefix;

    switch(algorithm)
    {
    case ALGO_RSA: prefix = QStringLiteral("rsa"); break;
    case ALGO_ELGAMAL: prefix = QStringLiteral("elg"); break;
    case ALGO_DSA: prefix = QStringLiteral("dsa"); break;
    }

    return prefix + QString::number(size);
}

//BEGIN KeySub
KgpgKeySubPrivate::KgpgKeySubPrivate(const QString &id, const uint size, const KgpgKeyTrust trust, const KgpgKeyAlgo algo,
                                     const KgpgSubKeyType type, const QDateTime &date, const QString &curve)
    : gpgsubvalid(false),
    gpgsubid(id),
    gpgsubsize(size),
    gpgsubcreation(date),
    gpgsubtrust(trust),
    gpgsubalgo(algo),
    gpgsubtype(type),
    gpgcurve(curve)
{
}

bool KgpgKeySubPrivate::operator==(const KgpgKeySubPrivate &other) const
{
    if (gpgsubvalid != other.gpgsubvalid) return false;
    if (gpgsubalgo != other.gpgsubalgo) return false;
    if (gpgsubid != other.gpgsubid) return false;
    if (gpgsubsize != other.gpgsubsize) return false;
    if (gpgsubexpiration != other.gpgsubexpiration) return false;
    if (gpgsubcreation != other.gpgsubcreation) return false;
    if (gpgsubtrust != other.gpgsubtrust) return false;
    if (gpgsubtype != other.gpgsubtype) return false;
    return true;
}

KgpgKeySub::KgpgKeySub(const QString &id, const uint size, const KgpgKeyTrust trust, const KgpgKeyAlgo algo, const KgpgSubKeyType type,
                       const QDateTime &date, const QString &curve)
    : d(new KgpgKeySubPrivate(id, size, trust, algo, type, date, curve))
{
}

KgpgKeySub::KgpgKeySub(const KgpgKeySub &other)
	: d(other.d)
{
}

void KgpgKeySub::setExpiration(const QDateTime &date)
{
    d->gpgsubexpiration = date;
}

void KgpgKeySub::setValid(const bool valid)
{
    d->gpgsubvalid = valid;
}

QString KgpgKeySub::id() const
{
    return d->gpgsubid;
}

uint KgpgKeySub::size() const
{
    return d->gpgsubsize;
}

QString KgpgKeySub::strength() const
{
    return _describe_key_strength(algorithm(), size(), d->gpgcurve);
}

bool KgpgKeySub::unlimited() const
{
    return d->gpgsubexpiration.isNull();
}

QDateTime KgpgKeySub::expirationDate() const
{
    return d->gpgsubexpiration;
}

QDateTime KgpgKeySub::creationDate() const
{
    return d->gpgsubcreation;
}

KgpgKeyTrust KgpgKeySub::trust() const
{
    return d->gpgsubtrust;
}

KgpgKeyAlgo KgpgKeySub::algorithm() const
{
    return d->gpgsubalgo;
}

bool KgpgKeySub::valid() const
{
    return d->gpgsubvalid;
}

KgpgSubKeyType KgpgKeySub::type() const
{
    return d->gpgsubtype;
}

QString KgpgKeySub::curve() const
{
    return d->gpgcurve;
}

bool KgpgKeySub::operator==(const KgpgKeySub &other) const
{
    if (d == other.d) return true;
    if ((*d) == (*(other.d))) return true;
    return false;
}

KgpgKeySub& KgpgKeySub::operator=(const KgpgKeySub &other)
{
    d = other.d;
    return *this;
}

//END KeySub


//BEGIN Key

KgpgKeyPrivate::KgpgKeyPrivate(const QString &id, const uint size, const KgpgKeyTrust trust, const KgpgKeyAlgo algo, const KgpgSubKeyType subtype,
                               const KgpgSubKeyType keytype, const QDateTime &creationDate, const QString &curve)
    : gpgkeysecret(false),
    gpgkeyvalid(false),
    gpgkeyid(id),
    gpgkeysize(size),
    gpgkeytrust(trust),
    gpgkeycreation(creationDate),
    gpgkeyalgo(algo),
    gpgsubtype(subtype),
    gpgkeytype(keytype),
    gpgcurve(curve),
    gpgsublist(new KgpgKeySubList())
{
}

bool KgpgKeyPrivate::operator==(const KgpgKeyPrivate &other) const
{
    if (gpgkeysecret != other.gpgkeysecret) return false;
    if (gpgkeyvalid != other.gpgkeyvalid) return false;
    if (gpgkeymail != other.gpgkeymail) return false;
    if (gpgkeyname != other.gpgkeyname) return false;
    if (gpgkeycomment != other.gpgkeycomment) return false;
    if (gpgkeyfingerprint != other.gpgkeyfingerprint) return false;
    if (gpgkeyid != other.gpgkeyid) return false;
    if (gpgkeysize != other.gpgkeysize) return false;
    if (gpgkeyownertrust != other.gpgkeyownertrust) return false;
    if (gpgkeytrust != other.gpgkeytrust) return false;
    if (gpgkeycreation != other.gpgkeycreation) return false;
    if (gpgkeyexpiration != other.gpgkeyexpiration) return false;
    if (gpgkeyalgo != other.gpgkeyalgo) return false;
    if (gpgsubtype != other.gpgsubtype) return false;
    if (gpgkeytype != other.gpgkeytype) return false;
    if (gpgsublist != other.gpgsublist) return false;
    return true;
}

KgpgKey::KgpgKey(const QString &id, const uint size, const KgpgKeyTrust trust, const KgpgKeyAlgo algo, const KgpgSubKeyType subtype,
                 const KgpgSubKeyType keytype, const QDateTime &creationDate, const QString &curve)
    : d(new KgpgKeyPrivate(id, size, trust, algo, subtype, keytype, creationDate, curve))
{
}

KgpgKey::KgpgKey(const KgpgKey &other)
	: d(other.d)
{
}

void KgpgKey::setSecret(const bool secret)
{
    d->gpgkeysecret = secret;
}

void KgpgKey::setValid(const bool valid)
{
    d->gpgkeyvalid = valid;
}

void KgpgKey::setName(const QString &name)
{
    d->gpgkeyname = name;
}

void KgpgKey::setEmail(const QString &email)
{
    d->gpgkeymail = email;
}

void KgpgKey::setComment(const QString &comment)
{
    d->gpgkeycomment = comment;
}

void KgpgKey::setFingerprint(const QString &fingerprint)
{
    d->gpgkeyfingerprint = fingerprint;
}

void KgpgKey::setOwnerTrust(const gpgme_validity_t owtrust)
{
    d->gpgkeyownertrust = owtrust;
}

void KgpgKey::setExpiration(const QDateTime &date)
{
    d->gpgkeyexpiration = date;
}

bool KgpgKey::secret() const
{
    return d->gpgkeysecret;
}

bool KgpgKey::valid() const
{
    return d->gpgkeyvalid;
}

QString KgpgKey::id() const
{
    return d->gpgkeyid.right(8);
}

QString KgpgKey::fullId() const
{
    return d->gpgkeyid;
}

QString KgpgKey::name() const
{
    return d->gpgkeyname;
}

QString KgpgKey::email() const
{
    return d->gpgkeymail;
}

QString KgpgKey::comment() const
{
    return d->gpgkeycomment;
}

const QString &KgpgKey::fingerprint() const
{
    return d->gpgkeyfingerprint;
}

uint KgpgKey::size() const
{
    return d->gpgkeysize;
}

QString KgpgKey::strength() const
{
    return _describe_key_strength(algorithm(), size(), d->gpgcurve);
}

/**
 * @brief find the "best" encryption key
 *
 * The "best" is the first one that is not expired, or simply the
 * first one.
 */
static const KgpgKeySub *
bestEncryptionKey(const KgpgKeySubList &list)
{
	const KgpgKeySub *enc = nullptr;
	// Get the first encryption subkey
	for (const KgpgKeySub &k : list) {
		if (k.type() & SKT_ENCRYPTION) {
			// if the first encryption subkey is expired
			// check if there is one that is not
			if (k.trust() > TRUST_EXPIRED)
				return &k;
			if (enc == nullptr)
				enc = &k;
		}
	}

	return enc;
}

uint KgpgKey::encryptionSize() const
{
	const KgpgKeySub *enc = bestEncryptionKey(*d->gpgsublist);
	if (enc != nullptr)
		return enc->size();
	return 0;
}

QString KgpgKey::encryptionStrength() const
{
	const KgpgKeySub *enc = bestEncryptionKey(*d->gpgsublist);
    if (enc != nullptr)
        return _describe_key_strength(enc->algorithm(), enc->size(), enc->curve());
    return QString();
}

gpgme_validity_t KgpgKey::ownerTrust() const
{
    return d->gpgkeyownertrust;
}

KgpgKeyTrust KgpgKey::trust() const
{
    return d->gpgkeytrust;
}

QDateTime KgpgKey::creationDate() const
{
    return d->gpgkeycreation;
}

QDateTime KgpgKey::expirationDate() const
{
    return d->gpgkeyexpiration;
}

bool KgpgKey::unlimited() const
{
    return d->gpgkeyexpiration.isNull();
}

KgpgKeyAlgo KgpgKey::algorithm() const
{
    return d->gpgkeyalgo;
}

KgpgKeyAlgo KgpgKey::encryptionAlgorithm() const
{
	// Get the first encryption subkey
	auto it = std::find_if(d->gpgsublist->cbegin(), d->gpgsublist->cend(),
			[](const KgpgKeySub &k) { return k.type() & SKT_ENCRYPTION; });
	if (it != d->gpgsublist->cend())
		return it->algorithm();

	return ALGO_UNKNOWN;
}

KgpgSubKeyType KgpgKey::subtype() const
{
    return d->gpgsubtype;
}

KgpgSubKeyType KgpgKey::keytype() const
{
	return d->gpgkeytype;
}

QString KgpgKey::curve() const
{
    return d->gpgcurve;
}

KgpgKeySubListPtr KgpgKey::subList() const
{
    return d->gpgsublist;
}

bool KgpgKey::operator==(const KgpgKey &other) const
{
    if (d == other.d) return true;
    if ((*d) == (*(other.d))) return true;
    return false;
}

KgpgKey& KgpgKey::operator=(const KgpgKey &other)
{
    d = other.d;
    return *this;
}

KgpgKeyList::operator QStringList() const
{
    QStringList res;
    res.reserve(count());

    for (const KgpgKey &key : *this)
        res << key.fullId();

    return res;
}

//END Key

} // namespace KgpgCore
