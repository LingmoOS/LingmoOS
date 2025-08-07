/*
    SPDX-FileCopyrightText: 2008, 2009, 2018 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGDELKEY_H
#define KGPGDELKEY_H

#include "kgpgtransaction.h"

#include <core/KGpgKeyNode.h>

#include <QObject>

/**
 * @brief delete a public key
 */
class KGpgDelKey: public KGpgTransaction {
	Q_OBJECT

	Q_DISABLE_COPY(KGpgDelKey)
	KGpgDelKey() = delete;
public:
	inline KGpgDelKey(QObject *parent, KGpgKeyNode *key)
		: KGpgDelKey(parent, KGpgKeyNode::List({key})) {}
	KGpgDelKey(QObject *parent, const KGpgKeyNode::List &keys);
    ~KGpgDelKey() override;

	/**
	 * @brief the keys that were requested to be removed
	 */
	const KGpgKeyNode::List keys;

	/**
	 * @brief the fingerprints of everything in keys
	 */
	const QStringList fingerprints;

protected:
	bool nextLine(const QString &line) override;
	ts_boolanswer boolQuestion(const QString &line) override;
	bool preStart() override;
	
private:
	int m_argscount;

	void setCmdLine();
};

#endif // KGPGDELKEY_H
