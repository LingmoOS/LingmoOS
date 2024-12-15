// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROGRAMMERARROWDELEGATE_H
#define PROGRAMMERARROWDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QDebug>
#include <DListWidget>

DWIDGET_USE_NAMESPACE

class ProgrammerArrowDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ProgrammerArrowDelegate(QObject *parent = nullptr);
    ~ProgrammerArrowDelegate() override;

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // PROGRAMMERARROWDELEGATE_H
