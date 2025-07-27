/*
    SPDX-FileCopyrightText: 2010, 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpgdelsign.h"

#include "model/kgpgitemnode.h"
#include "gpgproc.h"

#include <QStringList>

KGpgDelSign::KGpgDelSign(QObject *parent, const KGpgSignNode::List &signids)
	: KGpgUidTransaction(parent, signids.at(0)->getParentKeyNode()->getId())
{
	addArgument(QLatin1String( "delsig" ));

	const QStringList args = getProcess()->program();

	// If we run with --no-tty GnuPG will not tell which sign it is currently
	// asking to remove :(
	int ntty = args.indexOf(QLatin1String("--no-tty"));
	if (ntty >= 0)
		replaceArgument(ntty, QLatin1String("--with-colons"));
	else
		insertArgument(1, QLatin1String( "--with-colons" ));

	if (signids.at(0)->getParentKeyNode()->getType() & KgpgCore::ITYPE_PUBLIC)
		setUid(QLatin1String( "1" ));
	else
		setUid(signids.at(0)->getParentKeyNode()->getId());

#ifndef QT_NO_DEBUG
	for (const KGpgSignNode *snode : signids) {
		Q_ASSERT(signids.at(0)->getParentKeyNode() == snode->getParentKeyNode());
	}
#endif

	setSignIds(signids);
}

KGpgDelSign::KGpgDelSign(QObject* parent, KGpgSignNode *signid)
	: KGpgUidTransaction(parent, signid->getParentKeyNode()->getId())
{
	addArgument(QLatin1String( "delsig" ));
	insertArgument(1, QLatin1String( "--with-colons" ));

	if (signid->getParentKeyNode()->getType() & KgpgCore::ITYPE_PUBLIC)
		setUid(QLatin1String( "1" ));
	else
		setUid(signid->getParentKeyNode()->getId());

	setSignId(signid);
}


KGpgDelSign::~KGpgDelSign()
{
}

KGpgSignNode::List KGpgDelSign::getSignIds(void) const
{
	return m_signids;
}

void KGpgDelSign::setSignId(KGpgSignNode* keyid)
{
	m_signids.clear();
	m_signids << keyid;
}

void KGpgDelSign::setSignIds(const KGpgSignNode::List &keyids)
{
	m_signids = keyids;
}

bool
KGpgDelSign::nextLine(const QString &line)
{
	if (line.startsWith(QLatin1String("sig:"))) {
		m_cachedid = line;
		return false;
	} else if (line.startsWith(QLatin1String("[GNUPG:] "))) {
		return standardCommands(line);
	} else {
		// GnuPG will tell us a bunch of stuff because we are not in
		// --no-tty mode but we don't care.
		return false;
	}
}

KGpgTransaction::ts_boolanswer
KGpgDelSign::boolQuestion(const QString &line)
{
	if (line.startsWith(QLatin1String("keyedit.delsig."))) {
		const QStringList parts = m_cachedid.split(QLatin1Char( ':' ));

		if (parts.count() < 7)
			return KGpgTransaction::BA_NO;

		const QString &sigid = parts[4];
		const int snlen = sigid.length();
		KGpgSignNode *signode = nullptr;

		for (KGpgSignNode *snode : qAsConst(m_signids)) {
			if (snode->getId().rightRef(snlen).compare(sigid) == 0) {
				signode = snode;
				break;
			}
		}

		if (signode == nullptr)
			return KGpgTransaction::BA_NO;

		const QDateTime creation = QDateTime::fromSecsSinceEpoch(parts[5].toUInt());
		if (creation != signode->getCreation())
			return KGpgTransaction::BA_NO;

		QDateTime sigexp;
		if (!parts[6].isEmpty() && (parts[6] != QLatin1String("0")))
			sigexp = QDateTime::fromSecsSinceEpoch(parts[6].toUInt());
		if (sigexp != signode->getExpiration())
			return KGpgTransaction::BA_NO;

		m_signids.removeOne(signode);
		return KGpgTransaction::BA_YES;
	} else {
		return KGpgTransaction::boolQuestion(line);
	}
}
