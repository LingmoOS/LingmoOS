/*
    SPDX-FileCopyrightText: 2008, 2009, 2010, 2012, 2016, 2017 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "KGpgGroupNode.h"

#include "kgpg_general_debug.h"
#include "KGpgGroupMemberNode.h"
#include "KGpgRootNode.h"
#include "kgpgsettings.h"

#include <KLocalizedString>


#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

class KGpgGroupNodePrivate {
public:
	KGpgGroupNodePrivate(const QString &name);

	QString m_name;

	/**
	 * @brief find the line that defines this group in the configuration
	 * @param conffile file object (will be initialized)
	 * @param stream text stream (will be initialized and connected to conffile)
	 * @param lines the lines found in conffile (will be filled)
	 * @return the index in lines of the entry defining this group
	 * @retval -1 no entry defining this group was found
	 *
	 * stream will be positioned at the beginning.
	 */
	int findGroupEntry(QFile &conffile, QTextStream &stream, QStringList &lines);

	static const QRegularExpression &groupPattern();
	static const QString &groupTag();
};

KGpgGroupNodePrivate::KGpgGroupNodePrivate(const QString &name)
	: m_name(name)
{
}

int
KGpgGroupNodePrivate::findGroupEntry(QFile &conffile, QTextStream &stream, QStringList &lines)
{
	conffile.setFileName(KGpgSettings::gpgConfigPath());

	if (!conffile.exists())
		return -1;

	if (!conffile.open(QIODevice::ReadWrite))
		return -1;

	stream.setDevice(&conffile);
	int index = -1;
	int i = -1;

	while (!stream.atEnd()) {
		const QString rawLine = stream.readLine();
		i++;
		QString parsedLine = rawLine.simplified().section(QLatin1Char('#'), 0, 0);

		if (groupPattern().match(parsedLine).hasMatch()) {
			// remove "group "
			parsedLine.remove(0, 6);
			if (parsedLine.startsWith(m_name)) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
				if (parsedLine.midRef(m_name.length()).trimmed().startsWith(QLatin1Char('='))) {
#else
				if (QStringView(parsedLine).mid(m_name.length()).trimmed().startsWith(QLatin1Char('='))) {
#endif
					if (index >= 0) {
						// multiple definitions of the same group, drop the second one
						continue;
					} else {
						index = i;
					}
				}
			}
		}

		lines << rawLine;
	}

	stream.seek(0);

	return index;
}

const QRegularExpression &
KGpgGroupNodePrivate::groupPattern()
{
	static const QRegularExpression groupre(QStringLiteral("^group [^ ]+ ?= ?([0-9a-fA-F]{8,} ?)*$"));

	return groupre;
}

const QString &
KGpgGroupNodePrivate::groupTag()
{
	static const QString grouptag(QLatin1String("group "));

	return grouptag;
}

KGpgGroupNode::KGpgGroupNode(KGpgRootNode *parent, const QString &name, const QStringList &members)
	: KGpgExpandableNode(parent),
	d_ptr(new KGpgGroupNodePrivate(name))
{
	for (const QString &id : members)
		if (id.startsWith(QLatin1String("0x")))
			new KGpgGroupMemberNode(this, id.mid(2));
		else
			new KGpgGroupMemberNode(this, id);

	parent->m_groups++;
}

KGpgGroupNode::KGpgGroupNode(KGpgRootNode *parent, const QString &name, const KGpgKeyNode::List &members)
	: KGpgExpandableNode(parent),
	d_ptr(new KGpgGroupNodePrivate(name))
{
	Q_ASSERT(!members.isEmpty());

	for (KGpgKeyNode *nd : members)
		new KGpgGroupMemberNode(this, nd);

	parent->m_groups++;
}

KGpgGroupNode::~KGpgGroupNode()
{
	if (parent() != nullptr)
		m_parent->toRootNode()->m_groups--;
}

KgpgCore::KgpgItemType
KGpgGroupNode::getType() const
{
	return ITYPE_GROUP;
}

QString
KGpgGroupNode::getName() const
{
	const Q_D(KGpgGroupNode);

	return d->m_name;
}

QString
KGpgGroupNode::getSize() const
{
	return i18np("1 key", "%1 keys", children.count());
}

void
KGpgGroupNode::readChildren()
{
}

void
KGpgGroupNode::rename(const QString &newName)
{
	Q_D(KGpgGroupNode);

	QFile conffile;
	QTextStream t;
	QStringList lines;
	int index = d->findGroupEntry(conffile, t, lines);

	// check if file opening failed
	if (!t.device())
		return;

	if (index < 0) {
		qCDebug(KGPG_LOG_GENERAL) << "Group " << d->m_name << " not renamed, group does not exist";
		return;
	}

	// 6 = groupTag().length()
	const QString values = lines[index].simplified().mid(6 + d->m_name.length());
	lines[index] = d->groupTag() + newName + QLatin1Char(' ') + values;

	conffile.resize(0);
	t << lines.join(QLatin1String("\n")) + QLatin1Char('\n');

	d->m_name = newName;
}

void
KGpgGroupNode::saveMembers()
{
	Q_D(KGpgGroupNode);

	QFile conffile;
	QTextStream t;
	QStringList lines;
	int index = d->findGroupEntry(conffile, t, lines);

	// check if file opening failed
	if (!t.device())
		return;

	QStringList memberIds;

	for (int j = getChildCount() - 1; j >= 0; j--)
		memberIds << getChild(j)->toGroupMemberNode()->getId();

	const QString groupEntry = d->groupTag() + d->m_name + QLatin1String(" = ") +
			memberIds.join(QLatin1Char(' '));

	if (index >= 0)
		lines[index] = groupEntry;
	else
		lines << groupEntry;

	conffile.resize(0);
	t << lines.join(QLatin1String("\n")) + QLatin1Char('\n');
}

void
KGpgGroupNode::remove()
{
	Q_D(KGpgGroupNode);

	QFile conffile;
	QTextStream t;
	QStringList lines;
	int index = d->findGroupEntry(conffile, t, lines);

	// check if file opening failed
	if (!t.device())
		return;

	if (index < 0)
		return;

	lines.removeAt(index);
	conffile.resize(0);
	t << lines.join(QLatin1String("\n")) + QLatin1Char('\n');
}
