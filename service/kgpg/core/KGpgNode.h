/*
    SPDX-FileCopyrightText: 2008, 2009 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KGPGNODE_H
#define KGPGNODE_H

#include "kgpgkey.h"

class KGpgExpandableNode;
class KGpgKeyNode;
class KGpgRootNode;
class KGpgUidNode;
class KGpgSignableNode;
class KGpgSubkeyNode;
class KGpgUatNode;
class KGpgGroupNode;
class KGpgRefNode;
class KGpgGroupMemberNode;
class KGpgSignNode;
class KGpgOrphanNode;
class KGpgItemModel;

/**
 * @brief The abstract base class for all classes representing keyring data
 */
class KGpgNode : public QObject
{
	Q_OBJECT

	friend class KGpgItemModel;

	KGpgNode() = delete;
protected:
	KGpgExpandableNode * const m_parent;
	KGpgItemModel *m_model;

	/**
	 * constructor
	 * @param parent the parent node in item hierarchy
	 */
	explicit KGpgNode(KGpgExpandableNode *parent);

public:
	typedef QList<KGpgNode *> List;

	/**
	 * destructor
	 */
	~KGpgNode() override;

	/**
	 * Returns if this node has child nodes
	 *
	 * This may be reimplemented by child classes so they can indicate that
	 * there are child nodes before actually loading them.
	 */
	virtual bool hasChildren() const;
	/**
	 * Return how many child nodes exist
	 *
	 * When the child nodes do not already exist this will create them.
	 * This is the reason why this method is not const.
	 */
	virtual int getChildCount();
	/**
	 * Returns the child node at the given index
	 * @param index child index
	 *
	 * index may be in range 0 to getChildCount() - 1.
	 */
	virtual KGpgNode *getChild(const int index) const;
	/**
	 * Returns the index for a given child node
	 * @return -1 if the given node is not a child of this object
	 */
	virtual int getChildIndex(KGpgNode *node) const;

	/**
	 * Returns the item type of this object
	 *
	 * Since every subclass returns a distinct value you can use the
	 * result of this function to decide which cast to take. Note that
	 * there are subclasses (KGpgKeyNode, KGpgGroupMemberNode) that
	 * can return two different values.
	 */
	virtual KgpgCore::KgpgItemType getType() const = 0;
	virtual KgpgCore::KgpgKeyTrust getTrust() const;
	/**
	 * Returns a string describing the size of this object
	 *
	 * Subclasses may return a value that makes sense for whatever
	 * object they represent.
	 *
	 * The default implementation returns an empty string.
	 */
	virtual QString getSize() const;
	virtual QString getName() const;
	virtual QString getEmail() const;
	virtual QDateTime getExpiration() const;
	virtual QDateTime getCreation() const;
	virtual QString getId() const;
	virtual QString getComment() const;
	virtual QString getNameComment() const;
	/**
	 * Returns the parent node in the key hierarchy
	 *
	 * For all "primary" items like keys and key groups this will
	 * return the (invisible) root node. Calling this function for
	 * the root node will return %nullptr. No other node but the root
	 * node has a %nullptr parent.
	 */
	KGpgExpandableNode *getParentKeyNode() const;

	KGpgExpandableNode *toExpandableNode();
	const KGpgExpandableNode *toExpandableNode() const;
	KGpgSignableNode *toSignableNode();
	const KGpgSignableNode *toSignableNode() const;
	KGpgKeyNode *toKeyNode();
	const KGpgKeyNode *toKeyNode() const;
	KGpgRootNode *toRootNode();
	const KGpgRootNode *toRootNode() const;
	KGpgUidNode *toUidNode();
	const KGpgUidNode *toUidNode() const;
	KGpgSubkeyNode *toSubkeyNode();
	const KGpgSubkeyNode *toSubkeyNode() const;
	KGpgUatNode *toUatNode();
	const KGpgUatNode *toUatNode() const;
	KGpgGroupNode *toGroupNode();
	const KGpgGroupNode *toGroupNode() const;
	KGpgRefNode *toRefNode();
	const KGpgRefNode *toRefNode() const;
	KGpgGroupMemberNode *toGroupMemberNode();
	const KGpgGroupMemberNode *toGroupMemberNode() const;
	KGpgSignNode *toSignNode();
	const KGpgSignNode *toSignNode() const;
	KGpgOrphanNode *toOrphanNode();
	const KGpgOrphanNode *toOrphanNode() const;
};

#endif /* KGPGNODE_H */
