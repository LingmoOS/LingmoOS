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
#include <QApplication>
#include <file-item-proxy-filter-sort-model.h>
#include <file-item-model.h>
#include <path-bar-model.h>
#include <path-completer.h>
#include <file-item.h>

#include <QListView>
#include <QLineEdit>
#include <QToolBar>
#include <QLabel>

#include <QStandardPaths>
#include <QRegExp>
#include <QDebug>

#include <QThread>

#include <search-vfs-register.h>

#include "side-bar-model.h"
#include "side-bar-proxy-filter-sort-model.h"
#include "side-bar-abstract-item.h"
#include <QTreeView>
#include <QHeaderView>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QToolBar t;
    QLineEdit e;
    Peony::PathBarModel pm;
    Peony::PathCompleter c;
    c.setModel(&pm);
    e.setCompleter(&c);

    w.addToolBar(Qt::TopToolBarArea, &t);
    t.addWidget(&e);

    QLabel l;
    l.setWordWrap(true);
    l.setMaximumWidth(480);
    l.setText("The file item model has provided the drag and drop interface."
              "you can drag the file item(s) to a folder, and the files will be "
              "moved into this folder. If you are interested about it and want "
              "to know how it works, you should see the source code of FileItemModel, "
              "FileOperationManager and FileMoveOperation."
              "\n\n"
              "The QListView is only accept items drag and drop in same class instances by default. "
              "If you want to drag and drop in different kinds of views, you should "
              "dirved the view class and override it drag and drop event handler for non-limit "
              "drag and drop event acception. Here I don't re-implement those handler.");

    w.setCentralWidget(&l);

    e.setText("network:///");
    //e.setText("file://" + QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    e.connect(&e, &QLineEdit::returnPressed, [&]() {
        QListView *v = new QListView;

        /// set view attribute
        v->setAttribute(Qt::WA_DeleteOnClose);
        v->setWordWrap(true);
        v->setViewMode(QListView::IconMode);
        v->setGridSize(QSize(96, 96));
        v->setResizeMode(QListView::Adjust);

        /// set select action
        v->setSelectionMode(QAbstractItemView::ExtendedSelection);

        /// set dnd actions
        v->setDragDropMode(QAbstractItemView::DragDrop);
        v->setDefaultDropAction(Qt::MoveAction);

        Peony::FileItemModel *model = new Peony::FileItemModel(v);
        model->setRootUri(e.text());
        Peony::FileItemProxyFilterSortModel *proxy_model = new Peony::FileItemProxyFilterSortModel(model);
        proxy_model->setSourceModel(model);

        /// resort when load directory finished
        model->connect(model, &Peony::FileItemModel::findChildrenFinished, [=]() {
            proxy_model->sort(0);
        });

        /// re-sort when file(s) moved in/out.
        model->connect(model, &Peony::FileItemModel::updated, [=]() {
            proxy_model->sort(0);
        });

        /// double clicked for location change.
        v->setEditTriggers(QListView::EditKeyPressed);
        v->connect(v, &QListView::doubleClicked, [=](const QModelIndex &index) {
            auto item = proxy_model->itemFromIndex(index);
            if (item->hasChildren() || item->uri().startsWith("network:")) {
                model->setRootUri(item->uri());
            }
        });

        v->setModel(proxy_model);
        v->show();
    });

    w.show();

    return a.exec();
}
