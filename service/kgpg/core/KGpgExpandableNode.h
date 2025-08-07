/*
    SPDX-FileCopyrightText: 2008, 2009, 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KGPGEXPANDABLENODE_H
#define KGPGEXPANDABLENODE_H

#include "KGpgNode.h"

class KGpgRefNode;

/**
 * @brief The abstract base class for all classes that may have child objects
 *
 * Every class that represents something in the keyring that may have
 * child objects inherits from this class. That does not mean that every
 * child object always has children, but every child \em may have children.
 */
class KGpgExpandableNode : public KGpgNode
{
	Q_OBJECT

	friend class KGpgRefNode;
protected:
	KGpgNode::List children;

	/**
	 * reimplemented in every base class to read in the child data
	 *
	 * This allows the child objects to delay the loading of the
	 * child objects until they are really needed to avoid time
	 * consuming operations for data never used.
	 */
	virtual void readChildren() = 0;

	explicit KGpgExpandableNode(KGpgExpandableNode *parent = nullptr);
public:
    ~KGpgExpandableNode() override;

	/**
	 * check if there are any child nodes
	 *
	 * The default implementation returns true if any child nodes were loaded.
	 * This may be reimplemented by child classes so they can indicate that
	 * there are child nodes before actually loading them.
	 *
	 * This method indicates if there are children if this node is expanded.
	 * In contrast wasExpanded() will only return true if the child nodes
	 * are actually present in memory.
	 */
	bool hasChildren() const override;
	/**
	 * check if there are any child nodes present in memory
	 *
	 * Returns true if any child nodes were loaded.
	 *
	 * This method indicates if the children of this node are already loaded
	 * into memory. In contrast hasChildren() may return true even if the child
	 * objects are not present in memory.
	 */
	virtual bool wasExpanded() const;
	int getChildCount() override;
	virtual const KGpgNode::List &getChildren() const;
	KGpgNode *getChild(const int index) const override;
	int getChildIndex(KGpgNode *node) const override;
	virtual void deleteChild(KGpgNode *child);
};

#endif /* KGPGEXPANDABLENODE_H */
