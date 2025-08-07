/*
    SPDX-FileCopyrightText: 2006, 2007 Jimmy Gilles <jimmygilles@gmail.com>
    SPDX-FileCopyrightText: 2007, 2008, 2009, 2010, 2012, 2013, 2014 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGKEY_H
#define KGPGKEY_H

#include <gpgme.h>
#include <QSharedDataPointer>
#include <QSharedData>
#include <QPointer>
#include <QObject>
#include <QList>
#include <QDateTime>
#include <QStringList>

namespace KgpgCore
{

//BEGIN Enums

enum KgpgKeyAlgoFlag
{
    ALGO_UNKNOWN = 0,
    ALGO_RSA = 1,
    ALGO_DSA = 2,
    ALGO_ELGAMAL = 4,
    ALGO_DSA_ELGAMAL = ALGO_DSA | ALGO_ELGAMAL,
    ALGO_ECC = 8,
    ALGO_ECDSA = 16,
    ALGO_ECDH = 32,
    ALGO_EDDSA = 64,
    ALGO_RSA_RSA = 0x10001
};
Q_DECLARE_FLAGS(KgpgKeyAlgo, KgpgKeyAlgoFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(KgpgKeyAlgo)

/*! \brief trust levels of keys, uids and uats
 *
 * These values represent the trust that you have in a public key or obe if it's
 * user ids or attributes (i.e. photo ids). They are more or less ordered by
 * the level of trust. Every value but the first and the last matches one trust
 * value that is 
 */
enum KgpgKeyTrustFlag
{
    TRUST_MINIMUM = 0,		//!< internal value for use in filters
    TRUST_INVALID = 1,		//!< key is invalid
    TRUST_DISABLED = 2,		//!< key is disabled by user (not owner)
    TRUST_REVOKED = 3,		//!< key is revoked by owner
    TRUST_EXPIRED = 4,		//!< key is beyond it's expiry date
    TRUST_UNDEFINED = 5,	//!< trust value undefined (i.e. you did not set a trust level)
    TRUST_UNKNOWN = 6,		//!< trust value unknown (i.e. no entry in gpg's trust database)
    TRUST_NONE = 7,		//!< there is no trusted path to this key
    TRUST_MARGINAL = 8,		//!< there is a minimal level of trust
    TRUST_FULL = 9,		//!< you can fully trust this key
    TRUST_ULTIMATE = 10,	//!< this key has highest possible level of trust (e.g. your own secret keys)
    TRUST_NOKEY = 11		//!< internal value, e.g. for key groups
};
Q_DECLARE_FLAGS(KgpgKeyTrust, KgpgKeyTrustFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(KgpgKeyTrust)

enum KgpgSubKeyTypeFlag
{
    SKT_ENCRYPTION = 0x1,
    SKT_SIGNATURE = 0x2,
    SKT_AUTHENTICATION = 0x4,
    SKT_CERTIFICATION = 0x8
};
Q_DECLARE_FLAGS(KgpgSubKeyType, KgpgSubKeyTypeFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(KgpgSubKeyType)

/*! \brief types of items in the item models
 *
 * Every item in the item models is of one of the following types. Some of the
 * items can have properties of more than one basic type, e.g. a key pair can
 * act both as a secret and a public key. Because of this the value for key
 * pairs is a composite of the two "elementary" types for secret and public
 * keys. Other compositions than the ones defined here must not be used to set
 * an item type, but may of course be used as a mask for comparison.
 */
enum KgpgItemTypeFlag
{
    ITYPE_GROUP = 1,				//!< the element is a GnuPG key group
    ITYPE_SECRET = 2,				//!< secret key
    ITYPE_PUBLIC = 4,				//!< public key
    ITYPE_PAIR = ITYPE_SECRET | ITYPE_PUBLIC,	//!< key pair
    ITYPE_GSECRET = ITYPE_GROUP | ITYPE_SECRET,	//!< secret key as member of a key group
    ITYPE_GPUBLIC = ITYPE_GROUP | ITYPE_PUBLIC,	//!< public key as member of a key group
    ITYPE_GPAIR = ITYPE_GROUP | ITYPE_PAIR,	//!< key pair as member of a key group
    ITYPE_SUB = 8,				//!< subkey of a public or secret key
    ITYPE_UID = 16,				//!< additional user id
    ITYPE_UAT = 32,				//!< user attribute to a key (i.e. photo id)
    ITYPE_REVSIGN = 64,				//!< revokation signature
    ITYPE_SIGN = 128				//!< signature (to a key, uid or uat)
};
Q_DECLARE_FLAGS(KgpgItemType, KgpgItemTypeFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(KgpgItemType)

//END Enums

//BEGIN KeySub

class KgpgKeySubPrivate : public QSharedData
{
    KgpgKeySubPrivate();
public:
    KgpgKeySubPrivate(const QString &id, const uint size, const KgpgKeyTrust trust, const KgpgKeyAlgo algo, const KgpgSubKeyType type,
                      const QDateTime &date, const QString &curve);

    bool            gpgsubvalid;
    const QString   gpgsubid;
    const uint      gpgsubsize;
    QDateTime       gpgsubexpiration;
    const QDateTime gpgsubcreation;
    const KgpgKeyTrust gpgsubtrust;
    const KgpgKeyAlgo gpgsubalgo;
    const KgpgSubKeyType gpgsubtype;
    const QString   gpgcurve;

    bool operator==(const KgpgKeySubPrivate &other) const;
    inline bool operator!=(const KgpgKeySubPrivate &other) const
    { return !operator==(other); }
};

class KgpgKeySub
{
    KgpgKeySub();
public:
    KgpgKeySub(const QString &id, const uint size, const KgpgKeyTrust trust, const KgpgKeyAlgo algo, const KgpgSubKeyType type,
               const QDateTime &date, const QString &curve);
    KgpgKeySub(const KgpgKeySub &other);

    void setExpiration(const QDateTime &date);
    void setValid(const bool valid); // FIXME : is it possible to have a subkey that is not valid (disabled)? Please give an example. Thx. If not, this method should be removed.

    QString id() const;
    uint size() const;
    QString strength() const;
    bool unlimited() const;
    QDateTime expirationDate() const;
    QDateTime creationDate() const;
    KgpgKeyTrust trust() const;
    KgpgKeyAlgo algorithm() const;
    bool valid() const;
    KgpgSubKeyType type() const;
    QString curve() const;

    bool operator==(const KgpgKeySub &other) const;
    inline bool operator!=(const KgpgKeySub &other) const
    { return !operator==(other); }
    KgpgKeySub& operator=(const KgpgKeySub &other);

private:
    QSharedDataPointer<KgpgKeySubPrivate> d;
};

class KgpgKeySubList : public QList<KgpgKeySub>, public QObject
{
public:
    inline KgpgKeySubList() { }
    inline explicit KgpgKeySubList(const KgpgKeySub &sub) { append(sub); }
    inline KgpgKeySubList(const KgpgKeySubList &other) : QList<KgpgKeySub>(other), QObject() { }
    inline KgpgKeySubList(const QList<KgpgKeySub> &other) : QList<KgpgKeySub>(other), QObject() { }

    inline KgpgKeySubList operator+(const KgpgKeySubList &other) const
    {
        KgpgKeySubList n = *this;
        n += other;
        return n;
    }

    inline KgpgKeySubList &operator<<(KgpgKeySub sub)
    {
        append(sub);
        return *this;
    }

    inline KgpgKeySubList &operator<<(const KgpgKeySubList &l)
    {
        *this += l;
        return *this;
    }
};
typedef QPointer<KgpgKeySubList> KgpgKeySubListPtr;

//END KeySub


//BEGIN Key

class KgpgKeyPrivate : public QSharedData
{
    KgpgKeyPrivate();
public:
    /**
     * @brief constructor
     * @param id id of the key (i.e. fingerprint)
     * @param size length of the key in bits
     * @param trust trust to this key
     * @param algo public key algorithm
     * @param subtype key capabilities of this subkey
     * @param keytype key capabilities of this and all subkeys combined
     * @param creationDate date of key creation
     * @param curve GnuPG ECC curve name
     */
    KgpgKeyPrivate(const QString &id, const uint size, const KgpgCore::KgpgKeyTrust trust, const KgpgCore::KgpgKeyAlgo algo,
                   const KgpgCore::KgpgSubKeyType subtype, const KgpgCore::KgpgSubKeyType keytype, const QDateTime &creationDate,
                   const QString &curve);

    bool          gpgkeysecret;
    bool          gpgkeyvalid;
    QString       gpgkeymail;
    QString       gpgkeyname;
    QString       gpgkeycomment;
    QString       gpgkeyfingerprint;
    const QString gpgkeyid;
    const uint    gpgkeysize;
    gpgme_validity_t gpgkeyownertrust;
    const KgpgKeyTrust gpgkeytrust;
    const QDateTime gpgkeycreation;
    QDateTime     gpgkeyexpiration;
    const KgpgKeyAlgo gpgkeyalgo;
    const KgpgSubKeyType gpgsubtype;
    const KgpgSubKeyType gpgkeytype;
    QString       gpgcurve;

    KgpgKeySubListPtr gpgsublist;

    bool operator==(const KgpgKeyPrivate &other) const;
    inline bool operator!=(const KgpgKeyPrivate &other) const
    { return !operator==(other); }
};

class KgpgKey
{
public:
    /**
     * @brief constructor
     * @param id id of the key (i.e. fingerprint)
     * @param size length of the key in bits
     * @param trust trust to this key
     * @param algo public key algorithm
     * @param subtype key capabilities of this subkey
     * @param keytype key capabilities of this and all subkeys combined
     * @param creationDate date of key creation
     * @param curve GnuPG ECC curve name
     */
    KgpgKey(const QString &id, const uint size, const KgpgCore::KgpgKeyTrust trust, const KgpgCore::KgpgKeyAlgo algo,
            const KgpgCore::KgpgSubKeyType subtype, const KgpgCore::KgpgSubKeyType keytype, const QDateTime& creationDate,
            const QString &curve);
    KgpgKey(const KgpgKey &other);

    void setSecret(const bool secret);
    void setValid(const bool valid);
    void setName(const QString &name);
    void setEmail(const QString &email);
    void setComment(const QString &comment);
    void setFingerprint(const QString &fingerprint);
    void setOwnerTrust(const gpgme_validity_t owtrust);
    void setExpiration(const QDateTime &date);

    bool secret() const;
    bool valid() const;
    QString id() const;
    QString fullId() const;
    QString name() const;
    QString email() const;
    QString comment() const;
    const QString &fingerprint() const;
    uint size() const;
    QString strength() const;
    uint encryptionSize() const;
    QString encryptionStrength() const;
    gpgme_validity_t ownerTrust() const;
    KgpgKeyTrust trust() const;
    QDateTime creationDate() const;
    QDateTime expirationDate() const;
    bool unlimited() const;
    KgpgKeyAlgo algorithm() const;
    KgpgKeyAlgo encryptionAlgorithm() const;
    KgpgSubKeyType subtype() const;
    KgpgSubKeyType keytype() const;
    QString curve() const;

    KgpgKeySubListPtr subList() const;

    bool operator==(const KgpgKey &other) const;
    inline bool operator!=(const KgpgKey &other) const
    { return !operator==(other); }
    KgpgKey& operator=(const KgpgKey &other);

private:
    QSharedDataPointer<KgpgKeyPrivate> d;
};

class KgpgKeyList : public QList<KgpgKey>
{
public:
    inline KgpgKeyList() { }
    inline explicit KgpgKeyList(const KgpgKey &key) { append(key); }
    inline KgpgKeyList(const KgpgKeyList &other) : QList<KgpgKey>(other) { }
    inline KgpgKeyList(const QList<KgpgKey> &other) : QList<KgpgKey>(other) { }

    inline KgpgKeyList& operator=(const KgpgKeyList &other)
    {
        QList<KgpgKey>::operator=(static_cast<const QList<KgpgKey> >(other));
        return *this;
    }

    inline KgpgKeyList operator+(const KgpgKeyList &other) const
    {
        KgpgKeyList n = *this;
        n += other;
        return n;
    }

    inline KgpgKeyList &operator<<(KgpgKey key)
    {
        append(key);
        return *this;
    }

    inline KgpgKeyList &operator<<(const KgpgKeyList &l)
    {
        *this += l;
        return *this;
    }

    operator QStringList() const;
};

//END Key

} // namespace

#endif // KGPGKEY_H
