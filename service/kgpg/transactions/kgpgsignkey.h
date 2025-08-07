/*
    SPDX-FileCopyrightText: 2009 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGSIGNKEY_H
#define KGPGSIGNKEY_H

#include <QObject>

#include "kgpgeditkeytransaction.h"
#include "kgpgsigntransactionhelper.h"

class KGpgKeyNode;

/**
 * @brief transaction class to sign all user ids of a key
 */
class KGpgSignKey: public KGpgEditKeyTransaction, public KGpgSignTransactionHelper {
	Q_OBJECT

	Q_DISABLE_COPY(KGpgSignKey)
	KGpgSignKey() = delete;

public:
	/**
	 * @brief constructor
	 * @param parent parent object
	 * @param signer id of the key to sign with
	 * @param key node to sign
	 * @param local if signature should be local (not exportable)
	 * @param checking how carefully the identity of the key owner was checked
	 *
	 * See setUid() for description of uid.
	 */
	KGpgSignKey(QObject *parent, const QString &signer, KGpgKeyNode *key, const bool local, const carefulCheck checking);
	/**
	 * @brief destructor
	 */
    ~KGpgSignKey() override;

protected:
	bool nextLine(const QString &line) override;
	ts_boolanswer boolQuestion(const QString &line) override;
	bool passphraseReceived() override;

	KGpgTransaction *asTransaction() override;
	void replaceCmd(const QString &cmd) override;
};

#endif // KGPGSIGNKEY_H
