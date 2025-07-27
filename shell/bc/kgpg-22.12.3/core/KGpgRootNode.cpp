/*
    SPDX-FileCopyrightText: 2008, 2009, 2010, 2012, 2013, 2016, 2017 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "KGpgRootNode.h"

#include "KGpgGroupNode.h"
#include "kgpginterface.h"
#include "KGpgOrphanNode.h"
#include "kgpgsettings.h"

#include <QString>

KGpgRootNode::KGpgRootNode(KGpgItemModel *model)
	: KGpgExpandableNode(nullptr),
	m_groups(0)
{
	m_model = model;
}

KGpgRootNode::~KGpgRootNode()
{
	// clear the parents in all children to signal them not to do any
	// update signalling
	for (KGpgNode *child: qAsConst(children))
		child->setParent(nullptr);
}

void
KGpgRootNode::readChildren()
{
}

KgpgCore::KgpgItemType
KGpgRootNode::getType() const
{
    return {};
}

void
KGpgRootNode::addGroups(const QStringList &groups)
{
	for (const QString &group : groups) {
		const QStringList parts = group.split(QLatin1Char(':'));
		if (parts.count() < 2)
			continue;
		const QString groupName = parts.first();
		new KGpgGroupNode(this, groupName, parts.at(1).split(QLatin1Char(';'), Qt::SkipEmptyParts));
	}
}

void
KGpgRootNode::addKeys(const QStringList &ids)
{
	const KgpgCore::KgpgKeyList publiclist = KgpgInterface::readPublicKeys(ids);
	KgpgCore::KgpgKeyList secretlist = KgpgInterface::readSecretKeys();

	QStringList issec = secretlist;

	for (KgpgCore::KgpgKey key : publiclist) {
		int index = issec.indexOf(key.fullId());

		if (index >= 0) {
			key.setSecret(true);
			issec.removeAt(index);
			secretlist.removeAt(index);
		}

		KGpgKeyNode *nd = new KGpgKeyNode(this, key);
		Q_EMIT newKeyNode(nd);
	}

	for (const KgpgCore::KgpgKey &key : qAsConst(secretlist))
		new KGpgOrphanNode(this, key);
}

void
KGpgRootNode::refreshKeys(KGpgKeyNode::List nodes)
{
	QStringList ids;
	ids.reserve(nodes.count());

	for (const KGpgNode *nd : qAsConst(nodes))
		ids << nd->getId();

	const KgpgCore::KgpgKeyList publiclist = KgpgInterface::readPublicKeys(ids);
	QStringList issec = KgpgInterface::readSecretKeys(ids);

	for (KgpgCore::KgpgKey key : publiclist) {
		int index = issec.indexOf(key.fullId());
		if (index != -1) {
			key.setSecret(true);
			issec.removeAt(index);
		}

		for (int j = 0; j < nodes.count(); j++) {
			KGpgKeyNode *nd = nodes.at(j);
			if (nd->getId() == key.fullId()) {
				nodes.removeAt(j);
				nd->setKey(key);
				break;
			}
		}
	}
}

KGpgKeyNode *
KGpgRootNode::findKey(const QString &keyId)
{
	int i = findKeyRow(keyId);
	if (i >= 0) {
		return children[i]->toKeyNode();
	}

	return nullptr;
}

int
KGpgRootNode::findKeyRow(const QString &keyId)
{
	int i = 0;

	for (const KGpgNode *node : qAsConst(children)) {
		if ((node->getType() & ITYPE_PAIR) == 0) {
			++i;
			continue;
		}

		const KGpgKeyNode *key = node->toKeyNode();

		if (key->compareId(keyId))
			return i;
		++i;
	}

	return -1;
}

int
KGpgRootNode::groupChildren() const
{
	return m_groups;
}

int
KGpgRootNode::findKeyRow(const KGpgKeyNode *key)
{
	for (int i = 0; i < children.count(); i++) {
		if (children[i] == key)
			return i;
	}
	return -1;
}
