// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROLISTVIEW_H
#define PROLISTVIEW_H

#include "prolistmodel.h"
#include "prolistdelegate.h"

#include <DWidget>
#include <DListView>

#include <QModelIndex>
#include <QDebug>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QMouseEvent>

DWIDGET_USE_NAMESPACE

class ProListView : public DListView
{
    Q_OBJECT

public:
    explicit ProListView(DWidget *parent = nullptr);
    ~ProListView() override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void showTextEditMenuByAltM(const QModelIndex &index);
    QModelIndex focusIndex() const;
    QModelIndex indexBeforeFocusOut() const;

public slots:
    void itemclicked(const QModelIndex &index, bool isspace);

protected:
//    void mouseMoveEvent(QMouseEvent *e);
//    void mousePressEvent(QMouseEvent *event);
//    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *e) override;
    void focusInEvent(QFocusEvent *event) override;
Q_SIGNALS:
    void obtainingHistorical(const QModelIndex &index, bool isspace);

private:
    int m_focusrow = 1;
    int m_currentrow = 1;
    bool m_isMenuAltM = false; //是否是altM的菜单
};

#endif // PROLISTVIEW_H
