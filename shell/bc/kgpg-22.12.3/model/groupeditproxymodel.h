/*
    SPDX-FileCopyrightText: 2008, 2014 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef GROUPEDITPROXYMODEL_H
#define GROUPEDITPROXYMODEL_H

#include "core/kgpgkey.h"

#include <QSortFilterProxyModel>

class KGpgNode;
class KGpgItemModel;

class GroupEditProxyModel: public QSortFilterProxyModel
{
public:
	explicit GroupEditProxyModel(QObject * parent, const bool invert, QList<KGpgNode *> *ids, const KgpgCore::KgpgKeyTrust mintrust = KgpgCore::TRUST_FULL);

	void setKeyModel(KGpgItemModel *);

	KGpgNode *nodeForIndex(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool hasChildren(const QModelIndex &parent) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    int columnCount(const QModelIndex &) const override;

private:
	KGpgItemModel *m_model;
	bool m_invert;
	QList<KGpgNode *> *m_ids;
	KgpgCore::KgpgKeyTrust m_mintrust;
};

#endif
