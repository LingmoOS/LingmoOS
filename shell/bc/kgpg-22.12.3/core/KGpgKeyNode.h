/*
    SPDX-FileCopyrightText: 2008, 2009, 2010, 2012, 2013 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KGPGKEYNODE_H
#define KGPGKEYNODE_H

#include "KGpgSignableNode.h"
#include "KGpgSignNode.h"

#include "kgpgkey.h"

class KGpgRefNode;

/**
 * @brief A public key with or without corresponding secret key
 */
class KGpgKeyNode : public KGpgSignableNode
{
	Q_OBJECT

	friend class KGpgGroupMemberNode;

private:
	KgpgCore::KgpgKey *m_key;
	int m_signs;

protected:
        void readChildren() override;

	QList<KGpgRefNode *> m_refs;
	QList<KGpgRefNode *> getRefsOfType(const KgpgCore::KgpgItemType &type) const;

public:
	typedef QList<KGpgKeyNode *> List;
	typedef QList<const KGpgKeyNode *> ConstList;

	explicit KGpgKeyNode(KGpgRootNode *parent, const KgpgCore::KgpgKey &k);
        ~KGpgKeyNode() override;

        bool hasChildren() const override;

	static KgpgCore::KgpgItemType getType(const KgpgCore::KgpgKey *k);

        KgpgCore::KgpgItemType getType() const override;
        KgpgCore::KgpgKeyTrust getTrust() const override;
	const QString &getFingerprint() const;
        QString getSize() const override;
        QString getName() const override;
        QString getEmail() const override;
        QDateTime getExpiration() const override;
        QDateTime getCreation() const override;
        QString getId() const override;
        KGpgKeyNode *getKeyNode(void) override;
        const KGpgKeyNode *getKeyNode(void) const override;
	/**
	 * @brief Return if this key has a private key
	 */
	bool isSecret() const;
	/**
	 * @brief Print the full key fingerprint with spaces inserted
	 *
	 * For display purposes you normally don't want to print the full
	 * fingerprint as is because it's too many hex characters at once.
	 * This function returns the fingerprint in the format usually used
	 * for printing this out, i.e. with a space after each fourth hex
	 * character.
	 *
	 * @return the full fingerprint with spaces inserted
	 */
	QString getBeautifiedFingerprint() const;
        QString getComment() const override;
	/**
	 * @brief Return the number of signatures of the primary user id
	 *
	 * This is different from the number of children of this node as there
	 * is usually at least one subkey and there may also be additional
	 * user ids or attributes. This does not count the signatures to those
	 * slave objects, only the ones that are direct children of this node.
	 *
	 * @return the number of signatures to the primary user id
	 */
        QString getSignCount() const override;
	/**
	 * @brief Creates a copy of the KgpgKey that belongs to this class
	 */
	virtual KgpgCore::KgpgKey *copyKey() const;
	/**
	 * @brief Replaces the current key information with the new one.
	 * All sub-items (i.e. signatures, user ids ...) will be deleted. This must
	 * only be used when the id of both new and old key is the same.
	 */
	void setKey(const KgpgCore::KgpgKey &key);
	/**
	 * @brief Returns a reference to the key used in this object.
	 * This allows direct access to the values of the key e.g. for KgpgKeyInfo.
	 */
	const KgpgCore::KgpgKey *getKey() const;

	/**
	 * @brief Returns the size of the signing key.
	 * @return signing key size in bits
	 */
	virtual unsigned int getSignKeySize() const;
	/**
	 * @brief Returns the size of the first encryption subkey.
	 * @return encryption key size in bits
	 */
	virtual unsigned int getEncryptionKeySize() const;
	/**
	 * @brief Notify this key that a KGpgRefNode now references this key.
	 * @param node object that takes the reference
	 */
	void addRef(KGpgRefNode *node);
	/**
	 * @brief Remove a reference to this object
	 * @param node node that no longer has the reference
	 *
	 * Note that this must not be called as reply when this object
	 * emits updated(nullptr)
	 */
	void delRef(KGpgRefNode *node);
	/**
	 * @brief returns a list of all groups this key is member of
	 */
	QList<KGpgGroupNode *> getGroups(void) const;
	/**
	 * @brief returns a list of all group member nodes that reference this key
	 */
	QList<KGpgGroupMemberNode *> getGroupRefs(void) const;
	/**
	 * @brief returns a list of all sign nodes that reference this key
	 */
	KGpgSignNode::List getSignRefs(void) const;
	/**
	 * @brief returns a list of signatures to this key
	 * @param subkeys if signatures on subkeys should be included
	 */
	KGpgSignNode::List getSignatures(const bool subkeys) const;
	/**
	 * @brief get the user id or user attribute with the given number
	 * @param index the index of the user id to return
	 * @return the requested subitem or nullptr if that is not present
	 *
	 * User ids indexes are 1-based, so 0 is not a valid index. Passing
	 * 1 as index will return the object itself, representing the primary
	 * user id.
	 */
	const KGpgSignableNode *getUid(const unsigned int index) const;

	/**
	 * @brief compare the id of this node to the given other node
	 * @param other key id to compare to
	 * @return if ids are identical
	 *
	 * This handles different length of the id string.
	 */
	bool compareId(const QString &other) const;

	/**
	 * @brief return if this key can be used for encryption
	 */
	bool canEncrypt() const;

Q_SIGNALS:
	void expanded();

public Q_SLOTS:
	/**
	 * @brief read all subitems
	 *
	 * This will read in all subitems (e.g. subkeys, signatures). When
	 * this is done the expanded() signal is emitted. The signal is emitted
	 * immediately if the key has been expanded before.
	 *
	 * This will not update the child items in case they are already present.
	 * Use KGpgItemModel::refreshKey() instead.
	 */
	void expand();
};

#endif /* KGPGKEYNODE_H */
