/*
    SPDX-FileCopyrightText: 2009 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGKEYSERVERGETTRANSACTION_H
#define KGPGKEYSERVERGETTRANSACTION_H

#include <QObject>
#include <QString>
#include <QStringList>

#include "kgpgkeyservertransaction.h"

/**
 * @brief base class for transactions downloading from key servers
 */
class KGpgKeyserverGetTransaction: public KGpgKeyserverTransaction {
	Q_OBJECT

	Q_DISABLE_COPY(KGpgKeyserverGetTransaction)
	/**
	 * @brief forbidden
	 */
	KGpgKeyserverGetTransaction() = delete;

public:
	/**
	 * @brief construct a new transaction for the given keyserver
	 * @param parent object that owns the transaction
	 * @param keyserver keyserver to work with
	 * @param keys the key ids to get
	 * @param withProgress show a progress window with cancel button
	 * @param proxy http proxy to use
	 *
	 * You should call this from the childrens constructor to set up
	 * everything properly
	 */
	KGpgKeyserverGetTransaction(QObject *parent, const QString &keyserver, const QStringList &keys, const bool withProgress = false, const QString &proxy = QString());
    ~KGpgKeyserverGetTransaction() override;

	void setKeyIds(const QStringList &keys);

	const QStringList &getLog() const;

protected:
	virtual QString getGpgCommand() const = 0;
	bool preStart() override;
	bool nextLine(const QString &line) override;

private:
	int m_cmdpos;
	QStringList m_keys;
	QStringList m_log;
};

/**
 * @brief class for downloading new keys from keyserver
 */
class KGpgReceiveKeys: public KGpgKeyserverGetTransaction {
	Q_OBJECT

public:
	/**
	* @brief construct a new transaction for the given keyserver
	* @param parent object that owns the transaction
	* @param keyserver keyserver to work with
	* @param keys the key ids to get
	* @param withProgress show a progress window with cancel button
	* @param proxy http proxy to use
	*/
	KGpgReceiveKeys(QObject *parent, const QString &keyserver, const QStringList &keys, const bool withProgress = false, const QString &proxy = QString());
	~KGpgReceiveKeys() override;

protected:
	QString getGpgCommand() const override;
};

/**
 * @brief class for refreshing keys already in the keyring from keyserver
 */
class KGpgRefreshKeys: public KGpgKeyserverGetTransaction {
	Q_OBJECT

public:
	/**
	* @brief construct a new transaction for the given keyserver
	* @param parent object that owns the transaction
	* @param keyserver keyserver to work with
	* @param keys the key ids to get
	* @param withProgress show a progress window with cancel button
	* @param proxy http proxy to use
	*/
	KGpgRefreshKeys(QObject *parent, const QString &keyserver, const QStringList &keys, const bool withProgress = false, const QString &proxy = QString());
	~KGpgRefreshKeys() override;

protected:
	QString getGpgCommand() const override;
};

#endif // KGPGUIDTRANSACTION_H
