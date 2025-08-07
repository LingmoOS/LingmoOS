/*
    SPDX-FileCopyrightText: 2009, 2010 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-FileCopyrightText: 2016 David Zaslavsky <diazona@ellipsix.net>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef KGPGSEARCHRESULTMODEL_H
#define KGPGSEARCHRESULTMODEL_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QStringList>

class SearchResult;
class KGpgSearchResultModelPrivate;

/**
 * @brief A model to store the results of a keyserver search.
 *
 * This model parses and stores the results of a search on a keyserver.
 * It is never used directly, only by `KGpgSearchResultModel`.
 *
 * @author Rolf Eike Beer
 * @author David Zaslavsky
 */
class KGpgSearchResultBackingModel : public QAbstractItemModel {
	// The moc complains if I put this class definition in the .cpp file
	Q_OBJECT
public:
	explicit KGpgSearchResultBackingModel(QObject *parent = nullptr);
    ~KGpgSearchResultBackingModel() override;

	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex &index) const override;
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	/**
	 * @brief get the key fingerprint for the given index
	 * @param index valid index of any item in the model
	 * @return fingerprint of the corresponding key
	 */
	QString idForIndex(const QModelIndex &index) const;

	typedef enum {ROOT_LEVEL, KEY_LEVEL, ATTRIBUTE_LEVEL} NodeLevel;

	/**
	 * @brief Returns the level corresponding to a `QModelIndex` associated
	 * with this model.
	 *
	 * There are three levels of nodes. The top level, level 0, is the
	 * root node. Each first-level subnode corresponds to a key, and each
	 * second-level subnode corresponds to an attribute of that key: a UID,
	 * or the number of UATs, or the summary description of the key.
	 *
	 * @param index a `QModelIndex` representing the position of a node in the model
	 * @return the level of the node
	 */
	static NodeLevel nodeLevel(const QModelIndex &index);

	/**
	 * @brief Find the `SearchResult` associated with an index.
	 *
	 * This returns the `SearchResult` instance corresponding to an
	 * index regardless of whether the index represents a key (first-level)
	 * or an attribute (second-level). It also works for the root index,
	 * for which it returns `nullptr`.
	 */
	SearchResult *resultForIndex(const QModelIndex &index) const;

public Q_SLOTS:
	void slotAddKey(const QStringList &lines);

private:
	KGpgSearchResultModelPrivate * const d;
};

/**
 * @brief A model to parse, store, and display the results of
 * a keyserver search.
 *
 * This model manages the results returned by a keyserver search.
 * It is a proxy model, backed by a source model which parses
 * and stores the list of keys yielded by the search. The proxy
 * model exposes a sorted and/or filtered version of that list
 * to the view. On top of the sorting and regexp-based filtering
 * allowed by a basic `QSortFilterProxyModel`, this adds the
 * ability to filter out invalid keys.
 *
 * Unlike a generic `QSortFilterProxyModel`, this manages its
 * own source model internally. Don't set the source model with
 * `setSourceModel()` yourself.
 *
 * @author David Zaslavsky
 */
class KGpgSearchResultModel : public QSortFilterProxyModel {
	Q_OBJECT
public:
	explicit KGpgSearchResultModel(QObject *parent = nullptr);
    ~KGpgSearchResultModel() override;

	bool filterByValidity() const;
	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

	/**
	 * @brief get the key fingerprint for the given index
	 * @param index valid index of any item in the model
	 * @return fingerprint of the corresponding key
	 */
	QString idForIndex(const QModelIndex &index) const;

	/**
	 * @brief Return the total number of rows in the source model.
	 */
	int sourceRowCount(const QModelIndex &parent = QModelIndex()) const;

	/**
	 * Don't use this. The filter model manages its own source
	 * internally. Use `resetSourceModel()` if you want to clear the
	 * source model, and `slotAddKey()` to populate it.
	 */
	void setSourceModel(QAbstractItemModel *sourceModel) override;

public Q_SLOTS:
	/**
	 * @brief Control whether validity filtering of keys is enabled.
	 *
	 * @param filter `true` to hide expired/revoked keys, `false` to show them
	 */
	void setFilterByValidity(bool filter);
	/**
	 * @brief Adds a key to the underlying source model.
	 */
	void slotAddKey(const QStringList &lines);
	/**
	 * @brief Resets the source model to be empty.
	 */
	void resetSourceModel();

private:
	bool m_filterByValidity;
};

#endif
