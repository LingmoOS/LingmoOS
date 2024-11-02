/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#ifndef KYFILEDIALOG_H
#define KYFILEDIALOG_H

#include <QFileDialog>
#include <QApplication>
#include <QWidget>
#include <QTreeWidget>
#include <QPushButton>
#include <QStringList>
#include <QModelIndex>
#include <QDir>
#include <QDebug>
#include <QListView>
#include <QTreeView>

//此类意在改写QFileDialog无法同时选定文件和文件夹的问题
class KyFileDialog : public QFileDialog
{
    Q_OBJECT
public:
    explicit KyFileDialog(QWidget *parent = Q_NULLPTR);

        QStringList getselectedFiles();

        //取消禁止选择文件
//        bool eventFilter( QObject* watched, QEvent* event );

    public slots:
        void chooseClicked();

    private:
        QListView *listView;
        QTreeView *treeView;
        QPushButton *openButton;
        QStringList _selectedFiles;
};

#endif // KYFILEDIALOG_H
