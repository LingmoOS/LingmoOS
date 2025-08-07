/*
    SPDX-FileCopyrightText: 2008, 2009, 2010, 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KGPGORPHANNODE_H
#define KGPGORPHANNODE_H

#include "KGpgNode.h"

#include "kgpgkey.h"

using namespace KgpgCore;

class KGpgExpandableNode;

/**
 * @brief A lone secret key without public key
 */
class KGpgOrphanNode : public KGpgNode
{
private:
	KgpgCore::KgpgKey *m_key;

public:
	explicit KGpgOrphanNode(KGpgExpandableNode *parent, const KgpgKey &k);
    ~KGpgOrphanNode() override;

	KgpgCore::KgpgItemType getType() const override;
	KgpgCore::KgpgKeyTrust getTrust() const override;
	const QString &getFingerprint() const;
	QString getSize() const override;
	QString getName() const override;
	QString getEmail() const override;
	QDateTime getExpiration() const override;
	QDateTime getCreation() const override;
	QString getId() const override;
};

#endif /* KGPGORPHANNODE_H */
