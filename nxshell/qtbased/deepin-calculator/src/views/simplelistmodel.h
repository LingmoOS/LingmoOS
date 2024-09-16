// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIMPLELISTMODEL_H
#define SIMPLELISTMODEL_H

#include "../../3rdparty/core/evaluator.h"

#include <QAbstractListModel>

class SimpleListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    SimpleListModel(int mode = 0, QObject *parent = nullptr);//mode:0-标准模式 1-科学模式
    ~SimpleListModel() override;
    void refrushModel();

    //model的data数据类型
    enum Role {
        ExpressionWithOutTip = Qt::DisplayRole,//无提示文本
        ExpressionRole = Qt::ToolTipRole, //有提示文本
        ExpressionCount = 4, //数据数量
        ExpressionPrevious, //前一个数据
        ExpressionNext, //下一个数据
    };

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    void appendText(const QString &text, bool sci);
    void clearItems();
    void updataList(const QString &text, const int index, bool sci = false);
    void updataList(Quantity ans, const QString &text, const int index);
    void deleteItem(const int index);
    void copyToClipboard(const int index);
    Quantity getAnswer(const int index);
    //进制切换
    void radixChanged(int baseori, int basedest);
    bool isNumber(QChar a);//判断是否为数字(分隔符)
    QString formatExpression(const int &probase, const QString &text);
    void answerOutOfRange(Quantity ans);
    void expOutofRange();
    //更改数据分割位数
    void updataOfSeparate();

signals:
    void hisbtnhidden();

private:
    QList<QString> m_expressionList;
    bool m_selectedStatus;
    int m_mode = 0; //0-标准模式 1-科学模式
    QList<Quantity> m_answerlist; //对应历史记录的answer

    //进制切换时用于替换所有数字
    QVector<QString> m_numvec;
    QVector<QString> m_opvec;
    QString m_textorder;
    bool m_numchanged = false;
};

#endif
