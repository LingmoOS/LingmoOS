/*
    SPDX-FileCopyrightText: 2008 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef SELECTKEYPROXYMODEL_H
#define SELECTKEYPROXYMODEL_H

#include <QSortFilterProxyModel>

class KGpgNode;
class KGpgItemModel;

/**
 * @brief filter model to select a public key for encryption
 */
class SelectKeyProxyModel: public QSortFilterProxyModel
{
	Q_PROPERTY(bool showUntrusted read showUntrusted write setShowUntrusted)

public:
	explicit SelectKeyProxyModel(QObject * parent);

	void setKeyModel(KGpgItemModel *);

	KGpgNode *nodeForIndex(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool hasChildren(const QModelIndex &parent) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	bool showUntrusted() const;
	void setShowUntrusted(const bool b);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    int columnCount(const QModelIndex &) const override;

	KGpgItemModel *m_model;

private:
	bool m_showUntrusted;
};

class SelectSecretKeyProxyModel: public SelectKeyProxyModel
{
public:
	explicit SelectSecretKeyProxyModel(QObject *parent);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool hasChildren(const QModelIndex &parent) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    int columnCount(const QModelIndex &) const override;
};

#endif
