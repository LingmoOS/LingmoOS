/*
    SPDX-FileCopyrightText: 2008 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef KEYTREEVIEW_H
#define KEYTREEVIEW_H

#include <QTreeView>

#include <QUrl>
#include <vector>

#include "core/kgpgkey.h"

class QDragMoveEvent;
class QDropEvent;

class KGpgNode;
class KeyListProxyModel;
class KConfigGroup;

class KeyTreeView: public QTreeView
{
	Q_OBJECT
private:
	KeyListProxyModel *m_proxy;

public:
	explicit KeyTreeView(QWidget *parent = nullptr, KeyListProxyModel *model = nullptr);

	std::vector<KGpgNode *> selectedNodes(bool *psame = nullptr, KgpgCore::KgpgItemType *pt = nullptr) const;
	KGpgNode *selectedNode() const;

	void restoreLayout(KConfigGroup &cg);
	void saveLayout(KConfigGroup &cg) const;

	bool isEditing() const;

Q_SIGNALS:
	void importDrop(const QList<QUrl> &urls);
	void returnPressed();

public Q_SLOTS:
	void selectNode(KGpgNode *nd);
	void resizeColumnsToContents();

protected:
        virtual void contentsDragMoveEvent(QDragMoveEvent *e);
        virtual void contentsDropEvent(QDropEvent *e);
        void startDrag(Qt::DropActions) override;
        void keyPressEvent(QKeyEvent *event) override;
};

#endif
