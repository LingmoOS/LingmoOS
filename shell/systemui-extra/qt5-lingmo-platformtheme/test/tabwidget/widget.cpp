/*
 * Qt5-LINGMO
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "widget.h"
#include <QPushButton>

#include <QListView>
#include <QFileSystemModel>
#include <QTreeView>
#include <QLabel>

#include <QHBoxLayout>

Widget::Widget(QWidget *parent)
    : QTabWidget(parent)
{
    auto v1 = new QListView(this);
    v1->setViewMode(QListView::IconMode);
    auto m1 = new QFileSystemModel(v1);
    v1->setModel(m1);
    m1->setRootPath("/");
    v1->setRootIndex(m1->index("/"));
    addTab(v1, "view1");

    auto v2 = new QTreeView;
    v2->setModel(m1);
    v2->setRootIndex(m1->index("/"));
    addTab(v2, "view2");

    addTab(new QPushButton("test1", this), "test1");
    addTab(new QLabel("test2", this), "test2");
    addTab(new QPushButton("test3", this), "test3");
    addTab(new QLabel("test4", this), "test4");
}

Widget::~Widget()
{

}
