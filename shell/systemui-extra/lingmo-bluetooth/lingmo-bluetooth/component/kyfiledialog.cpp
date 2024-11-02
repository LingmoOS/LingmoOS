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

#include "kyfiledialog.h"

//被注释部分主要用于文件夹的选择，用于目录传输功能，暂不深度开发
KyFileDialog::KyFileDialog(QWidget *parent)
        : QFileDialog(parent)
{
    openButton = NULL;
    listView = NULL;
    treeView = NULL;
    _selectedFiles.clear();

    this->setWindowIcon(QIcon::fromTheme("preferences-system-bluetooth"));
    this->setWindowTitle(QString(tr("Bluetooth File")));
    this->setFileMode(QFileDialog::ExistingFiles);
//    QList<QPushButton*> btns = this->findChildren<QPushButton*>();

//    for (int i = 0; i < btns.size(); ++i) {
//        QString text = btns[i]->text();
//        if (text.toLower().contains("open") || text.toLower().contains("choose")
//            || btns[i]->text() == "打开(&O)" || btns[i]->text() == "选择(&C)")
//        {
//            openButton = btns[i];
//            break;
//        }
//    }

//    if (!openButton) {
//        qDebug() << "-- NULL --";
//        return;
//    }

//    openButton->installEventFilter(this);
//    openButton->disconnect(SIGNAL(clicked()));
//    connect(openButton, SIGNAL(clicked()), this, SLOT(chooseClicked()));

    //多选
    listView = findChild<QListView*>("listView");
    if (listView)
        listView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    treeView = findChild<QTreeView*>();
    if (treeView)
        treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

QStringList KyFileDialog::getselectedFiles()
{
    return _selectedFiles;
}

void KyFileDialog::chooseClicked()
{
    QModelIndexList indexList = listView->selectionModel()->selectedIndexes();
    foreach (QModelIndex index, indexList)
    {
    if (index.column()== 0)
        _selectedFiles.append(this->directory().absolutePath() + "/" + index.data().toString());
    }
    QDialog::accept();
}

//文件和文件夹均可选择
//bool KyFileDialog::eventFilter( QObject* watched, QEvent* event )
//{
//    QPushButton *btn = qobject_cast<QPushButton*>(watched);
//    if (btn)
//    {
//        if(event->type()==QEvent::EnabledChange) {
//            if (!btn->isEnabled())
//                btn->setEnabled(true);
//        }
//    }
//    return QWidget::eventFilter(watched, event);
//}
