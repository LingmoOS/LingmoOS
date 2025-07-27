/*
    SPDX-FileCopyrightText: 2009, 2010, 2013 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-FileCopyrightText: 2016 David Zaslavsky <diazona@ellipsix.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpgsearchresultmodel.h"
#include "kgpg_general_debug.h"

#include <KLocalizedString>


#include <QDateTime>
#include <QRegularExpression>
#include <QScopedPointer>
#include <QString>
#include <QTextCodec>

#include "core/convert.h"
#include "core/kgpgkey.h"

class SearchResult {
private:
	QStringList m_emails;
	QStringList m_names;

public:
	SearchResult(const QString &line);

	bool m_validPub;	// true when the "pub" line passed to constructor was valid

	void addUid(const QString &id);
	const QString &getName(const int index) const;
	const QString &getEmail(const int index) const;
	int getUidCount() const;
	bool valid() const;
	bool expired() const;
	bool revoked() const;

	QString m_fingerprint;
	unsigned int m_uatCount;

	QVariant summary() const;
private:
	QDateTime m_creation;
	bool m_expired;
	bool m_revoked;
	unsigned int m_bits;
	KgpgCore::KgpgKeyAlgo m_algo;
};

class KGpgSearchResultModelPrivate {
public:
	explicit KGpgSearchResultModelPrivate();
	~KGpgSearchResultModelPrivate();

	QList<SearchResult *> m_items;

	QString urlDecode(const QString &line);
};

SearchResult::SearchResult(const QString &line)
	: m_validPub(false),
	m_uatCount(0),
	m_expired(false),
	m_revoked(false),
	m_bits(0)
{
	const QStringList parts(line.split(QLatin1Char( ':' )));

	if (parts.count() < 6)
		return;

	if (parts.at(1).isEmpty())
		return;

	m_fingerprint = parts.at(1);
	m_algo = KgpgCore::Convert::toAlgo(parts.at(2));
	m_bits = parts.at(3).toUInt();
	m_creation.setTime_t(parts.at(4).toULongLong());
	m_expired = !parts.at(5).isEmpty() &&
			QDateTime::fromSecsSinceEpoch(parts.at(5).toULongLong()) <= QDateTime::currentDateTimeUtc();
	m_revoked = (parts.at(6) == QLatin1String( "r" ));

	m_validPub = true;
}

void
SearchResult::addUid(const QString &id)
{
	Q_ASSERT(m_emails.count() == m_names.count());
	const QRegularExpression hasmail(QRegularExpression::anchoredPattern(QStringLiteral("(.*) <(.*)>")));

	const QRegularExpressionMatch match = hasmail.match(id);
	if (match.hasMatch()) {
		m_names.append(match.captured(1));
		m_emails.append(match.captured(2));
	} else {
		m_names.append(id);
		m_emails.append(QString());
	}
}

const QString &
SearchResult::getName(const int index) const
{
	return m_names.at(index);
}

const QString &
SearchResult::getEmail(const int index) const
{
	return m_emails.at(index);
}

int
SearchResult::getUidCount() const
{
	Q_ASSERT(m_emails.count() == m_names.count());

	return m_emails.count();
}

bool
SearchResult::expired() const
{
	return m_expired;
}

bool
SearchResult::revoked() const
{
	return m_revoked;
}

bool
SearchResult::valid() const
{
	return !(revoked() || expired());
}

QVariant
SearchResult::summary() const
{
	if (m_revoked) {
		return i18nc("example: ID abc123xy, 1024-bit RSA key, created Jan 12 2009, revoked",
				"ID %1, %2-bit %3 key, created %4, revoked", m_fingerprint,
				m_bits, KgpgCore::Convert::toString(m_algo),
				m_creation.toString(Qt::SystemLocaleShortDate));
	} else {
		return i18nc("example: ID abc123xy, 1024-bit RSA key, created Jan 12 2009",
				"ID %1, %2-bit %3 key, created %4", m_fingerprint,
				m_bits, KgpgCore::Convert::toString(m_algo),
				m_creation.toString(Qt::SystemLocaleShortDate));
	}
}

KGpgSearchResultModelPrivate::KGpgSearchResultModelPrivate()
{
}

KGpgSearchResultModelPrivate::~KGpgSearchResultModelPrivate()
{
	qDeleteAll(m_items);
}

QString
KGpgSearchResultModelPrivate::urlDecode(const QString &line)
{
	if (!line.contains(QLatin1Char( '%' )))
		return line;

	QByteArray tmp(line.toLatin1());
	const QRegularExpression hex(
		QRegularExpression::anchoredPattern(QStringLiteral("[A-F0-9]{2}")));	// URL-encoding uses only uppercase

	int pos = -1;	// avoid error if '%' is URL-encoded
	while ((pos = tmp.indexOf("%", pos + 1)) >= 0) {
		const QByteArray hexnum(tmp.mid(pos + 1, 2));

		// the input is not properly URL-encoded, so assume it does not need to be decoded at all
		if (!hex.match(QLatin1String(hexnum)).hasMatch())
			return line;

		char n[2];
		// this must work as we checked the regexp before
		n[0] = hexnum.toUShort(nullptr, 16);
		n[1] = '\0';	// to use n as a 0-terminated string

		tmp.replace(pos, 3, n);
	}

	return QTextCodec::codecForName("utf8")->toUnicode(tmp);
}

KGpgSearchResultBackingModel::KGpgSearchResultBackingModel(QObject *parent)
	: QAbstractItemModel(parent), d(new KGpgSearchResultModelPrivate())
{
}

KGpgSearchResultBackingModel::~KGpgSearchResultBackingModel()
{
	delete d;
}

/*
 * In this implementation, the top-level node is identified by
 * an invalid `QModelIndex`. Sublevel nodes correspond to valid
 * `QModelIndex` instances. First-level nodes have a null `internalPointer`,
 * and the `SearchResult` that holds a key is stored as the `internalPointer`
 * of each second-level subnode under that key's first-level subnode.
 *
 * This design works better than storing pointers to the `SearchResult`s
 * in the first-level nodes because the second-level nodes need some way
 * to be linked to their parent nodes. Storing a pointer to the parent
 * `QModelIndex` in the second-level `QModelIndex` is tricky because of
 * the short lifetime of `QModelIndex` instances. However, it's
 * straightforward to get from a `SearchResult` to the corresponding
 * first-level `QModelIndex`, so effectively the `SearchResult` instances
 * do double duty as "proxy pointers" to first-level `QModelIndex`s,
 * which aren't going to disappear from memory at any moment.
 */

KGpgSearchResultBackingModel::NodeLevel
KGpgSearchResultBackingModel::nodeLevel(const QModelIndex &index)
{
	if (!index.isValid())
		return ROOT_LEVEL;
	else if (index.internalPointer() == nullptr)
		return KEY_LEVEL;
	else
		return ATTRIBUTE_LEVEL;
}

SearchResult *
KGpgSearchResultBackingModel::resultForIndex(const QModelIndex &index) const
{
	switch (nodeLevel(index)) {
	case KEY_LEVEL:
		return d->m_items.at(index.row());
	case ATTRIBUTE_LEVEL:
	{
		SearchResult *tmp = static_cast<SearchResult *>(index.internalPointer());
		Q_ASSERT(tmp != nullptr);
		return tmp;
	}
	default:
		return nullptr;
	}
}

QVariant
KGpgSearchResultBackingModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role != Qt::DisplayRole)
		return QVariant();

	if (index.row() < 0)
		return QVariant();

	SearchResult *tmp = resultForIndex(index);
	int row;

	switch (nodeLevel(index)) {
	case KEY_LEVEL:
		// this is a "top" item, show the first uid
		if (index.row() >= d->m_items.count())
			return QVariant();

		row = 0;
		break;
	case ATTRIBUTE_LEVEL:
	{
		row = index.row() + 1;
		int summaryRow = tmp->getUidCount();
		int uatRow;
		if (tmp->m_uatCount != 0) {
			uatRow = summaryRow;
			summaryRow++;
		} else {
			uatRow = -1;
		}

		if (row == uatRow) {
			if (index.column() == 0)
				return i18np("One Photo ID", "%1 Photo IDs", tmp->m_uatCount);
			else
				return QVariant();
		} else if (row == summaryRow) {
			if (index.column() == 0)
				return tmp->summary();
			else
				return QVariant();
		}
		Q_ASSERT(row < tmp->getUidCount());
		break;
	}
	case ROOT_LEVEL:
		// The root index, level 0, should have been caught by the conditional
		// if (!index.isValid()) {...} at the top of this method
		Q_ASSERT(false);
		break;
	}

	switch (index.column()) {
	case 0:
		return tmp->getName(row);
	case 1:
		return tmp->getEmail(row);
	default:
		return QVariant();
	}
}

int
KGpgSearchResultBackingModel::columnCount(const QModelIndex &parent) const
{
	switch (nodeLevel(parent)) {
	case KEY_LEVEL:
		if (parent.column() != 0)
			return 0;
		else
			return 2;
	case ATTRIBUTE_LEVEL:
		return 0;
	case ROOT_LEVEL:
		return 2;
	default:
		Q_ASSERT(false);
		return 0;
	}
}

QModelIndex
KGpgSearchResultBackingModel::index(int row, int column, const QModelIndex &parent) const
{
	switch (nodeLevel(parent)) {
	case ATTRIBUTE_LEVEL:
		return QModelIndex();
	case KEY_LEVEL:
	{
		if (parent.row() >= d->m_items.count())
			return QModelIndex();
		SearchResult *tmp = resultForIndex(parent);
		int maxRow = tmp->getUidCount();
		if (tmp->m_uatCount != 0)
			maxRow++;
		if ((row >= maxRow) || (column > 1))
			return QModelIndex();
		return createIndex(row, column, tmp);
	}
	case ROOT_LEVEL:
		if ((row >= d->m_items.count()) || (column > 1) || (row < 0) || (column < 0))
			return QModelIndex();
		return createIndex(row, column);
	default:
		Q_ASSERT(false);
		return QModelIndex();
	}
}

QModelIndex
KGpgSearchResultBackingModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex();

	switch (nodeLevel(index)) {
	case ROOT_LEVEL:
	case KEY_LEVEL:
		return QModelIndex();
	case ATTRIBUTE_LEVEL:
	{
		SearchResult *tmp = resultForIndex(index);
		return createIndex(d->m_items.indexOf(tmp), 0);
	}
	default:
		Q_ASSERT(false);
		return QModelIndex();
	}
}

int
KGpgSearchResultBackingModel::rowCount(const QModelIndex &parent) const
{
	switch (nodeLevel(parent)) {
	case ROOT_LEVEL:
		return d->m_items.count();
	case KEY_LEVEL:
		if (parent.column() == 0) {
			SearchResult *item = resultForIndex(parent);
			int cnt = item->getUidCount();
			if (item->m_uatCount != 0)
				cnt++;

			return cnt;
		} else {
			return 0;
		}
	case ATTRIBUTE_LEVEL:
		return 0;
	default:
		Q_ASSERT(false);
		return 0;
	}
}

QVariant
KGpgSearchResultBackingModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	if (orientation != Qt::Horizontal)
		return QVariant();

	switch (section) {
	case 0:
		return i18n("Name");
	case 1:
		return QString(i18nc("@title:column Title of a column of emails", "Email"));
	default:
		return QVariant();
	}
}

QString
KGpgSearchResultBackingModel::idForIndex(const QModelIndex &index) const
{
	Q_ASSERT(index.isValid());

	switch (nodeLevel(index)) {
	case KEY_LEVEL:
	case ATTRIBUTE_LEVEL:
		return resultForIndex(index)->m_fingerprint;
	default:
		// root level should have been caught at the beginning
		Q_ASSERT(false);
		return QString();
	}
}

void
KGpgSearchResultBackingModel::slotAddKey(const QStringList &lines)
{
	Q_ASSERT(!lines.isEmpty());
	Q_ASSERT(lines.first().startsWith(QLatin1String("pub:")));

	if (lines.count() == 1)
		return;

	QStringList::const_iterator it = lines.constBegin();

	QScopedPointer<SearchResult> nkey(new SearchResult(*it));
	if (!nkey->m_validPub)
		return;

	const QStringList::const_iterator itEnd = lines.constEnd();
	for (it++; it != itEnd; it++) {
		const QString &line = *it;
		if (line.startsWith(QLatin1String("uid:"))) {
			QString kid = d->urlDecode(line.section(QLatin1Char( ':' ), 1, 1));

			nkey->addUid(kid);
		} else if (line.startsWith(QLatin1String("uat:"))) {
			nkey->m_uatCount++;
		} else {
			qCDebug(KGPG_LOG_GENERAL) << "ignored search result line" << line;
		}
	}

	if (nkey->getUidCount() > 0) {
		beginInsertRows(QModelIndex(), d->m_items.count(), d->m_items.count());
		d->m_items.append(nkey.take());
		endInsertRows();
	}
}

KGpgSearchResultModel::KGpgSearchResultModel(QObject *parent)
	: QSortFilterProxyModel(parent),
	m_filterByValidity(true)
{
	resetSourceModel();
}

KGpgSearchResultModel::~KGpgSearchResultModel()
{
}

bool
KGpgSearchResultModel::filterByValidity() const
{
	return m_filterByValidity;
}

QString
KGpgSearchResultModel::idForIndex(const QModelIndex &index) const
{
	return static_cast<KGpgSearchResultBackingModel *>(sourceModel())->idForIndex(mapToSource(index));
}

int
KGpgSearchResultModel::sourceRowCount(const QModelIndex &parent) const
{
	return sourceModel()->rowCount(parent);
}

void
KGpgSearchResultModel::setFilterByValidity(bool filter)
{
	m_filterByValidity = filter;
	invalidateFilter();
}

void
KGpgSearchResultModel::setSourceModel(QAbstractItemModel *)
{
	Q_ASSERT(false);
}

void
KGpgSearchResultModel::slotAddKey(const QStringList &key)
{
	static_cast<KGpgSearchResultBackingModel *>(sourceModel())->slotAddKey(key);
}

void
KGpgSearchResultModel::resetSourceModel()
{
	QAbstractItemModel *oldSourceModel = sourceModel();
	if (oldSourceModel != nullptr)
		oldSourceModel->deleteLater();
	QSortFilterProxyModel::setSourceModel(new KGpgSearchResultBackingModel(this));
}

bool
KGpgSearchResultModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	// first check the text filter, implemented in the superclass
	if (!QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent)) {
		return false;
	} else if (!filterByValidity()) {
		// if the text filter matched and we're not hiding invalid keys, accept the row
		return true;
	}
	// otherwise, validity filtering is enabled, so check whether the row is valid
	KGpgSearchResultBackingModel *backingModel = static_cast<KGpgSearchResultBackingModel *>(sourceModel());
	QModelIndex currentKeyIndex = backingModel->index(sourceRow, 0, sourceParent);
	return backingModel->resultForIndex(currentKeyIndex)->valid();
}
