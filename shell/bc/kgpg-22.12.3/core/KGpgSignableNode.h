/*
    SPDX-FileCopyrightText: 2008, 2009, 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef KGPGSIGNABLENODE_H
#define KGPGSIGNABLENODE_H

#include "KGpgExpandableNode.h"
#include "KGpgSignNode.h"

/**
 * @brief An object that may have KGpgSignNode children
 *
 * This class represents an object that may be signed, i.e. key nodes,
 * user ids, user attributes, and subkeys.
 */
class KGpgSignableNode : public KGpgExpandableNode
{
	Q_OBJECT

public:
	typedef QList<KGpgSignableNode *> List;
	typedef QList<const KGpgSignableNode *> const_List;

	explicit KGpgSignableNode(KGpgExpandableNode *parent = nullptr);
	~KGpgSignableNode() override;

	KGpgSignNode::List getSignatures(void) const;
	/**
	 * @brief count signatures
	 * @return the number of signatures to this object
	 *
	 * This does not include the number of signatures to child objects.
	 */
	virtual QString getSignCount() const;

	bool operator<(const KGpgSignableNode &other) const;
	bool operator<(const KGpgSignableNode *other) const;

	/**
	 * @brief returns the key node this node belongs to
	 * @returns this node if the node itself is a key or it's parent otherwise
	 */
	virtual KGpgKeyNode *getKeyNode(void) = 0;
	virtual const KGpgKeyNode *getKeyNode(void) const = 0;
};

#endif /* KGPGSIGNABLENODE_H */
