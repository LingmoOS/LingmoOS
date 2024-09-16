// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RICHTEXTDELEGATE_H
#define RICHTEXTDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>
#include <QDomDocument>

/**
 * @brief The RichTextDelegate class
 * 封装富文本标签
 */
class RichTextDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit RichTextDelegate(QObject *parent);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;

private:
    void getDocFromLst(QDomDocument &doc, const QStringList &lst)const;
    void addRow(QDomDocument &doc, QDomElement &table, const QPair<QString, QString> &pair,const int &rowWidth)const;
    void addTd1(QDomDocument &doc, QDomElement &tr, const QString &value,const int &rowWidth)const;
    void addTd2(QDomDocument &doc, QDomElement &tr, const QString &value)const;
};

#endif // RICHTEXTDELEGATE_H
