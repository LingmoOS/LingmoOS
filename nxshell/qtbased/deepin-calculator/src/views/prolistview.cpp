// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "prolistview.h"

ProListView::ProListView(DWidget *parent)
    : DListView(parent)
{
    setSelectionBehavior(QAbstractItemView::SelectRows); //选中一行
    setSelectionMode(QAbstractItemView::SingleSelection); //选中单个目标
    setFixedSize(451, 106);
    setMouseTracking(true);
    setFocusPolicy(Qt::TabFocus);

    connect(this, &DListView::clicked, [ = ](const QModelIndex & index) {
        itemclicked(index, false);
    });
}

ProListView::~ProListView()
{

}

void ProListView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = new QMenu(this);
    //缺翻译
    QAction *copy = new QAction(tr("Copy"), menu);
    menu->addAction(copy);
    connect(copy, &QAction::triggered, this, [ = ]() {
        static_cast<ProListModel *>(model())->copyToClipboard(indexAt(event->pos()).row());
    });
    menu->exec(event->globalPos());
    delete menu;
}

void ProListView::showTextEditMenuByAltM(const QModelIndex &index)
{
    m_isMenuAltM = true;
    QMenu *menu = new QMenu(this);
    //缺翻译
    QAction *copy = new QAction(tr("Copy"), menu);
    menu->addAction(copy);
    connect(copy, &QAction::triggered, this, [ = ]() {
        static_cast<ProListModel *>(model())->copyToClipboard(index.row());
    });
    QPoint menupoint;
    menupoint.setX(mapToGlobal(visualRect(index).bottomRight()).x() - 10);
    menupoint.setY(mapToGlobal(visualRect(index).center()).y());
    menu->exec(menupoint);
    delete menu;
}

/**
 * @brief ProListView::focusIndex
 * @return 返回当前focus的索引
 */
QModelIndex ProListView::focusIndex() const
{
    return model()->index(m_focusrow, 0);
}

QModelIndex ProListView::indexBeforeFocusOut() const
{
    return model()->index(m_currentrow, 0);
}

/**
 * @brief ProListView::itemclicked
 * @param index:点击索引
 * @param isspace:是否为键盘交互
 * 进制列表的点击事件
 */
void ProListView::itemclicked(const QModelIndex &index, bool isspace)
{
    m_currentrow = index.row();
    this->setCurrentIndex(index);
    emit obtainingHistorical(index, isspace);
}

void ProListView::keyPressEvent(QKeyEvent *e)
{
    bool ispressalt = e->modifiers() == Qt::AltModifier;
    switch (e->key()) {
    case Qt::Key_Up:
        if (m_focusrow)
            m_focusrow--;
        else
            m_focusrow = 3;
        static_cast<ProListDelegate *>(itemDelegate(currentIndex()))->currentfocusindex(focusIndex());
        update();
        break;
    case Qt::Key_Down:
        if (m_focusrow < 3)
            m_focusrow++;
        else
            m_focusrow = 0;
        static_cast<ProListDelegate *>(itemDelegate(currentIndex()))->currentfocusindex(focusIndex());
        update();
        break;
    case Qt::Key_Space:
    case Qt::Key_Enter:
    case Qt::Key_Return:
        itemclicked(model()->index(m_focusrow, 0), true);
//        emit obtainingHistorical(model()->index(m_focusrow, 0));
        break;
    case Qt::Key_M:
        if (ispressalt)
            showTextEditMenuByAltM(model()->index(m_focusrow, 0));
        break;
    default:
        DListView::keyPressEvent(e);
        break;
    }
}

/**
 * @brief 焦点进入时设置焦点在第一条历史记录
 */
void ProListView::focusInEvent(QFocusEvent *event)
{
//    setCurrentIndex(this->model()->index(currentIndex().row(), 0));
    //当alt+M的菜单取消后，focus到原本的焦点行
    if (m_isMenuAltM) {
        static_cast<ProListDelegate *>(itemDelegate(currentIndex()))->currentfocusindex(focusIndex());
    } else {
        setCurrentIndex(indexBeforeFocusOut());
        static_cast<ProListDelegate *>(itemDelegate(currentIndex()))->currentfocusindex(currentIndex());
        m_focusrow = currentIndex().row();
    }
    scrollTo(this->model()->index(currentIndex().row(), 0));
    DListView::focusInEvent(event);
}
