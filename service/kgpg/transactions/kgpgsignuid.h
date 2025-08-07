/*
    SPDX-FileCopyrightText: 2009 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGSIGNUID_H
#define KGPGSIGNUID_H

#include <QObject>

#include "kgpguidtransaction.h"
#include "kgpgsigntransactionhelper.h"

class KGpgSignableNode;
class QString;

/**
 * @brief transaction class to sign a single user id of a key
 */
class KGpgSignUid: public KGpgUidTransaction, public KGpgSignTransactionHelper {
	Q_OBJECT

	Q_DISABLE_COPY(KGpgSignUid)
	KGpgSignUid() = delete;

public:
	/**
	 * @brief constructor
	 * @param parent parent object
	 * @param signer id of the key to sign with
	 * @param uid node to sign
	 * @param local if signature should be local (not exportable)
	 * @param checking how carefully the identity of the key owner was checked
	 *
	 * See setUid() for description of uid.
	 */
	KGpgSignUid(QObject *parent, const QString &signer, const KGpgSignableNode *uid, const bool local, const carefulCheck checking);
	/**
	 * @brief destructor
	 */
    ~KGpgSignUid() override;

	/**
	 * @brief set node to sign
	 * @param uid node to sign
	 *
	 * If uid is a KGpgKeyNode only the primary id of that key is
	 * signed. If you want to sign all user ids at once use
	 * KGpgSignKey instead. Legal values for uid are also KGpgUidNode
	 * and KGpgUatNode.
	 */
	void setUid(const KGpgSignableNode *uid);

protected:
	bool preStart() override;
	bool nextLine(const QString &line) override;
	ts_boolanswer boolQuestion(const QString &line) override;
	bool passphraseReceived() override;

	KGpgTransaction *asTransaction() override;
	void replaceCmd(const QString &cmd) override;

private:
	int m_cmdPos;		///< position of the command in GnuPG command line
};

#endif // KGPGSIGNUID_H
