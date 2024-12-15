// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QDebug>

#include "prolistmodel.h"
#include "../utils.h"

ProListModel::ProListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    clearItems();
}

ProListModel::~ProListModel() {}

void ProListModel::refrushModel()
{
    beginResetModel();
    endResetModel();
}

int ProListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_expressionList.size();
}

QVariant ProListModel::data(const QModelIndex &index, int role) const
{
    const int r = index.row();

    switch (role) {
    case ExpressionRole:
        return m_expressionList.at(r);
    case ExpressionCount:
        return m_expressionList.count();
    case ExpressionPrevious:
        return m_expressionList.value(r - 1);
    case ExpressionNext:
        return m_expressionList.value(r + 1);
    default:
        return QVariant();
    }
}

/**
 * @brief 显示区清零
 */
void ProListModel::clearItems()
{
    m_expressionList.clear();
    beginInsertRows(QModelIndex(), 0, 0);
    m_expressionList << "";
    endInsertRows();
    beginInsertRows(QModelIndex(), 1, 1);
    m_expressionList << "";
    endInsertRows();
    beginInsertRows(QModelIndex(), 2, 2);
    m_expressionList << "";
    endInsertRows();
    beginInsertRows(QModelIndex(), 3, 3);
    m_expressionList << "";
    endInsertRows();
}

/**
 * @brief 更新显示区
 * @param text　输入栏中的结果内容
 * @param index　当前选中行
 * 由当前选中行结果计算出其他行结果，同时更新四行数据
 */
void ProListModel::updataList(const Quantity ans)
{
    m_expressionList.clear();

    QString hex, dec, otc, bin;

    hex = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Hexadecimal()).remove("0x");
    otc = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65) + Quantity::Format::Octal()).remove("0o");
    bin = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65) + Quantity::Format::Binary()).remove("0b");
    dec = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65));

    while (bin.startsWith('0') && bin.length() > 1) {
        bin.remove(0, 1);
    }

    int i = bin.length() % 4;
    if (bin != "0") {
        while (i != 4 && i != 0) {
            bin.insert(0, "0");
            i++;
        }
    }

    hex = Utils::reformatSeparatorsPro(hex, 16);
    otc = Utils::reformatSeparatorsPro(otc, 8);
    bin = Utils::reformatSeparatorsPro(bin, 2);
    dec = Utils::reformatSeparatorsPro(dec, 10);

    //十六进制行
    beginInsertRows(QModelIndex(), 0, 0);
    m_expressionList << hex;
    endInsertRows();
    //十进制行
    beginInsertRows(QModelIndex(), 1, 1);
    m_expressionList << dec;
    endInsertRows();
    //八进制行
    beginInsertRows(QModelIndex(), 2, 2);
    m_expressionList << otc;
    endInsertRows();
    //二进制行
    beginInsertRows(QModelIndex(), 3, 3);
    m_expressionList << bin;
    endInsertRows();
}

void ProListModel::copyToClipboard(const int index)
{
    QClipboard *clipboard = QApplication::clipboard();
    QString copy = m_expressionList.at(index);
    clipboard->setText(copy.replace(" ", ""));
}
