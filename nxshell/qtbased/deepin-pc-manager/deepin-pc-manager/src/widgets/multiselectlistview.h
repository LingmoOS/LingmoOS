// Copyright (C) 2020 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MULTISELECTLISTVIEW_H
#define MULTISELECTLISTVIEW_H

#include <DListView>

DWIDGET_USE_NAMESPACE

// 列表视图代理 -- 主要为实现方向键切换功能
class MultiSelectListView : public DListView
{
    Q_OBJECT
public:
    explicit MultiSelectListView(QWidget *parent = nullptr);
    void resetStatus(const QModelIndex &index);

protected:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    int m_currentIndex;
};

#endif // MULTISELECTLISTVIEW_H
