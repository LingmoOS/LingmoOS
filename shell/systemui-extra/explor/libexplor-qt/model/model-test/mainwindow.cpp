/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin LINGMO Information Technology Co., Ltd.
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
 *
 */

#include "mainwindow.h"

#include <QLineEdit>
#include <QLayout>
#include <memory>

#include "file-info.h"
#include "file-item.h"
#include "file-item-model.h"
#include "file-item-proxy-filter-sort-model.h"
#include "file-enumerator.h"
#include "file-info-job.h"
#include "gerror-wrapper.h"
#include <QTreeView>
#include <QListView>
#include <QToolBar>

#include <QTimer>

#include <QDebug>
#include <QHeaderView>

#include <QAction>

#include "file-info-manager.h"
#include "file-operation-manager.h"

#define TEST

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
#ifndef TEST
#define TEST
    QLineEdit *line = new QLineEdit("file:///home/lanyue", this);
    QToolBar *toolbar = new QToolBar(this);
    toolbar->addWidget(line);
    addToolBar(Qt::TopToolBarArea, toolbar);

    //job
    /*
    connect(line, &QLineEdit::returnPressed, [=](){
        auto info = Peony::FileInfo::fromUri(line->text());
        Peony::FileInfoJob *job = new Peony::FileInfoJob(info);
        job->setAutoDelete();
        job->connect(job, &Peony::FileInfoJob::queryAsyncFinished, [=](bool successed){
            if (successed) {
                qDebug()<<info->displayName();
            }
        });
        job->queryAsync();
    });
    */

    //enumerator
    /*
    connect(line, &QLineEdit::returnPressed, [=](){
        Peony::FileEnumerator *enumerator = new Peony::FileEnumerator;
        enumerator->setEnumerateDirectory(line->text());
        connect(enumerator, &Peony::FileEnumerator::prepared, [=](std::shared_ptr<Peony::GErrorWrapper> err){
            if (err) {
                qDebug()<<err->message();
                return ;
            }
            connect(enumerator, &Peony::FileEnumerator::enumerateFinished, [=](bool successed){
                if (successed) {
                    auto files = enumerator->getChildren();
                    for (auto file : files) {
                        qDebug()<<file->uri();
                    }
                    delete enumerator;
                }
            });
            enumerator->enumerateAsync();
        });
        enumerator->prepare();
    });
    */

    //model/view

    connect(line, &QLineEdit::returnPressed, [=]() {


        Peony::FileItemModel *model = new Peony::FileItemModel(this);

        Peony::FileItem *item = new Peony::FileItem(Peony::FileInfo::fromUri(line->text().toUtf8().constData()), nullptr, model, this);

        model->setRootItem(item);

        Peony::FileItemProxyFilterSortModel *pm = new Peony::FileItemProxyFilterSortModel;
        pm->setSourceModel(model);

        //icon view, dnd.

        QListView *lv = new QListView;
        lv->setModel(pm);
        lv->setViewMode(QListView::IconMode);
        lv->setDragDropMode(QListView::DragDrop);
        lv->setDragEnabled(true);
        lv->setDefaultDropAction(Qt::MoveAction);
        lv->setAcceptDrops(true);
        lv->setSelectionMode(QAbstractItemView::ExtendedSelection);

        //layout

        lv->setGridSize(QSize(96, 96));
        lv->setIconSize(QSize(48, 48));
        lv->setResizeMode(QListView::Adjust);
        //lv->setUniformItemSizes(true);
        lv->setWordWrap(true);

        //undo redo action
        QAction *undoAction = new QAction(QIcon::fromTheme("undo"), tr("undo"), lv);
        undoAction->setShortcut(QKeySequence::Undo);
        QAction *redoAction = new QAction(QIcon::fromTheme("redo"), tr("redo"), lv);
        redoAction->setShortcut(QKeySequence::Redo);
        connect(undoAction, &QAction::triggered, [=]() {
            auto fileOpMgr = Peony::FileOperationManager::getInstance();
            fileOpMgr->undo();
        });
        connect(redoAction, &QAction::triggered, [=]() {
            auto fileOpMgr = Peony::FileOperationManager::getInstance();
            fileOpMgr->redo();
        });

        lv->addAction(undoAction);
        lv->addAction(redoAction);

        connect(lv, &QListView::doubleClicked, [=](const QModelIndex &index) {
            lv->setWindowTitle(index.data().toString());
            model->setRootIndex(index);
            //pm->update();
        });

        connect(model, &Peony::FileItemModel::findChildrenFinished, [=]() {
            pm->sort(0);
        });
        lv->show();


        //tree view, expandable
        /*
                QTreeView *v = new QTreeView();
                v->setAttribute(Qt::WA_DeleteOnClose);
                v->setModel(model);
                model->setParent(v);
                v->show();
                connect(v, &QTreeView::expanded, [=](const QModelIndex &index){
                    auto item = model->itemFromIndex(index);
                    item->findChildrenAsync();
                });
                connect(v, &QTreeView::collapsed, [=](const QModelIndex &index){
                    auto item = model->itemFromIndex(index);
                    item->clearChildren();
                });

                QTreeView *pv = new QTreeView;
                Peony::FileItemProxyFilterSortModel *proxy_model = new Peony::FileItemProxyFilterSortModel(pv);
                proxy_model->setSourceModel(model);
                pv->setAttribute(Qt::WA_DeleteOnClose);
                pv->setSortingEnabled(true);
                pv->setModel(proxy_model);
                connect(pv, &QTreeView::expanded, [=](const QModelIndex &proxyIndex){
                    auto item = proxy_model->itemFromIndex(proxyIndex);
                    item->findChildrenAsync();
                });
                connect(pv, &QTreeView::collapsed, [=](const QModelIndex &proxyIndex){
                    auto index = proxy_model->getSourceIndex(proxyIndex);
                    auto item = model->itemFromIndex(index);
                    item->clearChildren();
                });

                pv->show();
                pv->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);

                connect(model, &Peony::FileItemModel::findChildrenStarted, [pv](){
                    QCursor c;
                    c.setShape(Qt::WaitCursor);
                    pv->setCursor(c);
                });
                connect(model, &Peony::FileItemModel::findChildrenFinished, [=](){
                    QCursor c;
                    c.setShape(Qt::ArrowCursor);
                    pv->setCursor(c);
                    pv->sortByColumn(0, Qt::AscendingOrder);
                });
        */

    });
#endif
}

MainWindow::~MainWindow()
{

}
