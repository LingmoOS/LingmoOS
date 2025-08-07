/*
    SPDX-FileCopyrightText: 2008, 2009, 2010, 2013 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "KGpgSignNode.h"

#include "KGpgSignableNode.h"

#include <KLocalizedString>

class KGpgSignNodePrivate {
public:
	KGpgSignNodePrivate(const QStringList &sl);

	QDateTime m_creation;
	QDateTime m_expiration;
	bool m_local;
	bool m_revocation;
};

KGpgSignNodePrivate::KGpgSignNodePrivate(const QStringList &sl)
	: m_local(false)
{
	Q_ASSERT(!sl.isEmpty());
	m_revocation = (sl.at(0) == QLatin1String("rev"));
	if (sl.count() < 6)
		return;
	m_creation = QDateTime::fromSecsSinceEpoch(sl.at(5).toUInt());
	if (sl.count() < 7)
		return;
	if (!sl.at(6).isEmpty())
		m_expiration = QDateTime::fromSecsSinceEpoch(sl.at(6).toUInt());
	if (sl.count() < 11)
		return;
	m_local = sl.at(10).endsWith(QLatin1Char( 'l' ));
}

KGpgSignNode::KGpgSignNode(KGpgSignableNode *parent, const QStringList &s)
	: KGpgRefNode(parent, s.at(4)),
	d_ptr(new KGpgSignNodePrivate(s))
{
}

KGpgSignNode::~KGpgSignNode()
{
	delete d_ptr;
}

QDateTime
KGpgSignNode::getExpiration() const
{
	const Q_D(KGpgSignNode);

	return d->m_expiration;
}

QDateTime
KGpgSignNode::getCreation() const
{
	const Q_D(KGpgSignNode);

	return d->m_creation;
}

QString
KGpgSignNode::getName() const
{
	const Q_D(KGpgSignNode);
	const QString name = KGpgRefNode::getName();

	if (!d->m_local)
		return name;

	return i18n("%1 [local signature]", name);
}

KgpgCore::KgpgItemType
KGpgSignNode::getType() const
{
	return KgpgCore::ITYPE_SIGN;
}
