/*
    SPDX-FileCopyrightText: 2010 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGKEYSERVERSEARCHTRANSACTION_H
#define KGPGKEYSERVERSEARCHTRANSACTION_H

#include <QObject>
#include <QString>
#include <QStringList>

#include "kgpgkeyservertransaction.h"

/**
 * @brief base class for transactions downloading from key servers
 */
class KGpgKeyserverSearchTransaction: public KGpgKeyserverTransaction {
	Q_OBJECT

	Q_DISABLE_COPY(KGpgKeyserverSearchTransaction)
	/**
	 * @brief forbidden
	 */
	KGpgKeyserverSearchTransaction() = delete;

public:
	/**
	 * @brief construct a new search on the given keyserver
	 * @param parent object that owns the transaction
	 * @param keyserver keyserver to work with
	 * @param pattern the search pattern
	 * @param withProgress show a progress window with cancel button
	 * @param proxy http proxy to use
	 *
	 * You should call this from the childrens constructor to set up
	 * everything properly
	 */
	KGpgKeyserverSearchTransaction(QObject *parent, const QString &keyserver, const QString &pattern, const bool withProgress = false, const QString &proxy = QString());
    ~KGpgKeyserverSearchTransaction() override;

	void setPattern(const QString &pattern);

Q_SIGNALS:
	/**
	 * @brief emitted every time a new key is completed
	 * @param lines the lines that belong to that key
	 */
	void newKey(QStringList lines);

protected:
	bool preStart() override;
	bool nextLine(const QString &line) override;
	/**
	 * @brief needed to submit the last search result
	 */
	void finish() override;

private:
	QStringList m_keyLines;		///< the lines belonging to one key
	int m_patternPos;
	bool m_pageEmpty;		///< if the current page of output is empty
};

#endif // KGPGUIDTRANSACTION_H
