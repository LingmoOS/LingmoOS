/*
    SPDX-FileCopyrightText: 2009 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGSIGNTRANSACTIONHELPER_H
#define KGPGSIGNTRANSACTIONHELPER_H

#include "kgpgtransaction.h"

class KGpgKeyNode;
class QString;

/**
 * @brief helper class for key signing transactions
 */
class KGpgSignTransactionHelper {
	Q_DISABLE_COPY(KGpgSignTransactionHelper)
	KGpgSignTransactionHelper() = delete;
public:
	/**
	 * @brief the outcomes of nextLine()
	 */
	enum lineParseResults {
		handledFalse,	///< the line was parsed successfully and transaction can continue
		handledTrue,	///< the line was parsed successfully and the transaction shoult be shut down
		notHandled	///< the line was not handled
	};

	enum carefulCheck {
		noAnswer = 0,
		notChecked = 1,
		normalChecking = 2,
		carefulChecking = 3
	};

	enum ts_signuid {
		TS_ALREADY_SIGNED = KGpgTransaction::TS_COMMON_END + 1	///< user id is alredy signed by given key
	};

	/**
	 * @brief destructor
	 */
	virtual ~KGpgSignTransactionHelper();

protected:
	/**
	 * @brief constructor
	 * @param signer id of the key to sign with
	 * @param local if signature should be local (not exportable)
	 * @param checking how carefully the identity of the key owner was checked
	 */
	KGpgSignTransactionHelper(const QString &signer, const bool local, const carefulCheck checking);
	/**
	 * @brief handle signing commands from GnuPG
	 * @param line input to parse
	 *
	 * This will handle the GnuPG commands specific to signing.
	 */
	lineParseResults nextLine(const QString &line);
	KGpgTransaction::ts_boolanswer boolQuestion(const QString &line);

public:
	/**
	 * @brief set key node this transaction is using
	 * @param node new key node
	 */
	void setKey(const KGpgKeyNode *node);

	/**
	 * @brief get the key node this transaction is using
	 */
	const KGpgKeyNode *getKey(void) const;

	/**
	 * @brief set if the signature should be local (not exportable)
	 * @param local flag if local signature should be applied
	 */
	void setLocal(const bool local);

	/**
	 * @brief check if local signing is requested
	 */
	bool getLocal(void) const;

	/**
	 * @brief set the level how carefully the identity was checked
	 * @param level level to set
	 */
	void setChecking(const carefulCheck level);

	/**
	 * @brief check if local signing is requested
	 */
	carefulCheck getChecking(void) const;

	/**
	 * @brief set which private key is used to sign
	 * @param signer id of private key to use
	 */
	void setSigner(const QString &signer);

	/**
	 * @brief get key id which is used to sign
	 */
	QString getSigner(void) const;

	/**
	 * @brief add a secret keyring file
	 *
	 * This allows to specify an additional file where secret keys are
	 * stored to be used by this operation. This is especially useful
	 * if a different GnuPG home directory is set but the original keys
	 * should be used for signing.
	 */
	void setSecringFile(const QString &filename);

private:
	const KGpgKeyNode *m_node;
	QString m_signer;
	bool m_local;
	carefulCheck m_checking;

protected:
	int m_signerPos;	///< position of the signer argument in GnuPG command line

	/**
	 * @brief returns the transaction object to use
	 *
	 * This should really be static_cast<>(this) as you should
	 * only use this class as one of two anchestors of a transaction.
	 */
	virtual KGpgTransaction *asTransaction() = 0;
	/**
	 * @brief replaces the command passed to GnuPG
	 * @param cmd new command to use
	 */
	virtual void replaceCmd(const QString &cmd) = 0;
};

#endif // KGPGSIGNTRANSACTIONHELPER_H
