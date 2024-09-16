// Copyright (C) 2020 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "multiselectlistview.h"

#include <QDebug>
#include <QKeyEvent>
#include <QMouseEvent>

MultiSelectListView::MultiSelectListView(QWidget *parent)
    : DListView(parent)
    , m_currentIndex(0)
{
}

// 重置状态
void MultiSelectListView::resetStatus(const QModelIndex &index)
{
    m_currentIndex = index.row();
    DListView::clearSelection();
    DListView::setSelectionMode(DListView::SingleSelection);
    DListView::setCurrentIndex(index);
}

// 当前刚选中改变
void MultiSelectListView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    DListView::setSelectionMode(DListView::SingleSelection);
    DListView::currentChanged(current, previous);
}

// 光标移动
QModelIndex MultiSelectListView::moveCursor(CursorAction cursorAction,
                                            Qt::KeyboardModifiers modifiers)
{
    DListView::setSelectionMode(DListView::MultiSelection);
    return DListView::moveCursor(cursorAction, modifiers);
}

// 键盘事件
void MultiSelectListView::keyPressEvent(QKeyEvent *event)
{
    // 获取向上方向键点击事件
    if (event->key() == Qt::Key_Up) {
        QModelIndex nextIndex = model()->index(m_currentIndex - 1, 0);
        if (nextIndex.isValid()) {
            if (isRowHidden(m_currentIndex - 1)) {
                if (m_currentIndex - 1 <= 0) {
                    return;
                }
                m_currentIndex--;
                keyPressEvent(event);
                return;
            }
            setCurrentIndex(nextIndex);
            Q_EMIT pressed(nextIndex);
        }
        return;
    } else if (event->key() == Qt::Key_Down) {
        // 获取向下方向键点击事件
        QModelIndex nextIndex = model()->index(m_currentIndex + 1, 0);
        if (nextIndex.isValid()) {
            if (isRowHidden(m_currentIndex + 1)) {
                if (model()->rowCount() - 1 <= m_currentIndex + 1) {
                    return;
                }
                m_currentIndex++;
                keyPressEvent(event);
                return;
            }
            setCurrentIndex(nextIndex);
            Q_EMIT pressed(nextIndex);
        }
        return;
    }
    return DListView::keyPressEvent(event);
}
