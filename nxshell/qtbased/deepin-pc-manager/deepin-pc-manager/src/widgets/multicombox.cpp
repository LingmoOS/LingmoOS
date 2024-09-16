// Copyright (C) 2021 ~ 2021 Uniontech Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "multicombox.h"

#include "menuitem.h"

#include <QDebug>
#include <QLineEdit>
#include <QListView>
#include <QMouseEvent>
#include <QStandardItemModel>

MultiComBox::MultiComBox(QWidget *parent)
    : QComboBox(parent)
{
    this->setStyleSheet("QComboBox {combobox-popup: 0;}");
    this->view()->setMaximumHeight(200);

    // 为 ComBox 设置编辑框
    m_pLineEdit = new QLineEdit(this);
    m_pLineEdit->setReadOnly(true);
    this->setLineEdit(m_pLineEdit);

    // 设置 ComBox 下拉界面
    m_pListView = new QListView(this);
    QFont f = m_pListView->font();
    f.setPixelSize(14);
    m_pListView->setFont(f);
    m_pListView->installEventFilter(this);
    this->setView(m_pListView);

    // 设置 ComBox 数据模型
    m_pItemModel = new QStandardItemModel(this);
    this->setModel(m_pItemModel);

    connect(m_pListView, &QListView::pressed, this, &MultiComBox::sltActivated);
}

MultiComBox::~MultiComBox() { }

//*************************************************
// Function:        AddItems
// Description:     添加下拉菜单内容
// Input:           menus：菜单内容列表
// Return:          若中途执行失败，则返回false，中断操作；否则，返回真，继续后续操作
//*************************************************
void MultiComBox::addItems(QList<MenuItem *> menus)
{
    QStandardItem *pNewItem;
    foreach (MenuItem *pMenuItem, menus) {
        if (pMenuItem == nullptr) {
            // 释放内存
            delete pMenuItem;
            pMenuItem = nullptr;
            continue;
        }
        pNewItem = new QStandardItem(pMenuItem->GetMenuName());
        pNewItem->setCheckState(pMenuItem->GetIsChecked() ? Qt::Checked : Qt::Unchecked);
        pNewItem->setData(pMenuItem->GetMenuId());
        m_pItemModel->appendRow(pNewItem);

        // 释放内存
        delete pMenuItem;
        pMenuItem = nullptr;
    }
}

QStringList MultiComBox::getItemsText()
{
    QStringList strs;
    QString str = m_pLineEdit->text();
    if (!str.isEmpty()) {
        strs = str.split(";");
    }
    return strs;
}

QList<int> MultiComBox::getItemsId()
{
    QList<int> ids;
    for (int i = 0; i < m_pItemModel->rowCount(); i++) {
        QStandardItem *item = m_pItemModel->item(i);
        if (item->checkState() == Qt::Checked) {
            ids << item->data().toInt();
        }
    }
    return ids;
}

//*************************************************
// Function:        hidePopup
// Description:     根据鼠标相应位置，判断是否隐藏下拉菜单
//*************************************************
void MultiComBox::hidePopup()
{
    int width = this->view()->width();
    int height = this->view()->height();
    int x = QCursor::pos().x() - mapToGlobal(geometry().topLeft()).x() + geometry().x();
    int y = QCursor::pos().y() - mapToGlobal(geometry().topLeft()).y() + geometry().y();

    QRect rectView(0, this->height(), width, height);
    if (!rectView.contains(x, y) && this->view()->isVisible()) {
        QComboBox::hidePopup();
        updateText();

        // 发送全部已选选项的index  用于日志
        QList<int> listIndex;
        for (int i = 0; i < m_pItemModel->rowCount(); i++) {
            QStandardItem *item = m_pItemModel->item(i);
            if (item->checkState() == Qt::Checked) {
                listIndex.append(i);
            }
        }
        Q_EMIT sendSelected(listIndex);
    }
}

//*************************************************
// Function:        UpdateText
// Description:     更新lineedit文本
//*************************************************
void MultiComBox::updateText()
{
    QStringList strs;
    for (int i = 0; i < m_pItemModel->rowCount(); i++) {
        QStandardItem *item = m_pItemModel->item(i);
        if (item->checkState() == Qt::Checked) {
            strs << item->text();
        }
    }
    m_pLineEdit->setText(strs.join(";"));
    m_pLineEdit->setToolTip(strs.join("\n"));
}

void MultiComBox::sltActivated(const QModelIndex &index)
{
    QStandardItem *item = m_pItemModel->item(index.row());
    Qt::CheckState checkState = item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked;
    item->setCheckState(checkState);
}

void MultiComBox::setActivated(int row)
{
    QStandardItem *item = m_pItemModel->item(row);
    Qt::CheckState checkState = Qt::Checked;
    item->setCheckState(checkState);

    // 每次初始化显示全部日期
    updateText();
}

QString MultiComBox::getSelectedRows()
{
    QStringList strs;
    for (int i = 0; i < m_pItemModel->rowCount(); i++) {
        QStandardItem *item = m_pItemModel->item(i);
        if (item->checkState() == Qt::Checked) {
            strs << QString::number(i);
        }
    }

    return strs.join(";");
}

QString MultiComBox::getSelectedIndexs()
{
    QStringList strs;
    for (int i = 0; i < m_pItemModel->rowCount(); i++) {
        QStandardItem *item = m_pItemModel->item(i);
        if (item->checkState() == Qt::Checked) {
            strs << QString::number(i);
        } else if (item->checkState() == Qt::Unchecked) {
            strs.removeOne(QString::number(i));
        }
    }
    return strs.join(";");
}
