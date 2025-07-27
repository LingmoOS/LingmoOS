/*
    SPDX-FileCopyrightText: 2008, 2009, 2010 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "KGpgUidNode.h"

#include "KGpgKeyNode.h"
#include "convert.h"

#include <QStringList>

class KGpgUidNodePrivate {
public:
	KGpgUidNodePrivate(const unsigned int index, const QStringList &sl);

	const QString m_index;
	QString m_email;
	QString m_name;
	QString m_comment;
	QDateTime m_creation;
	KgpgCore::KgpgKeyTrust m_trust;
	bool m_valid;
};

KGpgUidNodePrivate::KGpgUidNodePrivate(const unsigned int index, const QStringList &sl)
	: m_index(QString::number(index))
{
	QString fullname(sl.at(9));
	if (fullname.contains(QLatin1Char( '<' )) ) {
		m_email = fullname;

		if (fullname.contains(QLatin1Char( ')' )) )
			m_email = m_email.section(QLatin1Char( ')' ), 1);

		m_email = m_email.section(QLatin1Char( '<' ), 1);
		m_email.chop(1);

		if (m_email.contains(QLatin1Char( '<' ))) {
			// several email addresses in the same key
			m_email.replace(QLatin1Char( '>' ), QLatin1Char( ';' ));
			m_email.remove(QLatin1Char( '<' ));
		}
	}

	m_name = fullname.section(QLatin1String( " <" ), 0, 0);
	if (fullname.contains(QLatin1Char( '(' )) ) {
		m_name = m_name.section(QLatin1String( " (" ), 0, 0);
		m_comment = fullname.section(QLatin1Char( '(' ), 1, 1);
		m_comment = m_comment.section(QLatin1Char( ')' ), 0, 0);
	}

	m_trust = KgpgCore::Convert::toTrust(sl.at(1));
	m_valid = ((sl.count() <= 11) || !sl.at(11).contains(QLatin1Char( 'D' )));
	m_creation = KgpgCore::Convert::toDateTime(sl.at(5));
}


KGpgUidNode::KGpgUidNode(KGpgKeyNode *parent, const unsigned int index, const QStringList &sl)
	: KGpgSignableNode(parent),
	d_ptr(new KGpgUidNodePrivate(index, sl))
{
}

KGpgUidNode::~KGpgUidNode()
{
	delete d_ptr;
}

QString
KGpgUidNode::getName() const
{
	const Q_D(KGpgUidNode);

	return d->m_name;
}

QString
KGpgUidNode::getEmail() const
{
	const Q_D(KGpgUidNode);

	return d->m_email;
}

QString
KGpgUidNode::getId() const
{
	const Q_D(KGpgUidNode);

	return d->m_index;
}

QDateTime
KGpgUidNode::getCreation() const
{
	const Q_D(KGpgUidNode);

	return d->m_creation;
}

KGpgKeyNode *
KGpgUidNode::getKeyNode(void)
{
	return getParentKeyNode()->toKeyNode();
}

const KGpgKeyNode *
KGpgUidNode::getKeyNode(void) const
{
	return getParentKeyNode()->toKeyNode();
}

KGpgKeyNode *
KGpgUidNode::getParentKeyNode() const
{
	return m_parent->toKeyNode();
}

void
KGpgUidNode::readChildren()
{
}

KgpgCore::KgpgItemType
KGpgUidNode::getType() const
{
	return KgpgCore::ITYPE_UID;
}

KgpgCore::KgpgKeyTrust
KGpgUidNode::getTrust() const
{
	const Q_D(KGpgUidNode);

	return d->m_trust;
}

QString
KGpgUidNode::getComment() const
{
	const Q_D(KGpgUidNode);

	return d->m_comment;
}
