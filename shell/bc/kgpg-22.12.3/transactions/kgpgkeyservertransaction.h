/*
    SPDX-FileCopyrightText: 2009 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGKEYSERVERTRANSACTION_H
#define KGPGKEYSERVERTRANSACTION_H

#include <QObject>
#include <QString>

#include "kgpgtransaction.h"

class QProgressDialog;

/**
 * @brief base class for transactions involving key servers
 */
class KGpgKeyserverTransaction: public KGpgTransaction {
	Q_OBJECT

	Q_DISABLE_COPY(KGpgKeyserverTransaction)
	/**
	 * @brief forbidden
	 */
	KGpgKeyserverTransaction() = delete;
protected:
	/**
	 * @brief construct a new transaction for the given keyserver
	 * @param parent object that owns the transaction
	 * @param keyserver keyserver to work with
	 * @param withProgress show a progress window with cancel button
	 * @param proxy http proxy to use
	 *
	 * You should call this from the childrens constructor to set up
	 * everything properly.
	 *
	 * If the progress window is enabled and parent is an QWidget it is
	 * set as the parent of the progress window.
	 */
	KGpgKeyserverTransaction(QObject *parent, const QString &keyserver, const bool withProgress = false, const QString &proxy = QString());
public:
    ~KGpgKeyserverTransaction() override;

	/**
	 * @brief set the keyserver
	 * @param server keyserver to work with
	 */
	void setKeyserver(const QString &server);
	/**
	 * @brief set the http proxy
	 * @param proxy http proxy to use
	 *
	 * If the server is set to an empty value no proxy is used.
	 */
	void setProxy(const QString &proxy);
	/**
	 * @brief activate the progress window
	 * @param b new activation status
	 */
	void setProgressEnable(const bool b);

protected:
	void finish() override;
	bool preStart() override;

private Q_SLOTS:
	/**
	 * @brief abort the current operation
	 */
	void slotAbort();

private:
	QString m_keyserver;
	int m_keyserverpos;
	QString m_proxy;
	int m_proxypos;
	QProgressDialog *m_progress;
	bool m_showprogress;
};

#endif // KGPGUIDTRANSACTION_H
