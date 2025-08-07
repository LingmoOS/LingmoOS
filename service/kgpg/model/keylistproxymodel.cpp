/*
    SPDX-FileCopyrightText: 2008, 2009, 2010, 2012, 2013 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-FileCopyrightText: 2013 Thomas Fischer <fischer@unix-ag.uni-kl.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "keylistproxymodel.h"
#include "model/kgpgitemnode.h"
#include "kgpgitemmodel.h"
#include "kgpgsettings.h"
#include "core/images.h"

#include <KLocalizedString>
#include <QDate>

using namespace KgpgCore;

class KeyListProxyModelPrivate {
	KeyListProxyModel * const q_ptr;

	Q_DECLARE_PUBLIC(KeyListProxyModel)
public:
	KeyListProxyModelPrivate(KeyListProxyModel *parent, const KeyListProxyModel::DisplayMode mode);

	bool lessThan(const KGpgNode *left, const KGpgNode *right, const int column) const;
	bool nodeLessThan(const KGpgNode *left, const KGpgNode *right, const int column) const;
	KGpgItemModel *m_model;
	bool m_onlysecret;
	bool m_encryptionKeys;
	KgpgCore::KgpgKeyTrustFlag m_mintrust;
	int m_previewsize;
	int m_idLength;
	KeyListProxyModel::DisplayMode m_displaymode;
	int m_emailSorting;

	QString reorderEmailComponents(const QString &emailAddress) const;
	QVariant dataSingleColumn(const QModelIndex &index, int role, const KGpgNode *node) const;
	QVariant dataMultiColumn(const QModelIndex &index, int role, const KGpgNode *node) const;
};

KeyListProxyModelPrivate::KeyListProxyModelPrivate(KeyListProxyModel *parent, const KeyListProxyModel::DisplayMode mode)
	: q_ptr(parent),
	m_model(nullptr),
	m_onlysecret(false),
	m_encryptionKeys(false),
	m_mintrust(TRUST_UNKNOWN),
	m_previewsize(22),
	m_idLength(8),
	m_displaymode(mode),
	m_emailSorting(KGpgSettings::emailSorting())
{
}

/**
 * Reverses the list's order (this modifies the list) and returns
 * a string containing the reversed list's elements joined by a char.
 */
static QString reverseListAndJoinWithChar(const QStringList &list, const QChar &separator)
{
	QString result = list.last();
	for (int i = list.count() - 2; i >= 0; --i)
		result.append(separator).append(list[i]);
	return result;
}

QString
KeyListProxyModelPrivate::reorderEmailComponents(const QString &emailAddress) const
{
	if (emailAddress.isEmpty())
		return QString();

	/// split email addresses at @
	static const QChar charAt = QLatin1Char('@');
	/// split domain at .
	static const QChar charDot = QLatin1Char('.');

	QString result = emailAddress;

	switch (m_emailSorting) {
	case KGpgSettings::EnumEmailSorting::TLDfirst:
	{
		/// get components of an email address
		/// john.doe@mail.kde.org becomes [john.doe, mail.kde.org]
		const QStringList emailComponents = result.split(charAt);
		if (emailComponents.count() != 2) /// expect an email address to contain exactly one @
			break;
		/// get components of a domain
		/// mail.kde.org becomes [mail, kde, org]
		const QString fqdn = emailComponents.last();
		QStringList fqdnComponents = fqdn.split(charDot);
		if (fqdnComponents.count() < 2) /// if domain consists of less than two components ...
			return fqdn + charDot + emailComponents.first(); /// ... take shortcut
		/// prepend localpart, will be last after list is reversed
		fqdnComponents.insert(0, emailComponents.first());
		/// reverse components of domain, result becomes e.g. org.kde.mail
		/// with localpart already in the list it becomes org.kde.mail.john.doe
		result = reverseListAndJoinWithChar(fqdnComponents, charDot);
		break;
	}
	case KGpgSettings::EnumEmailSorting::DomainFirst:
	{
		/// get components of an email address
		/// john.doe@mail.kde.org becomes [john.doe, mail.kde.org]
		const QStringList emailComponents = result.split(charAt);
		if (emailComponents.count() != 2) /// expect an email address to contain exactly one @
			break;
		/// get components of a domain
		/// mail.kde.org becomes [mail, kde, org]
		const QString fqdn = emailComponents.last();
		QStringList fqdnComponents = fqdn.split(charDot);
		if (fqdnComponents.count() < 2) /// if domain consists of less than two components ...
			return fqdn + charDot + emailComponents.first(); /// ... take shortcut
		/// reverse last two components of domain, becomes e.g. kde.org
		/// TODO will fail for three-part domains like kde.org.uk
		result = charDot + fqdnComponents.takeLast();
		result.prepend(fqdnComponents.takeLast());
		/// append remaining components of domain, becomes e.g. kde.org.mail
		result.append(charDot).append(fqdnComponents.join(charDot));
		/// append user name component of email address, becomes e.g. kde.org.mail.john.doe
		result.append(charDot).append(emailComponents.first());
		break;
	}
	case KGpgSettings::EnumEmailSorting::FQDNFirst:
	{
		/// get components of an email address
		/// john.doe@mail.kde.org becomes [john.doe, mail.kde.org]
		const QStringList emailComponents = result.split(charAt);
		/// assemble result by joining components in reverse order,
		/// separated by a dot, becomes e.g. mail.kde.org.john.doe
		result = reverseListAndJoinWithChar(emailComponents, charDot);
		break;
	}
	case KGpgSettings::EnumEmailSorting::Alphabetical:
		/// do not modify email address
		break;
	}

	return result;
}

QVariant
KeyListProxyModelPrivate::dataSingleColumn(const QModelIndex &index, int role, const KGpgNode *node) const
{
	Q_Q(const KeyListProxyModel);

	if (index.column() != 0)
		return QVariant();

	switch (role) {
	case Qt::DecorationRole:
		if (node->getType() == ITYPE_UAT) {
			if (m_previewsize > 0) {
				const KGpgUatNode *nd = node->toUatNode();
				return nd->getPixmap().scaled(m_previewsize + 5, m_previewsize, Qt::KeepAspectRatio);
			} else {
				return Images::photo();
			}
		} else {
			return m_model->data(q->mapToSource(index), Qt::DecorationRole);
		}
	case Qt::DisplayRole: {
		const QModelIndex srcidx(q->mapToSource(index));
		const int srcrow = srcidx.row();

		const QModelIndex ididx(srcidx.sibling(srcrow, KEYCOLUMN_ID));
		const QString id(m_model->data(ididx, Qt::DisplayRole).toString().right(m_idLength));

		const QModelIndex mailidx(srcidx.sibling(srcrow, KEYCOLUMN_EMAIL));
		const QString mail(m_model->data(mailidx, Qt::DisplayRole).toString());

		const QModelIndex nameidx(srcidx.sibling(srcrow, KEYCOLUMN_NAME));
		const QString name(m_model->data(nameidx, Qt::DisplayRole).toString());

		if (m_displaymode == KeyListProxyModel::SingleColumnIdFirst) {
			if (mail.isEmpty())
				return i18nc("ID: Name", "%1: %2", id, name);
			else
				return i18nc("ID: Name <Email>", "%1: %2 <%3>", id, name, mail);
		} else {
			if (mail.isEmpty())
				return i18nc("Name: ID", "%1: %2", name, id);
			else
				return i18nc("Name <Email>: ID", "%1 <%2>: %3", name, mail, id);
		}
		}
	case Qt::ToolTipRole: {
		const QModelIndex srcidx(q->mapToSource(index));
		const int srcrow = srcidx.row();

		const QModelIndex ididx(srcidx.sibling(srcrow, KEYCOLUMN_ID));
		return m_model->data(ididx, Qt::DisplayRole);
		}
	default:
		return QVariant();
	}
}

QVariant
KeyListProxyModelPrivate::dataMultiColumn(const QModelIndex &index, int role, const KGpgNode *node) const
{
	Q_Q(const KeyListProxyModel);

	if ((node->getType() == ITYPE_UAT) && (role == Qt::DecorationRole) && (index.column() == 0)) {
		if (m_previewsize > 0) {
			const KGpgUatNode *nd = node->toUatNode();
			return nd->getPixmap().scaled(m_previewsize + 5, m_previewsize, Qt::KeepAspectRatio);
		} else {
			return Images::photo();
		}
	} else if ((role == Qt::DisplayRole) && (index.column() == KEYCOLUMN_ID)) {
		QString id = m_model->data(q->mapToSource(index), Qt::DisplayRole).toString();
		return id.right(m_idLength);
	}
	return m_model->data(q->mapToSource(index), role);
}

KeyListProxyModel::KeyListProxyModel(QObject *parent, const DisplayMode mode)
	: QSortFilterProxyModel(parent),
	d_ptr(new KeyListProxyModelPrivate(this, mode))
{
	setFilterCaseSensitivity(Qt::CaseInsensitive);
	setFilterKeyColumn(-1);
	setDynamicSortFilter(true);
}

KeyListProxyModel::~KeyListProxyModel()
{
	delete d_ptr;
}

bool
KeyListProxyModel::hasChildren(const QModelIndex &idx) const
{
	return sourceModel()->hasChildren(mapToSource(idx));
}

void
KeyListProxyModel::setKeyModel(KGpgItemModel *md)
{
	Q_D(KeyListProxyModel);

	d->m_model = md;
	setSourceModel(md);
}

bool
KeyListProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
	Q_D(const KeyListProxyModel);

	KGpgNode *l = d->m_model->nodeForIndex(left);
	KGpgNode *r = d->m_model->nodeForIndex(right);

	return d->lessThan(l, r, left.column());
}

bool
KeyListProxyModelPrivate::lessThan(const KGpgNode *left, const KGpgNode *right, const int column) const
{
	const KGpgRootNode * const r = m_model->getRootNode();
	Q_ASSERT(r != left);
	Q_ASSERT(r != right);

	if (r == left->getParentKeyNode()) {
		if (r == right->getParentKeyNode()) {
			if (left->getType() == ITYPE_GROUP) {
				if (right->getType() == ITYPE_GROUP)
					return left->getName() < right->getName();
				else
					return true;
			} else if (right->getType() == ITYPE_GROUP)
				return false;

			// we don't need to care about group members here because they will never have root as parent
			bool test1 = (left->getType() & ITYPE_PUBLIC) && !(left->getType() & ITYPE_SECRET); // only a public key
			bool test2 = (right->getType() & ITYPE_PUBLIC) && !(right->getType() & ITYPE_SECRET); // only a public key

			// key-pair goes before simple public key
			// extra check needed to get sorting by trust right
			if (left->getType() == ITYPE_PAIR && test2) return (column != KEYCOLUMN_TRUST);
			if (right->getType() == ITYPE_PAIR && test1) return (column == KEYCOLUMN_TRUST);

			return nodeLessThan(left, right, column);
		} else {
			return lessThan(left, right->getParentKeyNode(), column);
		}
	} else {
		if (r == right->getParentKeyNode()) {
			return lessThan(left->getParentKeyNode(), right, column);
		} else if (left->getParentKeyNode() == right->getParentKeyNode()) {
			if (left->getType() != right->getType())
				return (left->getType() < right->getType());

			return nodeLessThan(left, right, column);
		} else {
			return lessThan(left->getParentKeyNode(), right->getParentKeyNode(), column);
		}
	}
	return false;
}

bool
KeyListProxyModelPrivate::nodeLessThan(const KGpgNode *left, const KGpgNode *right, const int column) const
{
	Q_ASSERT(left->getType() == right->getType());

	switch (column) {
	case KEYCOLUMN_NAME:
		if (left->getType() == ITYPE_SIGN) {
			const bool leftIsId = static_cast<const KGpgSignNode *>(left)->getRefNode() == nullptr;
			const bool rightIsId = static_cast<const KGpgSignNode *>(right)->getRefNode() == nullptr;
			if (leftIsId && !rightIsId)
				return false;
			else if (!leftIsId && rightIsId)
				return true;
			else if (leftIsId && rightIsId)
				return (left->getId() < right->getId());
		}
		return (left->getName().compare(right->getName(), Qt::CaseInsensitive) < 0);
	case KEYCOLUMN_EMAIL:
		/// reverse email address to sort by TLD first, then domain, and account name last
		return (reorderEmailComponents(left->getEmail()).compare(reorderEmailComponents(right->getEmail()), Qt::CaseInsensitive) < 0);
	case KEYCOLUMN_TRUST:
		return (left->getTrust() < right->getTrust());
	case KEYCOLUMN_EXPIR:
		return (left->getExpiration() < right->getExpiration());
	case KEYCOLUMN_SIZE:
		if ((left->getType() & ITYPE_PAIR) && (right->getType() & ITYPE_PAIR)) {
			unsigned int lsign, lenc, rsign, renc;

			if (left->getType() & ITYPE_GROUP) {
				const KGpgGroupMemberNode *g = static_cast<const KGpgGroupMemberNode *>(left);

				lsign = g->getSignKeySize();
				lenc = g->getEncryptionKeySize();
			} else {
				const KGpgKeyNode *g = static_cast<const KGpgKeyNode *>(left);

				lsign = g->getSignKeySize();
				lenc = g->getEncryptionKeySize();
			}

			if (right->getType() & ITYPE_GROUP) {
				const KGpgGroupMemberNode *g = static_cast<const KGpgGroupMemberNode *>(right);

				rsign = g->getSignKeySize();
				renc = g->getEncryptionKeySize();
			} else {
				const KGpgKeyNode *g = static_cast<const KGpgKeyNode *>(right);

				rsign = g->getSignKeySize();
				renc = g->getEncryptionKeySize();
			}

			if (lsign != rsign)
				return lsign < rsign;
			else
				return lenc < renc;
		} else {
			return (left->getSize() < right->getSize());
		}
	case KEYCOLUMN_CREAT:
		return (left->getCreation() < right->getCreation());
	default:
		Q_ASSERT(column == KEYCOLUMN_ID);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		return (left->getId().rightRef(m_idLength).compare(right->getId().rightRef(m_idLength)) < 0);
#else
		return (QStringView(left->getId()).right(m_idLength).compare(QStringView(right->getId()).right(m_idLength)) < 0);
#endif
	}
}

bool
KeyListProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
	Q_D(const KeyListProxyModel);
	QModelIndex idx = d->m_model->index(source_row, 0, source_parent);
	const KGpgNode *l = d->m_model->nodeForIndex(idx);

	if (l == d->m_model->getRootNode())
		return false;

	if (d->m_onlysecret) {
		switch (l->getType()) {
		case ITYPE_PUBLIC:
		case ITYPE_GPUBLIC:
		case ITYPE_GROUP:
			return false;
		default:
			break;
		}
	}

	switch (d->m_displaymode) {
	case SingleColumnIdFirst:
	case SingleColumnIdLast:
		if (l->getType() == ITYPE_GROUP)
			return false;
	default:
		break;
	}

	if (l->getTrust() < d->m_mintrust)
		return false;

	/* check for expired signatures */
	if ((d->m_mintrust > TRUST_EXPIRED) && (l->getType() == ITYPE_SIGN)) {
		const QDateTime expDate = l->toSignNode()->getExpiration();
		if (expDate.isValid() && (expDate < QDateTime::currentDateTime()))
			return false;
	}

	if (l->getParentKeyNode() != d->m_model->getRootNode())
		return true;

	if (d->m_encryptionKeys && ((l->getType() & ITYPE_GROUP) == 0)) {
		if (!l->toKeyNode()->canEncrypt())
			return false;
	}

	if (l->getName().contains(filterRegExp()))
		return true;

	if (l->getEmail().contains(filterRegExp()))
		return true;

	if (l->getId().contains(filterRegExp()))
		return true;

	return false;
}

void
KeyListProxyModel::setOnlySecret(const bool b)
{
	Q_D(KeyListProxyModel);

	d->m_onlysecret = b;
	invalidateFilter();
}

void
KeyListProxyModel::settingsChanged()
{
	Q_D(KeyListProxyModel);

	const int newSort = KGpgSettings::emailSorting();

	if (newSort != d->m_emailSorting) {
		d->m_emailSorting = newSort;
		invalidate();
	}
}

void
KeyListProxyModel::setTrustFilter(const KgpgCore::KgpgKeyTrustFlag t)
{
	Q_D(KeyListProxyModel);

	d->m_mintrust = t;
	invalidateFilter();
}

void
KeyListProxyModel::setEncryptionKeyFilter(bool b)
{
	Q_D(KeyListProxyModel);

	d->m_encryptionKeys = b;
	invalidateFilter();
}

KGpgNode *
KeyListProxyModel::nodeForIndex(const QModelIndex &index) const
{
	Q_D(const KeyListProxyModel);

	return d->m_model->nodeForIndex(mapToSource(index));
}

QModelIndex
KeyListProxyModel::nodeIndex(KGpgNode *node)
{
	Q_D(KeyListProxyModel);

	return mapFromSource(d->m_model->nodeIndex(node));
}

void
KeyListProxyModel::setPreviewSize(const int pixel)
{
	Q_D(KeyListProxyModel);

	Q_EMIT layoutAboutToBeChanged();
	d->m_previewsize = pixel;
	Q_EMIT layoutChanged();
}

QVariant
KeyListProxyModel::data(const QModelIndex &index, int role) const
{
	Q_D(const KeyListProxyModel);

	if (!index.isValid())
		return QVariant();

	const KGpgNode *node = nodeForIndex(index);

	switch (d->m_displaymode) {
	case MultiColumn:
		return d->dataMultiColumn(index, role, node);
	case SingleColumnIdFirst:
	case SingleColumnIdLast:
		return d->dataSingleColumn(index, role, node);
	}

	Q_ASSERT(0);

	return QVariant();
}

KGpgItemModel *
KeyListProxyModel::getModel() const
{
	Q_D(const KeyListProxyModel);

	return d->m_model;
}

int
KeyListProxyModel::idLength() const
{
	Q_D(const KeyListProxyModel);

	return d->m_idLength;
}

void
KeyListProxyModel::setIdLength(const int length)
{
	Q_D(KeyListProxyModel);

	if (length == d->m_idLength)
		return;

	d->m_idLength = length;
	invalidate();
}

bool
KeyListProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	Q_UNUSED(role);

	if (value.type() != QVariant::String)
		return false;

	KGpgNode *node = nodeForIndex(index);

	if (!node)
		return false;

	const QString newName = value.toString();

	if (newName.isEmpty() || (newName == node->getName()))
		return false;

	node->toGroupNode()->rename(newName);

	return true;
}

Qt::ItemFlags
KeyListProxyModel::flags(const QModelIndex &index) const
{
	KGpgNode *node = nodeForIndex(index);
	Qt::ItemFlags flags = QSortFilterProxyModel::flags(index);

	if ((node->getType() == ITYPE_GROUP) && (index.column() == KEYCOLUMN_NAME))
		flags |= Qt::ItemIsEditable;

	return flags;
}
