// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2021-2021 Uniontech Technology Co., Ltd.
 *
 * @file searchresoultwidget.cpp
 *
 * @brief 搜索框类
 *
 * @date 2021-01-08 15:22
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
 **/


#include "searchresoultwidget.h"
#include <QListWidgetItem>
#include <QWidget>
#include <QKeyEvent>
#include <dplatformwindowhandle.h>
#include "global.h"
#include <DGuiApplicationHelper>

SearchResoultWidget::SearchResoultWidget(QWidget *parent)
    : QListWidget(parent)
{
    setMinimumWidth(380);
    setFixedHeight(280);
    DPlatformWindowHandle handle(this);
    handle.setWindowRadius(18);
    setIconSize(QSize(12, 12));
    setSpacing(2);
}

void SearchResoultWidget::setData(QList<QString> &taskIDList,
                                  QList<int> &taskStatusList, QList<QString> &tasknameList)
{
    this->clear();
    bool first = true;
    for(int i = 0; i< taskIDList.count(); i++){
        QListWidgetItem *item = new QListWidgetItem;
        QString text = "   ";
        if(taskStatusList.at(i) == Global::Complete) {
            if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
                item->setIcon(QIcon::fromTheme("dcc_print_done", QIcon(":/icons/deepin/builtin/dark/actions/dcc_print_done_dark_11px.svg")));
            } else if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
                item->setIcon(QIcon::fromTheme("dcc_print_done", QIcon(":/icons/deepin/builtin/light/actions/dcc_print_done_11px.svg")));
            }
            text += tr("Completed");
            item->setData(Qt::UserRole, "Completed");
        } else if(taskStatusList.at(i) == Global::Removed) {
            if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
                item->setIcon(QIcon::fromTheme("dcc_list_delete", QIcon(":/icons/deepin/builtin/dark/actions/dcc_list_delete_dark_11px.svg")));
            } else if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
                item->setIcon(QIcon::fromTheme("dcc_list_delete", QIcon(":/icons/deepin/builtin/light/actions/dcc_list_delete_11px.svg")));
            }
            text += tr("Trash");
            item->setData(Qt::UserRole, "Trash");
        } else {
            if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
                item->setIcon(QIcon::fromTheme("dcc_list_downloading", QIcon(":/icons/deepin/builtin/dark/actions/dcc_list_downloading_dark_11px.svg")));
            } else if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
                item->setIcon(QIcon::fromTheme("dcc_list_downloading", QIcon(":/icons/deepin/builtin/light/actions/dcc_list_downloading_11px.svg")));
            }
            text += tr("Downloading");
            item->setData(Qt::UserRole, "Downloading");
        }

        item->setText(text + "  -->  " + tasknameList.at(i));
        item->setData(Qt::WhatsThisRole, taskIDList.at(i));
        item->setData(Qt::AccessibleTextRole, tasknameList.at(i));
        item->setData(Qt::AccessibleDescriptionRole, tasknameList.at(i));

        addItem(item);
        if(first) {
            setCurrentItem(item);
            first = false;
        }
    }

}

void SearchResoultWidget::onKeypressed(Qt::Key k)
{
    QModelIndex index = currentIndex();
    if(k == Qt::Key_Up) {
        if(currentItem() == nullptr){
            setCurrentIndex(index.sibling(0,0));
            return;
        }
        if(index.row() - 1 < 0) {
            return;
        }
        setCurrentIndex(index.sibling
                        (index.row() - 1, index.column()));
    } else if(k == Qt::Key_Down) {
        if(currentItem() == nullptr){
            setCurrentIndex(index.sibling(0,0));
            return;
        }
        if(index.row() + 1 >= count()) {
            return;
        }
        setCurrentIndex(index.sibling
                        (index.row() + 1, index.column()));
    } else if(k == Qt::Key_Enter) {
        if(currentItem() != nullptr) {
            emit itemClicked(currentItem());
        }
    }
}

void SearchResoultWidget::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    this->hide();
}

void SearchResoultWidget::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Enter) {
        emit itemClicked(currentItem());
    }
}
