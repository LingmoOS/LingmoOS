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
 *
 */

#include "intel-side-bar-abstract-item.h"
#include "intel-side-bar-model.h"
#include <QDebug>

using namespace Peony::Intel;

SideBarAbstractItem::SideBarAbstractItem(SideBarModel *model, QObject *parent) : QObject(parent)
{
    m_model = model;
    m_children = new QVector<SideBarAbstractItem*>();
}

SideBarAbstractItem::~SideBarAbstractItem()
{
    //qDebug()<<"~SideBarAbstractItem";
    for (auto child : *m_children) {
        delete child;
    }
    delete m_children;
}

void SideBarAbstractItem::clearChildren()
{
    m_model->removeRows(0, m_children->count(), firstColumnIndex());
    for (auto child : *m_children) {
        m_children->removeOne(child);
        child->deleteLater();
    }
    m_children->clear();
    //qDebug()<<"clear children has children"<<m_model->hasChildren(firstColumnIndex());
}
