/*
    SPDX-FileCopyrightText: 2010, 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGDELSIGN_H
#define KGPGDELSIGN_H

#include "kgpguidtransaction.h"

#include "core/KGpgSignNode.h"

#include <QObject>
#include <QString>

/**
 * @brief delete signatures from user ids
 */
class KGpgDelSign: public KGpgUidTransaction {
	Q_OBJECT

	Q_DISABLE_COPY(KGpgDelSign)
	KGpgDelSign() = delete;
public:
	/**
	 * @brief construct a new transaction to delete signatures
	 * @param parent object that own the transaction
	 * @param signids list of signature ids to remove
	 *
	 * All members of signids need to have the same parent node, i.e.
	 * they not even may be signatures to different uids of the same key.
	 */
	KGpgDelSign(QObject *parent, const KGpgSignNode::List &signids);
	/**
	 * @brief construct a new transaction to delete signatures
	 * @param parent object that own the transaction
	 * @param signid signature to delete
	 */
	KGpgDelSign(QObject *parent, KGpgSignNode *signid);

    ~KGpgDelSign() override;

	/**
	 * @brief set the ids to delete
	 * @param keyids fingerprints of the signatures to delete
	 *
	 * This will replace all previously set signature ids.
	 */
	void setSignIds(const KGpgSignNode::List &keyids);
	/**
	 * @brief set the id to delete
	 * @param keyid fingerprint of the signatures to delete
	 * @overload
	 *
	 * This will replace all previously set signature ids.
	 */
	void setSignId(KGpgSignNode *keyid);
	/**
	 * @brief return the signature ids to delete
	 */
	KGpgSignNode::List getSignIds(void) const;

protected:
	bool nextLine(const QString &line) override;
	ts_boolanswer boolQuestion(const QString &line) override;

private:
	KGpgSignNode::List m_signids;	///< the list of ids to delete
	QString m_cachedid;	///< the next id GnuPG will ask to delete
};

#endif // KGPGDELSIGN_H
