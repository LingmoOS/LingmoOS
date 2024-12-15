// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROLISTMODEL_H
#define PROLISTMODEL_H

#include "../../3rdparty/core/evaluator.h"

#include <QAbstractListModel>
#include <QClipboard>
#include <QApplication>

class ProListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ProListModel(QObject *parent = nullptr);
    ~ProListModel() override;
    void refrushModel();

    //model的data数据类型
    enum Role {
        ExpressionRole = Qt::DisplayRole, //无提示文本
        ExpressionCount = 4, //数据数量
        ExpressionPrevious, //前一个数据
        ExpressionNext //下一个数据
    };

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    void clearItems();
    void updataList(const Quantity ans);
    void copyToClipboard(const int index);

private:
    QList<QString> m_expressionList;
};

#endif // PROLISTMODEL_H
