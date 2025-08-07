/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-FileCopyrightText: 2007, 2008, 2013 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GROUPEDIT_H
#define GROUPEDIT_H

#include <QList>
#include "ui_groupedit.h"

class GroupEditProxyModel;
class KGpgNode;
class KGpgItemModel;
class QSortFilterProxyModel;

/**
 * @brief shows a widget that let's you change the keys that are part of a key group
 */
class groupEdit : public QWidget, public Ui::groupEdit
{
private:
	GroupEditProxyModel *m_in;
	GroupEditProxyModel *m_out;
	QSortFilterProxyModel * const m_outFilter;

public:
	QList<KGpgNode *> * const members;	///< the list of keys that are members of the group

	/**
	 * @brief constructor
	 * @param parent parent widget
	 * @param ids the members of the group
	 * @param md model to use
	 */
	explicit groupEdit(QWidget *parent, QList<KGpgNode *> *ids, KGpgItemModel *md);
	/**
	 * @brief destructor
	 */
	~groupEdit() override;

private:
	/**
	 * @brief called when the add button is clicked
	 */
	void groupAdd();
	/**
	 * @brief called when the remove button is clicked
	 */
	void groupRemove();
	/**
	 * @brief called when an available key is double clicked
	 */
	void groupAdd(const QModelIndex &index);
	/**
	 * @brief clicked when a group member key is double clicked
	 */
	void groupRemove(const QModelIndex &index);
};

#endif
