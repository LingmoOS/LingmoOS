/*
    SPDX-FileCopyrightText: 2008, 2009, 2010, 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "KGpgOrphanNode.h"

KGpgOrphanNode::KGpgOrphanNode(KGpgExpandableNode *parent, const KgpgKey &k)
	: KGpgNode(parent),
	m_key(new KgpgKey(k))
{
}

KGpgOrphanNode::~KGpgOrphanNode()
{
	delete m_key;
}

KgpgItemType
KGpgOrphanNode::getType() const
{
	return ITYPE_SECRET;
}

QString
KGpgOrphanNode::getName() const
{
	return m_key->name();
}

QString
KGpgOrphanNode::getEmail() const
{
	return m_key->email();
}

QString
KGpgOrphanNode::getSize() const
{
	return QString::number(m_key->size());
}

QDateTime
KGpgOrphanNode::getExpiration() const
{
	return m_key->expirationDate();
}

QDateTime
KGpgOrphanNode::getCreation() const
{
	return m_key->creationDate();
}

QString
KGpgOrphanNode::getId() const
{
	return m_key->fullId();
}

KgpgCore::KgpgKeyTrust
KGpgOrphanNode::getTrust() const
{
	return m_key->trust();
}

const QString &
KGpgOrphanNode::getFingerprint() const
{
	return m_key->fingerprint();
}
