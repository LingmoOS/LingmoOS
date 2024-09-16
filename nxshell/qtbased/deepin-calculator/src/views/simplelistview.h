// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIMPLELISTVIEW_H
#define SIMPLELISTVIEW_H

#include "simplelistdelegate.h"
#include "simplelistmodel.h"

#include <DListView>

#include <QScrollBar>
#include <DWidget>

DWIDGET_USE_NAMESPACE

class SimpleListView : public DListView
{
    Q_OBJECT

public:
    explicit SimpleListView(int mode = 0, DWidget *parent = nullptr); //mode:0-标准模式 1-科学模式
    ~SimpleListView() override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void listItemFill(bool itemfill); //itemfill中是否有历史记录
    void showTextEditMenuByAltM(const QModelIndex &index);

protected:
    void mouseMoveEvent(QMouseEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *e) override;
    void focusInEvent(QFocusEvent *event) override;
    void adjustScrollbarMargins();
Q_SIGNALS:
    void clickrow(QString str);
    void obtainingHistorical(const QModelIndex &index);
    void obtainingHistoricalSimple(const QModelIndex &index);

private:
    int m_mode = 0;
    int m_currentrow = -1;
    bool m_itemfill = false;
    bool m_ispressed = false;
    QPoint m_presspoint = QPoint();
    bool m_presschanged = false;
    int m_currentindexrow = 0;//当前row
};

#endif
