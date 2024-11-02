/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 * Authors: Meihong He <hemeihong@kylinos.cn>
 *
 */

#include "search-bar.h"
#include <QCompleter>
#include <QAction>
#include <QIcon>
#include <QWidget>
#include <QHeaderView>
#include <QVector4D>

#include <QDebug>

using namespace Peony;

SearchBar::SearchBar(FMWindow *window, QWidget *parent) : QLineEdit(parent)
{
    m_top_window = window;
    init(window? true: false);
}

void SearchBar::init(bool hasTopWindow)
{
    setTextMargins(5, 0, 0, 0);
    setFixedWidth(175);

    setToolTip(tr("Input the search key of files you would like to find."));

    m_model = new QStandardItemModel(this);
    QCompleter *completer = new QCompleter(this);
    completer->setModel(m_model);
    completer->setMaxVisibleItems(10);

    //add two button in the completer, use QTableView
    m_table_view= new QTableView(this);

    m_table_view->setAttribute(Qt::WA_TranslucentBackground);
    //m_table_view->viewport()->setAttribute(Qt::WA_TranslucentBackground);

    m_table_view->setProperty("useCustomShadow", true);
    m_table_view->setProperty("customShadowDarkness", 0.5);
    m_table_view->setProperty("customShadowWidth", 20);
    m_table_view->setProperty("customShadowRadius", QVector4D(6, 6, 6, 6));
    m_table_view->setProperty("customShadowMargins", QVector4D(20, 20, 20, 20));

    m_table_view->setShowGrid(false);
    m_table_view->horizontalHeader()->setDefaultSectionSize(120);
    m_table_view->verticalHeader()->setDefaultSectionSize(6);
    m_table_view->horizontalHeader()->setVisible(false);
    m_table_view->verticalHeader()->setVisible(false);
    m_table_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_table_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //m_table_view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    initTableModel();

    completer->setPopup(m_table_view);
    completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    this->setCompleter(completer);
    //end completer

    setLayoutDirection(Qt::LeftToRight);
    setPlaceholderText(tr("Input search key..."));
    QAction *searchAction = addAction(QIcon::fromTheme("edit-find-symbolic"), QLineEdit::TrailingPosition);
    //NOTE: we should not add a short cut for line edit,
    //because it might have some bad effect for other controls.
    //use returnPressed signal trigger the action instead.
    //searchAction->setShortcut(Qt::Key_Return);
    connect(this, &QLineEdit::returnPressed, searchAction, &QAction::trigger);
    connect(this, &QLineEdit::textChanged, this, &SearchBar::searchKeyChanged);
    connect(m_table_view, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onTableClicked(const QModelIndex &)));
    connect(searchAction, &QAction::triggered, this, &SearchBar::updateTableModel);
}

void SearchBar::focusInEvent(QFocusEvent *e)
{
    blockSignals(false);
    QLineEdit::focusInEvent(e);
    this->completer()->complete();
}

void SearchBar::focusOutEvent(QFocusEvent *e)
{
    blockSignals(true);
    QLineEdit::focusOutEvent(e);
    //this->clear();
}

void SearchBar::clearSearchRecord()
{
    m_model->clear();
    initTableModel();

    QTimer::singleShot(100, this, [=]() {
        m_table_view->setVisible(false);
    });
}

void SearchBar::initTableModel()
{
    QStandardItem *advance = new QStandardItem(tr("advance search"));
    QStandardItem *clear = new QStandardItem(tr("clear record"));
    QList<QStandardItem*> firstRow;
    firstRow<<advance<<clear;
    m_model->insertRow(0,firstRow);
    m_model->item(0)->setForeground(QBrush(QColor(10,10,255)));
    m_table_view->setMinimumHeight(25);
    m_table_view->setVisible(true);
}

void SearchBar::updateTableModel()
{
    //comment unused code
//    if (!this->text().isEmpty()) {
//        bool contained = false;
//        for(int i=0; i<m_model->rowCount(); i++)
//        {
//            if(m_model->item(i)->text() == this->text())
//            {
//                contained = true;
//                break;
//            }
//        }
//        if (! contained)
//        {
//            QStandardItem *key = new QStandardItem(this->text());
//            QList<QStandardItem*> row;
//            m_model->insertRow(0, row<<key);
//        }

//        m_table_view->setMinimumHeight(m_model->rowCount() * 25);
//        Q_EMIT this->searchRequest(this->text());
//        this->clear();
//        this->clearFocus();
//    }
}

void SearchBar::onTableClicked(const QModelIndex &index)
{
    qDebug()<<"onTableClicked"<<index.row()<<m_model->rowCount()<<index.column()<<this->text();
    m_table_view->clearSelection();
    if (index.row() != m_model->rowCount()-1)
    {
        this->setText(m_model->item(index.row())->text());
        return;
    }
    if (index.column() == 0)
    {
        //clicked advance search
        m_top_window->advanceSearch();
        //qDebug()<<"show or hidden advance filter";
    }
    else if(index.column() == 1)
    {
        //clicked clear record
        clearSearchRecord();
    }
    this->setText("");
}

void SearchBar::hideTableView()
{
    m_table_view->setVisible(false);
}
