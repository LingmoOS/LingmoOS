/*
    SPDX-FileCopyrightText: 2008, 2009 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "KGpgSubkeyNode.h"

#include <KLocalizedString>

#include "convert.h"
#include "KGpgKeyNode.h"

KGpgSubkeyNode::KGpgSubkeyNode(KGpgKeyNode *parent, const KgpgKeySub &k)
	: KGpgSignableNode(parent),
	m_skey(k)
{
	Q_ASSERT(parent != nullptr);
}
KGpgSubkeyNode::~KGpgSubkeyNode()
{
}

void
KGpgSubkeyNode::readChildren()
{
}

KgpgCore::KgpgItemType
KGpgSubkeyNode::getType() const
{
	return ITYPE_SUB;
}

KgpgCore::KgpgKeyTrust
KGpgSubkeyNode::getTrust() const
{
	return m_skey.trust();
}

QDateTime
KGpgSubkeyNode::getExpiration() const
{
	return m_skey.expirationDate();
}

QDateTime
KGpgSubkeyNode::getCreation() const
{
	return m_skey.creationDate();
}

QString
KGpgSubkeyNode::getId() const
{
	return m_skey.id();
}

KGpgKeyNode *
KGpgSubkeyNode::getKeyNode(void)
{
	return getParentKeyNode()->toKeyNode();
}

const KGpgKeyNode *
KGpgSubkeyNode::getKeyNode(void) const
{
	return getParentKeyNode()->toKeyNode();
}

QString
KGpgSubkeyNode::getName() const
{
	return i18n("%1 subkey", Convert::toString(m_skey.algorithm()));
}

QString
KGpgSubkeyNode::getSize() const
{
	return m_skey.strength();
}

KGpgKeyNode *
KGpgSubkeyNode::getParentKeyNode() const
{
	return m_parent->toKeyNode();
}

void
KGpgSubkeyNode::setFingerprint(const QString &fpr)
{
	m_fingerprint = fpr;
}

const QString &
KGpgSubkeyNode::getFingerprint() const
{
	return m_fingerprint;
}
