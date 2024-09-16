// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file tableView.cpp
 *
 * @brief 下载条目列表
 *
 * @date 2020-06-09 09:56
 *
 * Author: zhaoyue  <zhaoyue@uniontech.com>
 *
 * Maintainer: zhaoyue  <zhaoyue@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "tableView.h"

#include <QDebug>
#include <QScrollBar>
#include <QMouseEvent>
#include <QHeaderView>
#include <QModelIndex>
#include <QJsonObject>
#include <QDateTime>
#include <QProcess>
#include <QThread>
#include <QDesktopServices>
#include <DApplicationHelper>
#include <QStandardItemModel>

#include "../database/dbinstance.h"
#include "global.h"
#include "../aria2/aria2rpcinterface.h"
#include "tableModel.h"
#include "headerView.h"
#include "itemDelegate.h"
#include "settings.h"
#include "topButton.h"
#include "tabledatacontrol.h"
#include "global.h"

using namespace Global;

TableView::TableView(int Flag)
    : QTableView()
    , m_TableFlag(Flag)
    , m_TableModel(new TableModel(Flag))
    , m_TableDataControl(new TableDataControl(this))
    , m_Itemdegegate(new ItemDelegate(this, m_TableFlag))
    , m_Setting(Settings::getInstance())
{
    initUI();
    initConnections();
}

TableView::~TableView()
{
    delete (m_TableModel);
    delete (m_TableDataControl);
    delete (m_HeaderView);
}

void TableView::initUI()
{
    setModel(m_TableModel);
    setItemDelegate(m_Itemdegegate);
    setFrameShape(QFrame::NoFrame);
    setMinimumWidth(636);
    setMouseTracking(true);
    setSortingEnabled(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    verticalHeader()->hide();
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QAbstractItemView::SelectedClicked);

    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    setAlternatingRowColors(true);
    setShowGrid(false);

    setSelectionMode(QAbstractItemView::SingleSelection);

    m_HeaderView = new DownloadHeaderView(Qt::Horizontal, this);
    setHorizontalHeader(m_HeaderView);
    m_HeaderView->setStretchLastSection(true);
    m_HeaderView->setDefaultAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    setColumnWidth(0, 30);
    setColumnWidth(1, 248);
    m_HeaderView->setSectionResizeMode(1, QHeaderView::Interactive);
    m_HeaderView->setSectionResizeMode(0, QHeaderView::Fixed);
    setColumnWidth(2, 110);
    setColumnWidth(3, QHeaderView::Interactive);
    setColumnWidth(4, QHeaderView::Interactive);
    setTabKeyNavigation(true);
    QFont font;
    font.setFamily("Source Han Sans");
    setFont(font);
}

void TableView::initConnections()
{
    connect(m_HeaderView, &DownloadHeaderView::Statechanged, this, &TableView::HeaderStatechanged);
    //connect(this, &TableView::ClearHeaderCheck, m_HeaderView, &HeaderView::onClearHeaderChecked);
    connect(m_TableModel, &TableModel::tableviewAllcheckedOrAllunchecked, this, &TableView::isCheckHeader);
    connect(this, &TableView::isCheckHeader, m_HeaderView, &DownloadHeaderView::onHeaderChecked);
    connect(this, &TableView::Hoverchanged, m_Itemdegegate, &ItemDelegate::onHoverchanged);
    connect(m_TableModel, &TableModel::layoutChanged, this, &TableView::onModellayoutChanged);
}

void TableView::onListchanged()
{
    currentChanged(m_PreviousIndex.sibling(m_PreviousIndex.row(), 0), m_PreviousIndex);
}

void TableView::reset(bool switched)
{
    int size = QTableView::verticalScrollBar()->value();

    QTableView::reset();

    //selectRow(idx.row());
    if (switched) {
        size = 0;
    }
    QTableView::verticalScrollBar()->setValue(size);
}

TableModel *TableView::getTableModel()
{
    return m_TableModel;
}

TableDataControl *TableView::getTableControl()
{
    return m_TableDataControl;
}

DownloadHeaderView *TableView::getTableHeader()
{
    return m_HeaderView;
}

void TableView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        //setCurrentIndex(QModelIndex());
        QTableView::mousePressEvent(event);
        QModelIndex index = indexAt(event->pos());
        this->reset();
        if ((index.row() < 0) && (index.column() < 0)) {
            currentChanged(m_PreviousIndex.sibling(m_PreviousIndex.row(), 0), m_PreviousIndex);
            //return;
        } else {
            m_PreviousIndex = index;
        }
    }
}

void TableView::mouseMoveEvent(QMouseEvent *event)
{
    QModelIndex idx = indexAt(event->pos());
    emit Hoverchanged(idx);
}

void TableView::mouseReleaseEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());

    if ((index.row() < 0) && (index.column() < 0)) {
        emit HeaderStatechanged(false);
        emit isCheckHeader(false);
        //return;
    }
    //reset();
    QTableView::mouseReleaseEvent(event);
}

void TableView::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    //reset();
    // currentChanged(m_PreviousIndex.sibling(m_PreviousIndex.row(), 0), m_PreviousIndex);
    emit Hoverchanged(QModelIndex());
}

void TableView::keyPressEvent(QKeyEvent *event)
{
    if ((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_C)) {
        return;
    }
    //    if(event->key() == Qt::Key_Down) {
    //        currentChanged(m_PreviousIndex.sibling(m_PreviousIndex.row() + 1, 0), m_PreviousIndex);
    //        setCurrentIndex(m_PreviousIndex.sibling(m_PreviousIndex.row() + 1, 0));
    //    }
    //    if(event->key() == Qt::Key_Up) {
    //        currentChanged(m_PreviousIndex.sibling(m_PreviousIndex.row() - 1, 0), m_PreviousIndex);
    //        setCurrentIndex(m_PreviousIndex.sibling(m_PreviousIndex.row() - 1, 0));
    //    }
    QWidget::keyPressEvent(event);
}

void TableView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QTableView::currentChanged(current, previous);
}

bool TableView::refreshTableView(const int &index)
{
    if (index > 1) {
        return false;
    }
    switch (index) {
    case 0: {
        //if(1 == getTableModel()->getTablemodelMode()){
        getTableModel()->switchDownloadingMode();
        //}
        setColumnHidden(3, false);
        setColumnHidden(4, true);
        break;
    }

    case 1:
        //if(0 == getTableModel()->getTablemodelMode()){
        getTableModel()->switchFinishedMode();
        //}
        setColumnHidden(3, true);
        setColumnHidden(4, false);
        //getTableHeader()->setSortIndicator(4, Qt::AscendingOrder);
        break;
    }
    update();
    return true;
}

void TableView::onModellayoutChanged()
{
    if (m_TableFlag == 0) {
        const QList<DownloadDataItem *> &selectList = getTableModel()->renderList();
        for (int i = 0; i < selectList.size(); i++) {
            if (selectList.at(i)->isHide) {
                setRowHidden(i, true);
            } else {
                setRowHidden(i, false);
            }
        }
    } else {
        const QList<DeleteDataItem *> &selectList = getTableModel()->recyleList();
        for (int i = 0; i < selectList.size(); i++) {
            if (selectList.at(i)->isHide) {
                setRowHidden(i, true);
            } else {
                setRowHidden(i, false);
            }
        }
    }
}

LeftListView::LeftListView()
{
}

void LeftListView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    emit currentIndexChanged(current);
}

void LeftListView::paintEvent(QPaintEvent *e)
{
    DPalette pa;
    pa = DApplicationHelper::instance()->palette(this);
    pa.setBrush(DPalette::ItemBackground, pa.brush(DPalette::Base));
    DApplicationHelper::instance()->setPalette(this, pa);
    DListView::paintEvent(e);
}
