/*
    SPDX-FileCopyrightText: 2008, 2009, 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpgdeluid.h"

#include "gpgproc.h"
#include "core/kgpgkey.h"
#include "core/KGpgKeyNode.h"

#include <algorithm>

KGpgDelUid::KGpgDelUid(QObject *parent, const KGpgSignableNode *uid)
	: KGpgUidTransaction(parent, uid->getParentKeyNode()->getId(), uid->getId()),
	m_fixargs(addArgument(QLatin1String( "deluid" )))
{
	setUid(uid);
}

KGpgDelUid::KGpgDelUid(QObject *parent, const KGpgSignableNode::const_List &uids)
	: KGpgUidTransaction(parent),
	m_fixargs(addArgument(QLatin1String( "deluid" )))
{
	setUids(uids);
}

KGpgDelUid::KGpgDelUid(QObject *parent, const KGpgKeyNode *keynode, const int uid, const RemoveMode removeMode)
	: KGpgUidTransaction(parent),
	m_fixargs(addArgument(QLatin1String( "deluid" )))
{
	setUid(keynode, uid, removeMode);
}

KGpgDelUid::~KGpgDelUid()
{
}

void
KGpgDelUid::setUid(const KGpgSignableNode *uid)
{
	KGpgSignableNode::const_List uids;

	uids.append(uid);
	setUids(uids);
}

void
KGpgDelUid::setUids(const KGpgSignableNode::const_List &uids)
{
	Q_ASSERT(!uids.isEmpty());

	m_uids = uids;

	GPGProc *proc = getProcess();

	QStringList args(proc->program());
	proc->clearProgram();

	for (int i = args.count() - m_fixargs - 1; i > 0; i--)
		args.removeLast();

	std::sort(m_uids.begin(), m_uids.end(),
			[](const KGpgSignableNode *s, const KGpgSignableNode *t) { return *t < *s; });

	const KGpgSignableNode *nd = m_uids.first();
	const KGpgExpandableNode *parent;
	if (nd->getType() & KgpgCore::ITYPE_PAIR)
		parent = nd;
	else
		parent = nd->getParentKeyNode();

	for (auto it = ++m_uids.constBegin(); it != m_uids.constEnd(); it++) {
		nd = *it;
		Q_ASSERT((nd->getParentKeyNode() == parent) || (nd == parent));

		args.append(QLatin1String( "uid" ));
		if (nd->getType() & KgpgCore::ITYPE_PAIR)
			args.append(QLatin1String("1"));
		else
			args.append(nd->getId());
		args.append(QLatin1String( "deluid" ));
	}

	proc->setProgram(args);
	nd = m_uids.first();

	switch (nd->getType()) {
	case KgpgCore::ITYPE_PUBLIC:
	case KgpgCore::ITYPE_PAIR:
		KGpgUidTransaction::setUid(1);
		break;
	default:
		KGpgUidTransaction::setUid(nd->getId());
		break;
	}
	setKeyId(parent->getId());
}

void
KGpgDelUid::setUid(const KGpgKeyNode *keynode, const int uid, const RemoveMode removeMode)
{
	Q_ASSERT(uid != 0);

	KGpgSignableNode::const_List uids;
	const KGpgSignableNode *uidnode;

	if (uid > 0) {
		uidnode = keynode->getUid(uid);

		Q_ASSERT(uidnode != nullptr);
		uids.append(uidnode);
	} else {
		Q_ASSERT(keynode->wasExpanded());
		int idx = 0;

        while(1) {
			idx++;
			if (idx == -uid)
				continue;

			uidnode = keynode->getUid(idx);

			if (uidnode == nullptr)
				break;

			switch (removeMode) {
			case RemoveAllOther:
				uids.append(uidnode);
				break;
			case KeepUats:
				if (uidnode->getType() != KgpgCore::ITYPE_UAT)
					uids.append(uidnode);
				break;
			case RemoveWithEmail:
				if (!uidnode->getEmail().isEmpty())
					uids.append(uidnode);
				break;
			}
		}
	}

	if (!uids.isEmpty())
		setUids(uids);
}

bool
KGpgDelUid::preStart()
{
	if (m_uids.isEmpty()) {
		setSuccess(TS_NO_SUCH_UID);
		return false;
	}

	return true;
}

bool
KGpgDelUid::nextLine(const QString &line)
{
	return standardCommands(line);
}

KGpgTransaction::ts_boolanswer
KGpgDelUid::boolQuestion(const QString& line)
{
	if (line == QLatin1String("keyedit.remove.uid.okay")) {
		m_uids.removeFirst();
		return BA_YES;
	} else {
		return KGpgTransaction::boolQuestion(line);
	}
}

void
KGpgDelUid::finish()
{
	if (!m_uids.isEmpty())
		setSuccess(TS_MSG_SEQUENCE);
}
