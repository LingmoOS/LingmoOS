/*
    SPDX-FileCopyrightText: 2008, 2009, 2010, 2013 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "KGpgRefNode.h"

#include <KLocalizedString>

#include "KGpgExpandableNode.h"
#include "KGpgRootNode.h"

KGpgRefNode::KGpgRefNode(KGpgExpandableNode *parent, const QString &keyid)
	: KGpgNode(parent),
	m_id(keyid)
{
	Q_ASSERT(!keyid.isEmpty());

	KGpgRootNode *root = getRootNode();
	KGpgExpandableNode *pnd = parent;

	do {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		m_selfsig = (pnd->getId().rightRef(keyid.length()).compare(keyid) == 0);
#else
		m_selfsig = (QStringView(pnd->getId()).right(keyid.length()).compare(keyid) == 0);
#endif
		if (m_selfsig)
			m_keynode = pnd->toKeyNode();
		else
			pnd = pnd->getParentKeyNode();
	} while (!m_selfsig && (pnd != root));

	// Self signatures do net need to get notified by their key: if the key is changed
	// the key node is deleted, then those refnode would be deleted anyway. This avoids
	// crashes when they would try to find the root node by iterating over their parent
	// when the parents destructor is already called (see bug 208659).
	if (!m_selfsig) {
		m_keynode = root->findKey(keyid);

		if (m_keynode != nullptr) {
			m_keynode->addRef(this);
		} else {
			m_updateConnection = connect(root, &KGpgRootNode::newKeyNode, this, &KGpgRefNode::keyUpdated);
		}
	}

	parent->children.append(this);
}

KGpgRefNode::KGpgRefNode(KGpgExpandableNode *parent, KGpgKeyNode *key)
	: KGpgNode(parent),
	m_id(key->getId()),
	m_keynode(key)
{
	Q_ASSERT(key != nullptr);
	Q_ASSERT(parent != nullptr);
	m_keynode->addRef(this);

	parent->children.append(this);
}

KGpgRefNode::~KGpgRefNode()
{
	if (m_keynode && !m_selfsig)
		m_keynode->delRef(this);
}

KGpgRootNode *
KGpgRefNode::getRootNode() const
{
	KGpgExpandableNode *root;
	KGpgExpandableNode *pt = m_parent;

	do {
		root = pt;
		pt = pt->getParentKeyNode();
	} while (pt != nullptr);

	return root->toRootNode();
}

void
KGpgRefNode::keyUpdated(KGpgKeyNode *nkey)
{
	Q_ASSERT(m_keynode == nullptr);
	Q_ASSERT(nkey != nullptr);

	if (nkey->compareId(m_id)) {
		disconnect(m_updateConnection);
		m_keynode = nkey;
		m_keynode->addRef(this);
	}
}

void
KGpgRefNode::unRef(KGpgRootNode *root)
{
	if (root != nullptr)
		m_updateConnection = connect(root, &KGpgRootNode::newKeyNode, this, &KGpgRefNode::keyUpdated);

	m_keynode = nullptr;
}

QString
KGpgRefNode::getId() const
{
	if (m_keynode != nullptr)
		return m_keynode->getFingerprint();
	else
		return m_id;
}

QString
KGpgRefNode::getName() const
{
	if (m_keynode != nullptr)
		return m_keynode->getName();
	return i18n("[No user id found]");
}

QString
KGpgRefNode::getEmail() const
{
	if (m_keynode != nullptr)
		return m_keynode->getEmail();
	return QString();
}

bool
KGpgRefNode::isUnknown() const
{
	return (m_keynode == nullptr);
}

KGpgKeyNode *
KGpgRefNode::getRefNode() const
{
	return m_keynode;
}
