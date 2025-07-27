/*
    SPDX-FileCopyrightText: 2008, 2009 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "KGpgGroupMemberNode.h"

#include <QDateTime>

#include "KGpgGroupNode.h"
#include "KGpgKeyNode.h"

KGpgGroupMemberNode::KGpgGroupMemberNode(KGpgGroupNode *parent, const QString &k)
	: KGpgRefNode(parent, k)
{
}

KGpgGroupMemberNode::KGpgGroupMemberNode(KGpgGroupNode *parent, KGpgKeyNode *k)
	: KGpgRefNode(parent, k)
{
}

KGpgGroupMemberNode::~KGpgGroupMemberNode()
{
}

KgpgKeyTrust
KGpgGroupMemberNode::getTrust() const
{
	if (m_keynode != nullptr)
		return m_keynode->getTrust();
	return KgpgCore::TRUST_NOKEY;
}

KgpgItemType
KGpgGroupMemberNode::getType() const
{
	if (m_keynode != nullptr)
		return m_keynode->getType() | KgpgCore::ITYPE_GROUP;
	return KgpgCore::ITYPE_PUBLIC | KgpgCore::ITYPE_GROUP;
}

QString
KGpgGroupMemberNode::getSize() const
{
	if (m_keynode != nullptr)
		return m_keynode->getSize();
	return QString();
}

QDateTime
KGpgGroupMemberNode::getExpiration() const
{
	if (m_keynode != nullptr)
		return m_keynode->getExpiration();
	return QDateTime();
}

QDateTime
KGpgGroupMemberNode::getCreation() const
{
	if (m_keynode != nullptr)
		return m_keynode->getCreation();
	return QDateTime();
}

unsigned int
KGpgGroupMemberNode::getSignKeySize() const
{
	if (m_keynode != nullptr)
		return m_keynode->getSignKeySize();
	return 0;
}

unsigned int
KGpgGroupMemberNode::getEncryptionKeySize() const
{
	if (m_keynode != nullptr)
		return m_keynode->getEncryptionKeySize();
	return 0;
}

KGpgGroupNode *
KGpgGroupMemberNode::getParentKeyNode() const
{
	return m_parent->toGroupNode();
}
