/*
    SPDX-FileCopyrightText: 2008, 2013 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef KEYLISTPROXYMODEL_H
#define KEYLISTPROXYMODEL_H

#include <QSortFilterProxyModel>
#include "core/kgpgkey.h"

class KGpgNode;
class KGpgItemModel;
class KeyListProxyModelPrivate;

class KeyListProxyModel: public QSortFilterProxyModel
{
	Q_OBJECT

	Q_PROPERTY(int idLength READ idLength WRITE setIdLength)
	Q_DECLARE_PRIVATE(KeyListProxyModel)

	KeyListProxyModelPrivate * const d_ptr;

public:
	enum DisplayMode {
		MultiColumn = 0,
		SingleColumnIdFirst = 1,
		SingleColumnIdLast = 2
	};

    explicit KeyListProxyModel(QObject * parent = nullptr, const DisplayMode mode = MultiColumn);
    ~KeyListProxyModel() override;

    bool hasChildren(const QModelIndex &idx) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
	void setKeyModel(KGpgItemModel *);
	/**
	 * @brief set the minimum trust level to be shown
	 * @param t trust level
	 *
	 * This enables filtering by key trust. All keys that have a lower trust than
	 * the given value will be hidden.
	 */
	void setTrustFilter(const KgpgCore::KgpgKeyTrustFlag t);

	/**
	 * @brief show only keys capable of encryption
	 */
	void setEncryptionKeyFilter(bool b);

	KGpgNode *nodeForIndex(const QModelIndex &index) const;
	QModelIndex nodeIndex(KGpgNode *node);
	void setPreviewSize(const int pixel);
	KGpgItemModel *getModel() const;
	int idLength() const;
	void setIdLength(const int length);

public Q_SLOTS:
	/**
	 * @brief set if only secret keys should be shown
	 * @param b new value
	 */
	void setOnlySecret(const bool b);

	/**
	 * @brief call this when the settings have changed
	 */
	void settingsChanged();

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

#endif
