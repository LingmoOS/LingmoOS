// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DRIVERLISTVIEW_H
#define DRIVERLISTVIEW_H

#include <DTreeView>
#include <QMouseEvent>

DWIDGET_USE_NAMESPACE

class DriverListView : public DTreeView
{
    Q_OBJECT
public:
    explicit DriverListView(QWidget *parent = nullptr);
    void initUI();
    /**
     * @brief keyPressEvent 重写按钮事件
     */
    void keyPressEvent(QKeyEvent *event) override;
public slots:

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *event) override;
    void drawRow(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const override;
};

#endif // DRIVERLISTVIEW_H
