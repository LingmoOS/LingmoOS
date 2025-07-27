/*
    SPDX-FileCopyrightText: 2008, 2009, 2010 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef KGPGUIDNODE_H
#define KGPGUIDNODE_H

#include "KGpgSignableNode.h"

#include "kgpgkey.h"

class KGpgKeyNode;

class KGpgUidNodePrivate;

/**
 * @brief A user id of a public key or key pair
 */
class KGpgUidNode : public KGpgSignableNode
{
private:
	KGpgUidNodePrivate * const d_ptr;
	Q_DECLARE_PRIVATE(KGpgUidNode)

protected:
	void readChildren() override;

public:
	explicit KGpgUidNode(KGpgKeyNode *parent, const unsigned int index, const QStringList &sl);
    ~KGpgUidNode() override;

	KgpgCore::KgpgItemType getType() const override;
	KgpgCore::KgpgKeyTrust getTrust() const override;
	QString getName() const override;
	QString getEmail() const override;
	QString getId() const override;
	QDateTime getCreation() const override;
	KGpgKeyNode *getKeyNode(void) override;
	const KGpgKeyNode *getKeyNode(void) const override;
	virtual KGpgKeyNode *getParentKeyNode() const;
	QString getComment() const override;
};

#endif /* KGPGUIDNODE_H */
