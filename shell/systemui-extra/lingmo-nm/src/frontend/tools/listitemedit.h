/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef LISTITEMEDIT_H
#define LISTITEMEDIT_H

#include <QWidget>
#include <QStyledItemDelegate>
#include <QLineEdit>

class ListItemEdit: public QStyledItemDelegate
{
    Q_OBJECT
public:
    ListItemEdit(const QRegExp &rx, QObject *parent = nullptr);
    ~ListItemEdit() = default;

    //创建一个控件
    virtual QWidget *createEditor (QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    //将数据设置到控件中
    virtual void setEditorData (QWidget *editor, const QModelIndex &index) const;
    //将控件中的数据更新到对应的model中
    virtual void setModelData (QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    //更新控件位置
    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;
private:
    QRegExp m_regExp;

};

#endif // LISTITEMEDIT_H
