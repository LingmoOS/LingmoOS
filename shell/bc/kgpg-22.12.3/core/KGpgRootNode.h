/*
    SPDX-FileCopyrightText: 2008, 2009 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KGPGROOTNODE_H
#define KGPGROOTNODE_H

#include "KGpgExpandableNode.h"
#include "KGpgKeyNode.h"

#include <QStringList>

class KGpgGroupNode;
class QString;

/**
 * @brief The parent of all key data objects
 *
 * This object is invisible to the user but acts as the internal base object for
 * everything in the keyring. It is anchestor of all other KGpgNode objects and
 * the only one that will ever return nullptr when calling getParentKeyNode() on it.
 *
 * There is only one object of this type around at any time.
 */
class KGpgRootNode : public KGpgExpandableNode
{
	Q_OBJECT

	friend class KGpgGroupNode;

private:
	int m_groups;

protected:
	void readChildren() override;

public:
	explicit KGpgRootNode(KGpgItemModel *model);
    ~KGpgRootNode() override;

	KgpgCore::KgpgItemType getType() const override;

	/**
	 * Create new group nodes
	 * @param groups list of group names and keys to create
	 *
	 * The format of each entry of groups is name:keys, where keys is
	 * a list of key ids separated by semicolons. This is the format
	 * that is output by "gpg --list-config --with-colons".
	 */
	void addGroups(const QStringList &groups);
	void addKeys(const QStringList &ids = QStringList());
	void refreshKeys(KGpgKeyNode::List nodes);
	/**
	 * Find a key node with the given id
	 *
	 * This scans the list of primary keys for a key with the given id
	 * and returns the corresponding key node.
	 *
	 * The key id will be matched against the characters given in keyId.
	 * If you give only 8 or 16 byte you will still find the key if it
	 * exists. To be really sure to find the correct node you should pass
	 * the complete fingerprint whenever possible.
	 *
	 * @param keyId the key id to find, any length is permitted
	 * @return pointer to key node or %nullptr if no such key
	 */
	KGpgKeyNode *findKey(const QString &keyId);
	/**
	 * Return the child number of the key with the given id
	 *
	 * This scans the list of direct children for a key with the given
	 * key id. It returns the number in the internal list of children
	 * which is identical to the row number in the item model. Since
	 * proxy models may sort the items you should only call this function
	 * from the primary model (i.e. KGpgItemModel).
	 *
	 * The key id will be matched against the characters given in keyId.
	 * If you give only 8 or 16 byte you will still find the key if it
	 * exists. To be really sure to find the correct node you should pass
	 * the complete fingerprint whenever possible.
	 *
	 * @param keyId the key id to find, any length is permitted
	 * @return the child number or -1 if there is no such key
	 */
	int findKeyRow(const QString &keyId);

	/**
	 * Return the child number of the given key
	 * @param key the key to search for
	 *
	 * @overload
	 */
	int findKeyRow(const KGpgKeyNode *key);

	/**
	 * Return the group count
	 * @return the number of group nodes
	 */
	int groupChildren() const;

Q_SIGNALS:
	void newKeyNode(KGpgKeyNode *);
};

#endif /* KGPGROOTNODE_H */
