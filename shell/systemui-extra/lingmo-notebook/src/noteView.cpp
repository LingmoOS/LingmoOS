/*
* Copyright (C) 2020 Tianjin LINGMO Information Technology Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include <QDebug>
#include <QPainter>
#include <QApplication>
#include <QAbstractItemView>
#include <QPaintEvent>
#include <QSortFilterProxyModel>
#include <QTimer>
#include <QScrollBar>

#include "noteView.h"
#include "listViewModeDelegate.h"

NoteView::NoteView(QWidget *parent)
    : QListView( parent )
    , m_isScrollBarHidden(true)
    , m_animationEnabled(true)
    , m_isMousePressed(false)
    , m_rowHeight(38)
{
    //不可编辑
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setWindowOpacity(0.7);
    setAttribute(Qt::WA_TranslucentBackground);//设置窗口透明显示(毛玻璃效果)
    viewport()->setAttribute(Qt::WA_TranslucentBackground);
    //一次性定时器,槽函数只处理一次
    QTimer::singleShot(0, this, SLOT(init()));
}

NoteView::~NoteView()
{
}

void NoteView::animateAddedRow(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(parent)
    Q_UNUSED(end)

    //model(): 返回此视图显示的模型。
    QModelIndex idx = model()->index(start,0);
    // Note: 这一行添加了flikering，当动画运行缓慢时可以看到
    // QItemSelectionModel::ClearAndSelect 将清除完整的选择 | 将选择所有指定的索引
    // selectionModel(): Returns the current selection model.
    // select(): 使用指定的命令选择模型项索引，并发出selectionChanged（）
    selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect);

    listViewModeDelegate* delegate = static_cast<listViewModeDelegate*>(itemDelegate());
    if(delegate != Q_NULLPTR){
        delegate->setState(listViewModeDelegate::Insert, idx);

        // TODO find a way to finish this function till the animation stops
        while(delegate->animationState() == QTimeLine::Running){
            qApp->processEvents();
        }
    }
}

void NoteView::animateRemovedRow(const QModelIndex& parent, int start, int end)
{
    qDebug() << "当前文件 :" << __FILE__ << "当前函数 :" << __FUNCTION__ << "当前行号 :" << __LINE__;
    Q_UNUSED(parent)
    Q_UNUSED(end)

    //model(): 返回此视图显示的模型。
    QModelIndex idx = model()->index(start,0);
    // Note: 这一行添加了flikering，当动画运行缓慢时可以看到
    // QItemSelectionModel::ClearAndSelect 将清除完整的选择 | 将选择所有指定的索引
    // selectionModel(): Returns the current selection model.
    // select(): 使用指定的命令选择模型项索引，并发出selectionChanged（）
    selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect);

    listViewModeDelegate* delegate = static_cast<listViewModeDelegate*>(itemDelegate());
    if(delegate != Q_NULLPTR){
        delegate->setCurrentSelectedIndex(QModelIndex());
        delegate->setState(listViewModeDelegate::Remove, idx);

        // TODO find a way to finish this function till the animation stops
        while(delegate->animationState() == QTimeLine::Running){
            qApp->processEvents();
        }
    }
}

void NoteView::paintEvent(QPaintEvent *e)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this->viewport());
    //p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(opt.palette.color(QPalette::Base));
    //p.setOpacity(0.7);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(opt.rect,1,1);

    QListView::paintEvent(e);
}

void NoteView::rowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
                                  const QModelIndex &destinationParent, int destinationRow)
{
    Q_UNUSED(sourceParent)
    Q_UNUSED(sourceEnd)
    Q_UNUSED(destinationParent)
    Q_UNUSED(destinationRow)

    if(model() != Q_NULLPTR){
        QModelIndex idx = model()->index(sourceStart,0);
        listViewModeDelegate* delegate = static_cast<listViewModeDelegate*>(itemDelegate());
        if(delegate != Q_NULLPTR){

            if(m_animationEnabled){
                delegate->setState(listViewModeDelegate::MoveOut, idx);
            }else{
                delegate->setState(listViewModeDelegate::Normal, idx);
            }

            // TODO find a way to finish this function till the animation stops
            while(delegate->animationState() == QTimeLine::Running){
                qApp->processEvents();
            }
        }
    }
}

void NoteView::rowsMoved(const QModelIndex &parent, int start, int end,
                         const QModelIndex &destination, int row)
{
    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)
    Q_UNUSED(destination)

    QModelIndex idx = model()->index(row,0);
    setCurrentIndex(idx);

    listViewModeDelegate* delegate = static_cast<listViewModeDelegate*>(itemDelegate());
    if(delegate == Q_NULLPTR)
        return;

    if(m_animationEnabled){
        delegate->setState(listViewModeDelegate::MoveIn, idx );
    }else{
        delegate->setState(listViewModeDelegate::Normal, idx);
    }

    // TODO find a way to finish this function till the animation stops
    while(delegate->animationState() == QTimeLine::Running){
        qApp->processEvents();
    }
}

void NoteView::init()
{
    setMouseTracking(true);
    setUpdatesEnabled(true);
    //当鼠标进入或离开小部件时，强制Qt生成绘制事件
    viewport()->setAttribute(Qt::WA_Hover);
}

void NoteView::mouseMoveEvent(QMouseEvent* e)
{
    if(!m_isMousePressed){
        QListView::mouseMoveEvent(e);
    }
}

void NoteView::mousePressEvent(QMouseEvent* e)
{
    m_isMousePressed = true;
    QListView::mousePressEvent(e);
}

void NoteView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    emit viewportPressed();
    if (selected.indexes().isEmpty()){
        return;
    }
    QListView::selectionChanged(selected, deselected);
}

void NoteView::mouseReleaseEvent(QMouseEvent* e)
{
    m_isMousePressed = false;
    QListView::mouseReleaseEvent(e);
}

void NoteView::setCurrentRowActive(bool isActive)
{
    qDebug() << "当前文件 :" << __FILE__ << "当前函数 :" << __FUNCTION__ << "当前行号 :" << __LINE__;
    listViewModeDelegate* delegate = static_cast<listViewModeDelegate*>(itemDelegate());
    if(delegate == Q_NULLPTR)
        return;

    delegate->setActive(isActive);
    viewport()->update(visualRect(currentIndex()));
}

void NoteView::setAnimationEnabled(bool isEnabled)
{
    m_animationEnabled = isEnabled;
}
