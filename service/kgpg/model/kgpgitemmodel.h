/*
    SPDX-FileCopyrightText: 2008, 2009, 2010, 2011, 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef KGPGITEMMODEL_H
#define KGPGITEMMODEL_H

#include "core/kgpgkey.h"
#include "core/KGpgKeyNode.h"
#include "core/KGpgNode.h"

#include <QAbstractItemModel>
#include <QString>
#include <QStringList>

#define KEYCOLUMN_NAME	0
#define KEYCOLUMN_EMAIL	1
#define KEYCOLUMN_TRUST	2
#define KEYCOLUMN_EXPIR	3
#define KEYCOLUMN_SIZE	4
#define KEYCOLUMN_CREAT	5
#define KEYCOLUMN_ID	6

class KGpgExpandableNode;
class KGpgGroupNode;
class KGpgGroupMemberNode;
class KGpgRootNode;
class QColor;

class KGpgItemModel : public QAbstractItemModel
{
	Q_OBJECT

private:
	KGpgRootNode *m_root;
	QString m_default;

public:

    explicit KGpgItemModel(QObject *parent = nullptr);
    ~KGpgItemModel() override;

    QModelIndex index(int row, int column,
                const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex & /*parent = QModelIndex()*/ ) const override
			{ return 7; }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool hasChildren(const QModelIndex &parent) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	KGpgNode *nodeForIndex(const QModelIndex &index) const;
	KGpgKeyNode *findKeyNode(const QString &id) const;

	KGpgRootNode *getRootNode() const;
	QString statusCountMessage() const;
	static QString statusCountMessageString(const unsigned int keys, const unsigned int groups);
	bool isDefaultKey(const KGpgNode *node) const;

public Q_SLOTS:
	KGpgGroupNode *addGroup(const QString &name, const KGpgKeyNode::List &keys);
	void delNode(KGpgNode *node);
	void changeGroup(KGpgGroupNode *node, const KGpgNode::List &keys);
	void deleteFromGroup(KGpgGroupNode *group, KGpgGroupMemberNode *member);
	void setDefaultKey(KGpgKeyNode *def);
	QModelIndex nodeIndex(KGpgNode *node, const int column = 0);
	inline void refreshKey(const QString &id)
	{ refreshKeys(QStringList(id)); }
	inline void refreshKey(KGpgKeyNode *nd)
	{ refreshKeys(KGpgKeyNode::List() << nd); }
	void refreshKeys(const QStringList &ids);
	void refreshKeys(KGpgKeyNode::List keys);
	void refreshAllKeys();
	void refreshGroups();
	void invalidateIndexes(KGpgNode *nd);
	void refreshTrust(const KgpgCore::KgpgKeyTrust trust, const QColor &color);

protected:
	int rowForNode(KGpgNode *node) const;
	void updateNodeTrustColor(KGpgExpandableNode *node, const KgpgCore::KgpgKeyTrust trust, const QColor &color);
};

#endif
