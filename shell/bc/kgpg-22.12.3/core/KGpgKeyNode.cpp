/*
    SPDX-FileCopyrightText: 2008, 2009, 2010, 2012, 2013, 2014, 2017 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "KGpgKeyNode.h"

#include "KGpgGroupMemberNode.h"
#include "KGpgRootNode.h"
#include "KGpgSubkeyNode.h"
#include "KGpgUatNode.h"
#include "KGpgUidNode.h"
#include "convert.h"
#include "kgpginterface.h"
#include "kgpgsettings.h"
#include "model/kgpgitemmodel.h"

#include <KLocalizedString>

KGpgKeyNode::KGpgKeyNode(KGpgRootNode *parent, const KgpgCore::KgpgKey &k)
	: KGpgSignableNode(parent),
	m_key(new KgpgCore::KgpgKey(k)),
	m_signs(0)
{
}

KGpgKeyNode::~KGpgKeyNode()
{
	// do not try to access the root node if we are being deleted from there
	KGpgRootNode * const root = parent() != nullptr ? m_parent->toRootNode() : nullptr;
	for (KGpgRefNode *nd : qAsConst(m_refs))
		nd->unRef(root);
}

KgpgCore::KgpgItemType
KGpgKeyNode::getType() const
{
	return getType(m_key);
}

bool
KGpgKeyNode::hasChildren() const
{
	return true;
}

KgpgCore::KgpgItemType
KGpgKeyNode::getType(const KgpgCore::KgpgKey *k)
{
	if (k->secret())
		return KgpgCore::ITYPE_PAIR;

	return KgpgCore::ITYPE_PUBLIC;
}

KgpgCore::KgpgKeyTrust
KGpgKeyNode::getTrust() const
{
	return m_key->trust();
}

const QString &
KGpgKeyNode::getFingerprint() const
{
	return m_key->fingerprint();
}

QString
KGpgKeyNode::getSize() const
{
	return i18nc("size of signing key / size of encryption key",
			"%1 / %2", m_key->strength(),
			m_key->encryptionStrength());
}

QString
KGpgKeyNode::getName() const
{
	return m_key->name();
}

QString
KGpgKeyNode::getEmail() const
{
	return m_key->email();
}

QDateTime
KGpgKeyNode::getExpiration() const
{
	return m_key->expirationDate();
}

QDateTime
KGpgKeyNode::getCreation() const
{
	return m_key->creationDate();
}

QString
KGpgKeyNode::getId() const
{
	return m_key->fullId();
}

KGpgKeyNode *
KGpgKeyNode::getKeyNode(void)
{
	return this;
}

bool
KGpgKeyNode::isSecret() const
{
	return m_key->secret();
}

const KGpgKeyNode *
KGpgKeyNode::getKeyNode(void) const
{
	return this;
}

QString
KGpgKeyNode::getBeautifiedFingerprint() const
{
	static const QChar space = QLatin1Char(' ');
	QString fingervalue = m_key->fingerprint();
	int len = fingervalue.length();
	if ((len > 0) && (len % 4 == 0))
		for (int n = 0; 4 * (n + 1) < len; n++)
			fingervalue.insert(5 * n + 4, space);
	return fingervalue;
}

QString
KGpgKeyNode::getComment() const
{
	return m_key->comment();
}

void
KGpgKeyNode::readChildren()
{
	KgpgInterface::readSignatures(this);

	m_signs = std::count_if(children.cbegin(), children.cend(),
			[](const KGpgNode *n) { return n->getType() == ITYPE_SIGN; });
}

QString
KGpgKeyNode::getSignCount() const
{
	if (!wasExpanded())
		return QString();
	return i18np("1 signature", "%1 signatures", m_signs);
}

KgpgKey *
KGpgKeyNode::copyKey() const
{
	return new KgpgKey(*m_key);
}

void
KGpgKeyNode::setKey(const KgpgKey &key)
{
	Q_ASSERT(m_key->fingerprint() == key.fingerprint());
	delete m_key;

	for (int i = 0; i < children.count(); i++)
		delete children.at(i);
	children.clear();

	m_key = new KgpgKey(key);
}

const KgpgKey *
KGpgKeyNode::getKey() const
{
	return m_key;
}

unsigned int
KGpgKeyNode::getSignKeySize() const
{
	return m_key->size();
}

unsigned int
KGpgKeyNode::getEncryptionKeySize() const
{
	return m_key->encryptionSize();
}

void
KGpgKeyNode::addRef(KGpgRefNode *node)
{
	Q_ASSERT(m_refs.indexOf(node) == -1);
	m_refs.append(node);
}

void
KGpgKeyNode::delRef(KGpgRefNode *node)
{
	Q_ASSERT(m_refs.indexOf(node) != -1);
	m_refs.removeOne(node);
	Q_ASSERT(m_refs.indexOf(node) == -1);
}

QList<KGpgGroupNode *>
KGpgKeyNode::getGroups(void) const
{
	QList<KGpgGroupNode *> ret;
	const QList<KGpgGroupMemberNode *> refs = getGroupRefs();
	ret.reserve(refs.count());

	for (KGpgGroupMemberNode *gnd : refs)
		ret.append(gnd->getParentKeyNode());

	return ret;
}

QList<KGpgRefNode *>
KGpgKeyNode::getRefsOfType(const KgpgItemType &type) const
{
	QList<KGpgRefNode *> ret;

	for (KGpgRefNode *nd : m_refs) {
		if (nd->getType() & type)
			ret.append(nd);
	}

	return ret;
}

QList<KGpgGroupMemberNode *>
KGpgKeyNode::getGroupRefs(void) const
{
	QList<KGpgGroupMemberNode *> ret;
	const QList<KGpgRefNode *> refs = getRefsOfType(KgpgCore::ITYPE_GROUP);
	ret.reserve(refs.count());

	for (KGpgRefNode *rn : refs)
		ret.append(rn->toGroupMemberNode());

	return ret;
}

KGpgSignNode::List
KGpgKeyNode::getSignRefs(void) const
{
	KGpgSignNode::List ret;
	const QList<KGpgRefNode *> refs = getRefsOfType(KgpgCore::ITYPE_SIGN);
	ret.reserve(refs.count());

	for (KGpgRefNode *rn : refs)
		ret.append(rn->toSignNode());

	return ret;
}

KGpgSignNode::List
KGpgKeyNode::getSignatures(const bool subkeys) const
{
	KGpgSignNode::List ret = KGpgSignableNode::getSignatures();

	if (!subkeys)
		return ret;

	for (KGpgNode *child : children) {
		switch (child->getType()) {
		case KgpgCore::ITYPE_UID:
		case KgpgCore::ITYPE_UAT:
			break;
		default:
			continue;
		}

		const KGpgSignNode::List tmp = child->toSignableNode()->getSignatures();

		for (KGpgSignNode *sn : tmp) {
			bool found = false;
			const QString snid(sn->getId());

			for (const KGpgSignNode *retsn : qAsConst(ret)) {
				found = (retsn->getId() == snid);
				if (found)
					break;
			}

			if (!found)
				ret << sn;
		}
	}

	return ret;
}

const KGpgSignableNode *
KGpgKeyNode::getUid(const unsigned int index) const
{
	Q_ASSERT(index > 0);

	if (index == 1)
		return this;

	const QString idxstr = QString::number(index);

	for (const KGpgNode *child : children) {
		switch (child->getType()) {
		case KgpgCore::ITYPE_UID:
		case KgpgCore::ITYPE_UAT:
			if (child->getId() == idxstr)
				return child->toSignableNode();
			break;
		default:
			continue;
		}
	}

	return nullptr;
}

bool
KGpgKeyNode::compareId(const QString &other) const
{
	if (other.length() == m_key->fullId().length())
		return (other.compare(m_key->fullId(), Qt::CaseInsensitive) == 0);

	if (other.length() == m_key->fingerprint().length())
		return (other.compare(m_key->fingerprint(), Qt::CaseInsensitive) == 0);

	const QString comId = m_key->fullId().isEmpty() ? m_key->fingerprint() : m_key->fullId();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	return (other.rightRef(comId.length()).compare(comId.rightRef(other.length()), Qt::CaseInsensitive) == 0);
#else
	return (QStringView(other).right(comId.length()).compare(QStringView(comId).right(other.length()), Qt::CaseInsensitive) == 0);
#endif
}

bool
KGpgKeyNode::canEncrypt() const
{
	return ((m_key->keytype() & KgpgCore::SKT_ENCRYPTION) != 0);
}

void
KGpgKeyNode::expand()
{
	if (!wasExpanded())
		readChildren();

	Q_EMIT expanded();
}
