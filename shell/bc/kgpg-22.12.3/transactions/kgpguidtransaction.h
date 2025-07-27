/*
    SPDX-FileCopyrightText: 2008, 2009 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGUIDTRANSACTION_H
#define KGPGUIDTRANSACTION_H

#include <QObject>
#include <QString>

#include "kgpgtransaction.h"

/**
 * @brief base class for transactions involving only one user id of a key
 */
class KGpgUidTransaction: public KGpgTransaction {
	Q_OBJECT

	Q_DISABLE_COPY(KGpgUidTransaction)
public:
	/**
	 * @brief construct a new transaction for the given key and uid
	 * @param parent object that own the transaction
	 * @param keyid key to work with
	 * @param uid uid to work with
	 *
	 * You should call this from the childrens constructor to set up
	 * everything properly
	 */
	explicit KGpgUidTransaction(QObject *parent, const QString &keyid = QString(), const QString &uid = QString());
    ~KGpgUidTransaction() override;

	/**
	 * @brief set the key id of the transaction to the given value
	 * @param keyid fingerprint of the key to work with
	 */
	void setKeyId(const QString &keyid);
	/**
	 * @brief return the key id of the current transaction
	 */
	QString getKeyId(void) const;
	/**
	 * @brief set the uid number of the transaction to the given value
	 * @param uid the number of the user id to work with
	 */
	void setUid(const QString &uid);
	/**
	 * @brief set the uid number of the transaction to the given value
	 * @param uid the number of the user id to work with
	 *
	 * @overload
	 */
	void setUid(const unsigned int uid);

protected:
	bool preStart() override;

	/**
	 * @brief handle common GnuPG messages for uid transactions
	 * @param line GnuPG message
	 * @return true if "quit" should be sent to process
	 *
	 * You should call these function for all messages in nextLine()
	 * you do not need to handle yourself.
	 */
	bool standardCommands(const QString &line);

private:
	QString m_uid;
	int m_uidpos;
	QString m_keyid;
	int m_keyidpos;
};

#endif // KGPGUIDTRANSACTION_H
