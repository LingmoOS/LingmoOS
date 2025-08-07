/*
    SPDX-FileCopyrightText: 2008, 2009, 2017 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "KGpgSignableNode.h"

#include <KLocalizedString>

KGpgSignableNode::KGpgSignableNode(KGpgExpandableNode *parent)
	: KGpgExpandableNode(parent)
{
}

KGpgSignableNode::~KGpgSignableNode()
{
}

KGpgSignNode::List
KGpgSignableNode::getSignatures(void) const
{
	KGpgSignNode::List ret;

	for (KGpgNode *kn : children) {
		if (kn->getType() == KgpgCore::ITYPE_SIGN)
			ret << kn->toSignNode();
	}

	return ret;
}

QString
KGpgSignableNode::getSignCount() const
{
	return i18np("1 signature", "%1 signatures", children.count());
}

bool
KGpgSignableNode::operator<(const KGpgSignableNode &other) const
{
	return operator<(&other);
}

bool
KGpgSignableNode::operator<(const KGpgSignableNode *other) const
{
	switch (getType()) {
	case KgpgCore::ITYPE_PUBLIC:
	case KgpgCore::ITYPE_PAIR: {
		const QString myid(getId());

		switch (other->getType()) {
		case KgpgCore::ITYPE_PUBLIC:
		case KgpgCore::ITYPE_PAIR:
			return (myid < other->getId());
		default: {
			const QString otherid(other->getParentKeyNode()->getId());

			if (myid == otherid)
				return true;
			return (myid < otherid);
		}
		}
	}
	default: {
		const QString myp(getParentKeyNode()->getId());

		switch (other->getType()) {
		case KgpgCore::ITYPE_PAIR:
		case KgpgCore::ITYPE_PUBLIC:
			return (myp < other->getId());
		default: {
			const QString otherp(other->getParentKeyNode()->getId());

			if (otherp == myp)
				return (getId().toInt() < other->getId().toInt());

			return (myp < otherp);
		}
		}
	}
	}
}
