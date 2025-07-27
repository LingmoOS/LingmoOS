/*
    SPDX-FileCopyrightText: 2008, 2009 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KGPGREFNODE_H
#define KGPGREFNODE_H

#include "KGpgNode.h"

class KGpgKeyNode;
class KGpgRootNode;

/**
 * @brief Class for child objects that are only a reference to a primary key
 *
 * This is the base class for all type of objects that match these criteria:
 * -they can not have child objects
 * -they are only a reference to a primary key (which needs not to be in the
 *  key ring)
 *
 * Do not create instances from this class. Use KGpgGroupMemberNode and
 * KGpgSignNode as those represent the existing objects. This class exists
 * only to get the hierarchy right.
 */
class KGpgRefNode : public KGpgNode
{
	Q_OBJECT

private:
	const QString m_id;
	bool m_selfsig;		///< if this is a reference to it's own parent
	QMetaObject::Connection m_updateConnection;

protected:
	KGpgKeyNode *m_keynode;

	explicit KGpgRefNode(KGpgExpandableNode *parent, KGpgKeyNode *key);
	explicit KGpgRefNode(KGpgExpandableNode *parent, const QString &keyid);

	KGpgRootNode *getRootNode() const;

public:
        ~KGpgRefNode() override;

        QString getId() const override;
        QString getName() const override;
        QString getEmail() const override;
	/**
	 * Get the node of the primary key this node references to
	 *
	 * This will return the key node of the primary key this node
	 * references. This may be %nullptr if the primary key is not in the key
	 * ring, e.g. if this is a signature of an unknown key.
	 *
	 * @return the node of the primary key or %nullptr
	 */
	virtual KGpgKeyNode *getRefNode() const;

	/**
	 * Check if the referenced key exists
	 *
	 * @return if getRefNode() will return %nullptr or not
	 */
	bool isUnknown() const;

	/**
	 * Break the current reference
	 * @param root root node
	 *
	 * This is called when the referenced node is going away.
	 *
	 * The root node is passed for two reasons:
	 * @li it doesn't need to be searched again for every ref node which
	 * can be many in case of an important key node get's deleted
	 * @li the ref node may be a child of the deleted node, then we can
	 * not call the parents functions to find the root anymore. This helps
	 * simplifying the code
	 */
	void unRef(KGpgRootNode *root);

private Q_SLOTS:
	void keyUpdated(KGpgKeyNode *);
};

#endif /* KGPGREFNODE_H */
