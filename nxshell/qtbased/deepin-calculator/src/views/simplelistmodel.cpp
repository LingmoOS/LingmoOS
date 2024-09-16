// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "simplelistmodel.h"

#include <QDebug>
#include <QClipboard>
#include <QApplication>

#include "../utils.h"
#include "../globaldefine.h"
#include "../../3rdparty/core/settings.h"

const QString AtoF = "ABCDEF";

SimpleListModel::SimpleListModel(int mode, QObject *parent)
    : QAbstractListModel(parent)
{
    m_selectedStatus = false;
    if (mode == 1) {
        m_mode = mode;
        clearItems();
    }
}

SimpleListModel::~SimpleListModel() {}

/**
 * @brief 返回历史记录条数
 */
int SimpleListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_expressionList.size();
}

/**
 * @brief delegate中获取表达式；ExpressionRole：显示提示文本
 */
QVariant SimpleListModel::data(const QModelIndex &index, int role) const
{
    const int r = index.row();

    switch (role) {
    case ExpressionWithOutTip:
        return m_expressionList.at(r);
    case ExpressionRole:
        if (m_mode == 1)
            return QVariant();
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
 * @brief  科学模式上方历史记录区
 */
void SimpleListModel::appendText(const QString &text, bool sci)
{
    auto expression = text.simplified();

    const int size = m_expressionList.size();

    if (sci) {
        beginRemoveRows(QModelIndex(), 0, 0);
        m_expressionList.removeAt(0);
        endRemoveRows();

        beginInsertRows(QModelIndex(), 0, 0);
        m_expressionList << expression;
        endInsertRows();
    } else {
        beginInsertRows(QModelIndex(), size, size);
        m_expressionList << expression;
        endInsertRows();
    }
}

/**
 * @brief 清空历史记录
 */
void SimpleListModel::clearItems()
{
    beginRemoveRows(QModelIndex(), 0, m_expressionList.size());
    m_expressionList.clear();
    endRemoveRows();
    m_answerlist.clear();
    if (m_mode == 1) {
        beginInsertRows(QModelIndex(), 0, 0);
        m_expressionList << tr("No history");
        endInsertRows();
    }
}

/**
 * @brief ResetModel
 */
void SimpleListModel::refrushModel()
{
    beginResetModel();
    endResetModel();
}

/**
 * @brief 科学模式上方历史记录区与标准模式添加历史记录
 */
void SimpleListModel::updataList(const QString &text, const int index, bool sci)
{
    if (m_expressionList.count() == 500) {
        if (sci)
            deleteItem(499);
        else
            deleteItem(0);
    }
    QString exp = text;
    exp = exp.replace(EN_ADD, CN_ADD)
          .replace(EN_MIN, CN_MIN)
          .replace(EN_MUL, CN_MUL);

    if (index == -1) {
        appendText(exp, sci); //科学模式上方历史记录区
    } else {
        if (sci) {
            beginInsertRows(QModelIndex(), index, index);
            m_expressionList.insert(index, exp);
            endInsertRows();
        } else {
            beginRemoveRows(QModelIndex(), index, index);
            m_expressionList.removeAt(index);
            m_expressionList.insert(index, exp);
            endRemoveRows();
        }
    }
}

/**
 * @brief 添加历史记录
 */
void SimpleListModel::updataList(Quantity ans, const QString &text, const int index)
{
    if (m_expressionList.count() == 500) {
        deleteItem(499); //历史记录不超过500条
        m_answerlist.pop_back();

    }
    QString exp = text;
    exp = exp.replace(EN_ADD, CN_ADD)
          .replace(EN_MIN, CN_MIN)
          .replace(EN_MUL, CN_MUL);

    beginInsertRows(QModelIndex(), index, index);
    m_expressionList.insert(index, exp);
    endInsertRows();
    m_answerlist.insert(0, ans);
}

/**
 * @brief 删除单条历史记录
 */
void SimpleListModel::deleteItem(const int index)
{
    beginRemoveRows(QModelIndex(), index, index);
    m_expressionList.removeAt(index);
    endRemoveRows();
    if (m_answerlist.count() > 0)
        m_answerlist.removeAt(index);
    if (m_expressionList.count() == 0) {
        emit hisbtnhidden(); //发送历史记录无数据信号
    }
}

/**
 * @brief 复制到粘贴板
 */
void SimpleListModel::copyToClipboard(const int index)
{
    QClipboard *clipboard = QApplication::clipboard();
    QString copy = m_expressionList.at(index);
    clipboard->setText(copy.replace(" ", ""));
}

/**
 * @brief 返回所点击历史记录的answer.Quantity
 */
Quantity SimpleListModel::getAnswer(const int index)
{
    if (m_answerlist.count() > index)
        return m_answerlist.at(index);
    return Quantity();
}

void SimpleListModel::radixChanged(int baseori, int basedest)
{
    if (m_expressionList.count() <= 0)
        return;
    m_numvec.clear();
    m_opvec.clear();
    m_textorder = QString();
    m_numchanged = false;
    QString oldtext = m_expressionList.at(0).left(m_expressionList.at(0).indexOf("＝") + 1);
    QString resultnum = m_expressionList.at(0).split("＝").last();
    oldtext.remove("，").remove(" ");
    resultnum.remove("，").remove(" ");
    for (int i = 0; i < oldtext.length();) {
        if (isNumber(oldtext.at(i))) {
            for (int j = 0; j < oldtext.length() - i; j++) {
                if (i + j == oldtext.length() - 1) {
                    if (isNumber(oldtext.at(i + j))) {
                        m_numvec.append(oldtext.mid(i, j + 1));
                        m_textorder += "0";
                        i += j + 1;
                    } else {
                        m_numvec.append(oldtext.mid(i, j));
                        m_textorder += "0";
                        i += j;
                    }
                    break;
                }
                if (!isNumber(oldtext.at(i + j))) {
                    m_numvec.append(oldtext.mid(i, j));
                    m_textorder += "0";
                    i += j;
                    break;
                }
            }
        } else {
            if (oldtext.at(i).isLower()) {
                if (oldtext.at(i) == 'n' && oldtext.at(i + 1) == 'a') {
                    m_opvec.append(oldtext.mid(i, 4));
                    m_textorder += "1";
                    i += 4;
                } else if (oldtext.at(i) == 'o') {
                    m_opvec.append(oldtext.mid(i, 2));
                    m_textorder += "1";
                    i += 2;
                } else {
                    m_opvec.append(oldtext.mid(i, 3));
                    m_textorder += "1";
                    i += 3;
                }
            } else if ((i == 0 || !isNumber(oldtext.at(i - 1))) && oldtext.at(i) == CN_MIN
                       && oldtext.length() > i + 1 && isNumber(oldtext.at(i + 1))) {
                i++;
                for (int j = 0; j < oldtext.length() - i; j++) {
                    if (!isNumber(oldtext.at(i + j))) {
                        m_numvec.append(oldtext.mid(i - 1, j + 1));
                        m_textorder += "0";
                        i += j;
                        break;
                    }
                    if (i + j == oldtext.length() - 1) {
                        m_numvec.append(oldtext.mid(i - 1, j + 2));
                        m_textorder += "0";
                        i += j + 1;
                        break;
                    }
                }
            } else {
                m_opvec.append(oldtext.at(i));
                m_textorder += "1";
                i++;
            }
        }
    }
    for (int i = 0; i < m_numvec.size(); i++) {
        QString num = formatExpression(baseori, m_numvec.at(i));
        Quantity ans(HNumber(num.toLatin1().data()));
        switch (basedest) {
        case 16:
            num = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Hexadecimal()).remove("0x");
            break;
        case 8:
            num = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65) + Quantity::Format::Octal()).remove("0o");
            break;
        case 2:
            num = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65) + Quantity::Format::Binary()).remove("0b");
            break;
        default:
            num = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65));
            break;
        }
        if (m_numvec.at(i) != num)
            m_numchanged = true;
        m_numvec.replace(i, num);
    }
    QString result = formatExpression(baseori, resultnum);
    Quantity ans(HNumber(result.toLatin1().data()));
    switch (basedest) {
    case 16:
        result = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Hexadecimal()).remove("0x");
        break;
    case 8:
        result = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65) + Quantity::Format::Octal()).remove("0o");
        break;
    case 2:
        result = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65) + Quantity::Format::Binary()).remove("0b");
        break;
    default:
        result = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65));
        break;
    }
    QString newtext = QString();
    for (int i = 0; i < m_textorder.length(); i++) {
        if (m_textorder.at(i) == "0") {
            newtext.append(m_numvec.first());
            m_numvec.pop_front();
        } else {
            newtext.append(m_opvec.first());
            m_opvec.pop_front();
        }
    }
    newtext = newtext + (ans.isNan() ? m_expressionList.at(0).split("＝").last() : result);
    newtext = Utils::reformatSeparatorsPro(newtext, basedest).replace(EN_ADD, CN_ADD)
              .replace(EN_MIN, CN_MIN)
              .replace(EN_MUL, CN_MUL)
              .replace(EN_DIV, CN_DIV);
    m_expressionList.clear();
    beginInsertRows(QModelIndex(), 0, 0);
    m_expressionList << newtext;
    endInsertRows();
}

bool SimpleListModel::isNumber(QChar a)
{
    if (a.isDigit() || a == " " || a == "," || AtoF.contains(a))
        return true;
    else
        return false;
}

QString SimpleListModel::formatExpression(const int &probase, const QString &text)
{
    QString formattext = text;
    formattext.replace(CN_ADD, EN_ADD)
    .replace(CN_MIN, EN_MIN)
    .replace(CN_MUL, EN_MUL)
    .replace(CN_DIV, EN_DIV)
    .replace(EN_Comma, "")
    .replace(QString::fromUtf8(" "), "")
    .replace(EN_Percent, "mod");

    QString base = QString();
    switch (probase) {
    case 16:
        base = "0x";
        break;
    case 8:
        base = "0o";
        break;
    case 2:
        base = "0b";
        break;
    default:
        break;
    }
    if (base != QString()) {
        for (int i = 0; i < formattext.length();) {
            if ((i == 0) && isNumber(formattext.at(i))) {
                formattext.insert(i, base);
                i += 3;
                continue;
            } else if ((i < formattext.length() - 1) && !isNumber(formattext.at(i)) && isNumber(formattext.at(i + 1))) {
                formattext.insert(i + 1, base);
                i += 3;
                continue;
            }
            i++;
        }
    }
    return formattext;
}

void SimpleListModel::answerOutOfRange(Quantity ans)
{
    QString resultnum;
    switch (Settings::instance()->programmerBase) {
    case 16:
        resultnum = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Hexadecimal()).remove("0x");
        break;
    case 8:
        resultnum = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65) + Quantity::Format::Octal()).remove("0o");
        break;
    case 2:
        resultnum = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65) + Quantity::Format::Binary()).remove("0b");
        break;
    default:
        resultnum = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65));
        break;
    }
    if (m_expressionList.count() > 0) {
        QString expression = m_expressionList.at(0);
        expression =  Utils::reformatSeparatorsPro((expression.left(expression.indexOf("＝") + 1) + resultnum), Settings::instance()->programmerBase)
                      .replace(EN_ADD, CN_ADD)
                      .replace(EN_MIN, CN_MIN)
                      .replace(EN_MUL, CN_MUL)
                      .replace(EN_DIV, CN_DIV);;
        m_expressionList.clear();
        beginInsertRows(QModelIndex(), 0, 0);
        m_expressionList << expression;
        endInsertRows();
    }
}

void SimpleListModel::expOutofRange()
{
    radixChanged(Settings::instance()->programmerBase, Settings::instance()->programmerBase);
}

/**
 * @brief SimpleListModel::updataOfSeparate
 * 更新所有历史记录算式的分割位数
 */
void SimpleListModel::updataOfSeparate()
{
    for (int i = 0; i < m_expressionList.count(); ++i) {
        QString expression = m_expressionList.at(i);    //获取原算式
        expression.replace(",", "");   //清空逗号分隔符
        QString newExp = Utils::reformatSeparators(expression); //重新填充分割符
        //更新数据
        beginInsertRows(QModelIndex(), 0, 0);
        m_expressionList[i] = newExp;
        endInsertRows();
    }
}

