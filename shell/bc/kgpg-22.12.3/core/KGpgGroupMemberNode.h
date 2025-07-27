/*
    SPDX-FileCopyrightText: 2008, 2009 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KGPGGROUPMEMBERNODE_H
#define KGPGGROUPMEMBERNODE_H

#include "KGpgRefNode.h"

#include "kgpgkey.h"

using namespace KgpgCore;

class KGpgKeyNode;
class KGpgGroupNode;

/**
 * @brief A member of a GnuPG group
 */
class KGpgGroupMemberNode : public KGpgRefNode
{
public:
	explicit KGpgGroupMemberNode(KGpgGroupNode *parent, const QString &k);
	explicit KGpgGroupMemberNode(KGpgGroupNode *parent, KGpgKeyNode *k);
    ~KGpgGroupMemberNode() override;

	KgpgCore::KgpgKeyTrust getTrust() const override;
	KgpgCore::KgpgItemType getType() const override;
	QString getSize() const override;
	QDateTime getExpiration() const override;
	QDateTime getCreation() const override;
	virtual KGpgGroupNode *getParentKeyNode() const;

	/**
	 * Returns the size of the signing key.
	 * @return signing key size in bits
	 */
	virtual unsigned int getSignKeySize() const;
	/**
	 * Returns the size of the first encryption subkey.
	 * @return encryption key size in bits
	 */
	virtual unsigned int getEncryptionKeySize() const;
};

#endif /* KGPGGROUPMEMBERNODE_H */
